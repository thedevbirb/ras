#!/usr/bin/env python3
"""Compare two relocatable ELF objects (ras vs GNU as) section by section.

Reads sqlite3_ras.o and sqlite3_gnu.o from the current directory.
Prints a human-readable comparison and a JSON summary line ("JSON:...").

Besides raw section bytes, it reports two deeper checks that are independent of
symbol *ordering* (which differs between the two because of FILE/SECTION symbol
placement and mapping symbols):
  - `.rela.*` contents: each relocation compared by (offset, type, addend) plus
    the *value* of the referenced symbol (indices may differ).
  - `.symtab` contents: symbols compared as a multiset of
    (name, type, bind, section-index, value, size), ignoring order.
"""
import json
import struct
import sys
from collections import Counter

FIELDS = ('name', 'type', 'flags', 'addr', 'offset', 'size',
          'link', 'info', 'addralign', 'entsize')


def cstr(data, off):
    end = data.index(b'\0', off)
    return data[off:end].decode('utf-8', 'replace')


def read_sections(path):
    data = open(path, 'rb').read()
    if data[:4] != b'\x7fELF':
        return None, 'not an ELF'
    if data[4] != 2:
        return None, 'not ELFCLASS64'
    e_shoff = struct.unpack_from('<Q', data, 40)[0]
    e_shentsize = struct.unpack_from('<H', data, 58)[0]
    e_shnum = struct.unpack_from('<H', data, 60)[0]
    e_shstrndx = struct.unpack_from('<H', data, 62)[0]
    shstr = data[e_shoff + e_shstrndx * e_shentsize:
                 e_shoff + (e_shstrndx + 1) * e_shentsize]
    str_off = struct.unpack_from('<Q', shstr, 24)[0]
    str_size = struct.unpack_from('<Q', shstr, 32)[0]
    sections = {}
    order = []
    for i in range(e_shnum):
        sh = data[e_shoff + i * e_shentsize:
                  e_shoff + (i + 1) * e_shentsize]
        name_off = struct.unpack_from('<I', sh, 0)[0]
        name = cstr(data, str_off + name_off)
        sections[name] = struct.unpack('<IIQQQQIIQQ', sh)
        order.append(name)
    return (sections, order, data), None


def symbol_table(data, symtab, strtab):
    """Return a list of (name, type, bind, section, value, size)."""
    soff = symtab[4]
    ssz = symtab[5]
    ses = symtab[9]
    stroff = strtab[4]
    strsz = strtab[5]
    out = []
    for i in range(ssz // ses):
        s = data[soff + i * ses: soff + (i + 1) * ses]
        no = struct.unpack_from('<I', s, 0)[0]
        name = cstr(data, stroff + no)
        info = struct.unpack_from('<B', s, 4)[0]
        shndx = struct.unpack_from('<H', s, 6)[0]
        value = struct.unpack_from('<Q', s, 8)[0]
        size = struct.unpack_from('<Q', s, 16)[0]
        out.append((name, info & 0xf, info >> 4, shndx, value, size))
    return out


def main():
    ras, err = read_sections('sqlite3_ras.o')
    if err:
        print('error: ras object:', err)
        return 1
    gnu, err = read_sections('sqlite3_gnu.o')
    if err:
        print('error: gnu object:', err)
        return 1

    rsecs, rorder, rdata = ras
    gsecs, gorder, gdata = gnu

    identical, differing = [], []

    print('=== SECTION HEADERS ===')
    for name in gorder:
        if not name:
            continue
        g = gsecs[name]
        if name not in rsecs:
            print(f'  MISSING  {name}')
            differing.append(f'{name}: missing in ras')
            continue
        r = rsecs[name]
        diffs = []
        for i, label in enumerate(FIELDS):
            if i in (0, 3, 4):  # name, addr, offset
                continue
            if g[i] != r[i]:
                diffs.append(f'{label} gnu={g[i]:#x} ras={r[i]:#x}')
        print(f"  {'OK  ' if not diffs else 'DIFF'}  {name}"
              + (f'  [{"; ".join(diffs)}]' if diffs else ''))

    # Symbols: needed to interpret relocations and for the symtab comparison.
    rsym = symbol_table(rdata, rsecs['.symtab'], rsecs['.strtab'])
    gsym = symbol_table(gdata, gsecs['.symtab'], gsecs['.strtab'])
    rval = {i: v[4] for i, v in enumerate(rsym)}
    gval = {i: v[4] for i, v in enumerate(gsym)}

    print('=== CONTENT ===')
    for name in gorder:
        if not name:
            continue
        if name not in rsecs:
            continue
        g = gsecs[name]
        if g[1] == 8 or name in ('.note.GNU-stack',):
            continue
        r = rsecs[name]
        gb = gdata[g[4]:g[4] + g[5]]
        rb = rdata[r[4]:r[4] + r[5]]
        if gb == rb:
            print(f'  OK    {name} ({len(gb)}B)')
            identical.append(name)
        else:
            n = min(len(gb), len(rb))
            first = next((i for i in range(n) if gb[i] != rb[i]), n)
            print(f'  DIFF  {name} gnu={len(gb)}B ras={len(rb)}B '
                  f'first={first:#x}')
            differing.append(f'{name}: content (first byte {first:#x}, '
                             f'gnu {len(gb)}B ras {len(rb)}B)')

    for name in rorder:
        if name and name not in gsecs:
            differing.append(f'{name}: extra in ras')

    print('=== RELOCATIONS (content, by offset/type/addend/symbol-value) ===')
    rel_diff = []
    for name in gorder:
        if not name.startswith('.rela'):
            continue
        if name not in rsecs:
            continue
        g = gsecs[name]
        r = rsecs[name]
        gents = gdata[g[4]:g[4] + g[5]]
        rents = rdata[r[4]:r[4] + r[5]]
        ges = g[9]
        res = r[9]
        gset = set()
        for i in range(len(gents) // ges):
            off, info, add = struct.unpack_from('<QQq', gents, i * ges)
            gset.add((off, info & 0xffffffff, add, gval.get(info >> 32, -1)))
        rset = set()
        for i in range(len(rents) // res):
            off, info, add = struct.unpack_from('<QQq', rents, i * res)
            rset.add((off, info & 0xffffffff, add, rval.get(info >> 32, -1)))
        gonly = gset - rset
        ronly = rset - gset
        status = 'OK  ' if not gonly and not ronly else 'DIFF'
        print(f'  {status} {name}: GNU={len(gset)} ras={len(rset)} '
              f'common={len(gset & rset)} gnu-only={len(gonly)} '
              f'ras-only={len(ronly)}')
        if gonly or ronly:
            rel_diff.append(name)
            sample = next(iter(gonly or ronly), None)
            differing.append(f'{name}: reloc content differs '
                             f'(sample {sample})')

    print('=== SYMBOL TABLE (contents as multiset, order ignored) ===')
    gc = Counter(gsym)
    rc = Counter(rsym)
    gonly = gc - rc
    ronly = rc - gc
    if not gonly and not ronly:
        print(f'  OK    {len(gsym)} symbols, all match')
    else:
        print(f'  DIFF  GNU={len(gsym)} ras={len(rsym)} '
              f'common={sum((gc & rc).values())} '
              f'gnu-only={sum(gonly.values())} ras-only={sum(ronly.values())}')
        for k in gonly:
            differing.append(f'.symtab: gnu-only {k}')
        for k in ronly:
            differing.append(f'.symtab: ras-only {k}')

    text_first = None
    if '.text' in gsecs and '.text' in rsecs:
        g = gsecs['.text']
        r = rsecs['.text']
        gb = gdata[g[4]:g[4] + g[5]]
        rb = rdata[r[4]:r[4] + r[5]]
        n = min(len(gb), len(rb))
        text_first = next((i for i in range(n) if gb[i] != rb[i]), None)

    print('=== SUMMARY ===')
    print(f'identical sections: {len(identical)}')
    print(f'differing: {len(differing)}')
    for d in differing:
        print(f'  - {d}')
    print(f'text first diff: '
          f'{("none (identical)" if text_first is None else hex(text_first))}')

    summary = {
        'identical_sections': len(identical),
        'differing_sections': len(differing),
        'reloc_content_diff': rel_diff,
        'text_first_diff': text_first,
        'diffs': differing,
    }
    print('JSON:' + json.dumps(summary))
    return 1 if differing else 0


if __name__ == '__main__':
    sys.exit(main())
