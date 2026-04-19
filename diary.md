# Lexer

It has been very fun and challenging! First time writing one. I liked using Claude as an assistant
or rubber duck. It basically never came up with good design decision for what I wanted, but it's
very helpful to chat it as if it were someone doing the same project as you.

It's very challenging to come up with something that feels simple, for example how to do you want to
iter over your point, how it feels right to lay out data in memory, avoiding as much as you can the
amount of codepaths you can have.  The main logic is around ~200 lines of code or less and a
satisfying stub of it took a couple of days, almost full time. There are so many design choices I
like in every line:
- namaing,
- to scoping,
- flow of code execution,
- formatting,
- grouping of related logic
- what updates should be done where,
- what feels like to do in a lexer stage, what feels not

and overall trying to make the code as debugger / debugging friendly as I possibly can.
In particular handling escapes and strings took me quite a bit to figure out. I really wanted to do
something that resulted in simple code to read and execute. And sometimes the key is not trying to
solve every problem immediately recognising when it is the right level to tackle it.
For example for escaped characters the goal was to just identify where to close the string.
Verifying right escape sequences etc to have well-formed strings already in the lexing stage is just
not what you should do it. It should be in the stage where everything is _parsed_ and checked i.e.
parsing.


I feel like all of this is worth the effort. I'm so happy in caring so much about this, and while I
don't know yet it works well, I'd never be satisfied with it unless I wouldn't have crafted it line
by line with all the energy it required.

implementation notes:

it is useful that the lexer already returns a mapping of where every line stars, because in this way
we can easily create diagnostics.
in a similar fashion, it is useful to keep also comments as part of the lexing.
in general should it be there an associations between tokens and lines?

maybe on error finish to read the line?

lesson: it is so important to save all the intermediate computation that you can. For example in the
lexing stage you want to save every single possible piece of information regarding a token, as much
as you can. it makes everything else easier to build on top. Everything that you don't keep is lost,
although it has been already read and computed, which is a waste.

I didnt' realize how effective is centralizing all state in a cursor fat struct.

It's crazy how you don't even think about lifetimes and memory management with arenas.

The first 80% takes 20% of the time, but tiny details take a good amount of time.

ZII is awesome.

I'm starting to get a feeling on when branching is a code smell or not. Intuitively, branching
should be done when the shape of your data is different, _or_ when you decide how to exit a loop.
This makes so much sense in a data-oriented-design programming.


# Parser

It is very challenging how to model the data accordigly, trying to forecast how you will need it.
Implementation its expensive, so you have to design carefully considering what you want out of it.

Not trivial to understand what is the right output of the parsing phase. Again, it's important to
not get too carried away with finding the right semantics that fit our mental model but rather
transforming the data in the most effective way to reach our goal: producing an object file.

A good approach to find out what should be the right output, is thinking in terms of the next step
after parsing, and thinking what is a well-defined input you'd like to have for that stage.

when parsing directive arguments what can be done immediately is written in the section, otherwise I
keep a fixup list, where the size is known and its computation is deferred.

it makes sense in the parser to do some object file writing already because some information is just
there already and you see it.

Creating an expression parser is its own adventure. I had to generate the grammar because that is
out of scope for this project and it's already a well defined problem. I hope the generated grammar
is correct tho. Won't be too hard to tweak it if the setting of the code is right.

Finding good pattern that leverage clearly documented no-ops can make your life a lot easier and
reduce a lot the number of codepaths. Examples:

- Advancing a cursor is a no-op if the underlying data stream has ended
- Error setters functions are a no-op if an error has already been set.

Another interesting example is not caring about whether an error is found and continue with the
codeflow (when possible) but still note the error. The idea is not do any "if error" logic until
absolutely necessary. It depends a lot on the context, but sometimes it's not a big deal if a
certain action is done on invalid input, the most important part is mark the error so that then you
stop. This reduces a LOT the number of branching you do.

Proper ELF format is tough, ngl.

Again, centralizing state with a fat struct makes code simpler. There is an argument to be said
about writing functions that take the minimal amount of arguments, but that doesn't come for free:
it often comes with more types and code.

I didn't expect that I'd find a use-case for a try-catch-finally in this codebase. I strongly
believe it is a 99% poor error handling, but I may have found a good 1%: given that when you
encounter an error when parsing you can do a long jump, the rest of the code can be strongly
simplified without trying to either do if-err checks or make a lot of code no-op in certain
conditions.
As such, the toolbox can be:
- assert invariants
- return errors and its intermediate state when you can recover it right after function call, or
  close to it
- snapshot progress and longjmp to known point

While I dislike early returns, for the parser logic a long jump would be nice. When an error occurs,
jump at the end of parsing function. This reduces the amount of if-err checking and no-ops to be
implemented. However, I'm still not sure about it.

I have to say assembly syntax if full of exceptions and not very regular, a lot of things depend on
context, which I don't like.

The amount of different directives out there and how differently they behave is quite astonishing.

I've found super effective to implement growable arrays that have no reallocations cost by letting
them have a dedicated arena with a large virtual size, small reserved size, and no grow options.
Super cool!

Next big challenge is going to be relaxation and evaluation of expressions taking into account
possible undefined symbols, and relocation entries.

Now a sufficiently varied input file uncovers holes in both lexer and parser.

Again, so many times preserving ALL information created during transformations saves my butt so many
times. It's already computed, not saving it is a waste and most of the times is not that memory
heavy, yet it makes extending the software so much easier.

It could be interesting to avoid recursion functions entirely, this is an excellent video about it: https://www.youtube.com/watch?v=YuaJ8x_NcLw.
However, I'd still bound the number of iterations since each would allocate. As such, for now it's
okay to recurse imposing a very short recusion level limit.

# relaxation

Now there is a tough part, where you evaluate every expression and compute sizes and offsets for
each section. It's surprising that this requires a fixed point iterative algorithm with a
potentially very large number of iterations over all the program statements.

TODO: limit what instructions and directives can appear in .bss section.

# relocations

TODO: Relocations change the in some way the expression evaluation algorithm. When encoding, if some
undefined symbols are found, relocations entry must be emitted, and they can only be of the form
`symbol + addend` (or viceversa, and signed, so a minus can appear).
Probably this should be done by another component, the Encoder + Relocator, or in another function
of the Resolver.

Relocations is really forcing me to think through more details about expression evaluation and all,
influencing back on the design on some components such as expression parsing and evaluation.
Details matter, and details influence design. If I wanted to make a really toy assembler that didn't
support relocation, these problems wouldn't have been there, but it would have been a way less
significant project.

Any expression after sufficient simplification and evaluation should be of the form `(constant, symbol_a,
symbol_b, operand`) so that it can be written with relocations if needed.

Solving this properly without doing too many invasive chances blocked me for a few days. This, along
with solving circular dependencies.

For circular dependencies, I solved it by recursively looking to into symbol definitions until an
a declared symbol or constant was found, and tracking in the meanwhile which symbols were being
resolved.

For the relocation friendly format, in the end I kinda mimic what gas did with its expressionS type,
that is saving the pair of symbols, the addend and the operand in the same expression node. The way
I did the parser was fine, because it was very detailed, so the information was there, however I
needed to group them in a cohoerent way so that I could emit relocations. This can be easily done
during evaluation, and it is a correct place to do it, because relaxation might move things so I
need to recompute all expressions anyway.

I'm essentially reverse engineering a lot of gas behaviour by feeding it some inputs and see how it
behaves. In doing so, I've also found a couple of bugs to report!

# encoding

Now I'm finally at the encoding stage. By this time, I'm quite confident in understand and reading
riscv assembly, which is honestly awesome, and I feel very empowered. Examples:

1. Understanding why a table lookup is better than a jump table, both in execution time and in
   binary size.

Writing the code for the encoding phase is sometimes a bit mechanic. However, there are still so
many little details you can add that can make even simil-boilerplate code so much nicer to read,
understand, write and execute. Intention is so important. I want this code to read as a reference
book.

Even though this is a very "mechanical" section, you can really pour a lot of thinking in trying to
make it as simple as possible, meaning with the least amount of code needed.

I've added encoding to the instructions, now it's time to run it via the complete examples. Again,
that reveals stuff. In general, I've noticed some extra care is needed again during label
difference, ensuring that no expandable instructions are in between. Then, I noticed that gas may
produce relocation symbols for local labels, which are encoded as `.L<numeric_label>^B<occurrence>`.
Currently, I haven't treated local labels as symbols to save, and this must be changed.

# General about C

Lookup tables, paired with designated initializers, are incredibly powerful. While the latter is
technically just syntax sugar, it makes creating the former so much easy and safe. I acknowledge
that it imposes some complexity on the compiler that may not be desiderable, however I just like
them.
It also allows to write function with optional arguments, using a quite reasonable macro trick.
While optional arguments are bad when exposed by dependencies, since the defaults may change without
you noticing, are very handy when you write your application.

# Resources

Note that some of them may not be updated or reflect accurately what the actual assembler does, but
still better than nothing.

- https://sourceware.org/cgit/binutils-htdocs/commit/?id=30b032c8ecd7b53d995058be3faf6c031e229de5
- https://sourceware.org/binutils/docs/as/
- https://github.com/riscv-non-isa/riscv-elf-psabi-doc

# Random

Just found out that there are some good docs that I just skipped :) https://sourceware.org/cgit/binutils-htdocs/commit/?id=30b032c8ecd7b53d995058be3faf6c031e229de5
Also https://sourceware.org/binutils/docs/as/.

Reading through this, and some things don't match, at least not in all targets. For examples, empty
expressions are not really supported on rv, while they claim they are.

I should try to write neovim snippets.

In general assemblers don't follow a standard. So some stuff is a bit unique or up to the assembly
implementation. I'm trying to follow a bit gas for some compatibility, but some stuff is just funky.

In general there is some value in being wary about 64-bit types. Most of the times you do not need
them and 32-bit is large enough. On the plus side, makes porting to 32-bit easier, and simplifies
padding calculations.

Representing a maybe U32 with `Vec2_U32` has the nice effect that given you access it as a (x,y)
pair, you can do `if (my_value_maybe.y)` as if "yes, it is set" lol.

# On exceptions

Assembly seems an exception or corner-case driven language:

1. Label definitions are unique, and they behave in a very specific way. But then you have numeric
labels, which must be lexed, parsed and evaluated differently. But they still kinda behave like a
label.
2. Instructions have some consistent format and sizes, but then you have pseudoinstructions, and
each of them must be parsed and expanded individually. This leads to relaxation algorithms, which
while I understand why they must exist, and I can't really think of any other simpler way to
resolve the same problem, I really wonder if we should have that problem in the first place.
3. Load and store instructions have many different formats, which must be parsed with care. For
   example: a load word instruction can be in these forms: `mnemonic rd, rs1`, `mnemonic rd, (rs1)`,
   `mnemonic rd, offset(rs1)`.
4. Some relocations require context of the previous lines and symbols, e.g. `%pcrel_lo(symbol)`
   requires going back to the matching `%pcrel_hi(other_symbol)` occurrence and then check
   whether `other_symbol` is local or not.
5. You can use symbols in expressions? Yes, however you need context about instructions. In some
   instructions, symbols e.g. labels are not allowed because they'd either get truncated or are not
   intended to use there. E.g. addi x1, x0, symbol. Unless the symbol is absolute!

# GAS quirks

The following is a list of gotchas and quirks I have found while using `gas`, specifically
`riscv64-elf-as` binary on MacOS.

1. if you declare a label with `.local`, without defining it, it is marked as global.
2. when assembling, it doesn't validate that `pcrel_lo` doesn't point to a defined label where
   `pcrel_hi` is used.
3. The `li` instruction MUST accept a constant (not even an absolute expression!) which is okay but
   seems a bit counterintuitive considering how flexible gas is elsewhere, sometimes. I understand
   though that accepting an undefined symbol is problematic because on 64 bit you would not have a
   way to express it via relocations.
4. Writing `j global_2 - global_1` produces the following ELF relocation:
   ```
   Relocation section '.rela.text' at offset 0x1d0 contains 1 entry:
     Offset          Info           Type           Sym. Value    Sym. Name + Addend
   000000000000  000700000011 R_RISCV_JAL       0000000000000000 global_2 + 0

   The decoding of unwind sections for machine type RISC-V is not currently supported.

   Symbol table '.symtab' contains 8 entries:
      Num:    Value          Size Type    Bind   Vis      Ndx Name
        0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND
        1: 0000000000000000     0 SECTION LOCAL  DEFAULT    1 .text
        2: 0000000000000000     0 SECTION LOCAL  DEFAULT    3 .data
        3: 0000000000000000     0 SECTION LOCAL  DEFAULT    4 .bss
        4: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT    1 $xrv64i2p1_m2p0_[...]
        5: 0000000000000000     0 SECTION LOCAL  DEFAULT    5 .riscv.attributes
        6: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND global_1
        7: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND global_2
   ```
   That is, the symbol difference doesn't error which is very weird and probably a bug.
5. `.skip` directives accepted a symbol difference for the padding value, while `.align` does not.
   For example:
   ```asm
   .skip  2, global_2 - global_1
   .align 2, global_2 - global_1 # doesn't assemble
   ```
   I really don't see a strong reason why! I'm supporting it.
6. Again, handling differences between some globals has weird behaviour, similar to being buggy.
   Consider the following source:
   ```asm
   .globl global_1
   .globl global_2
   beq x2, x1, global_2 - global_1
   ```
   After assembly, `readelf` returns the following:
   ```
   Relocation section '.rela.text' at offset 0x1d0 contains 1 entry:
     Offset          Info           Type           Sym. Value    Sym. Name + Addend
   000000000004  000700000011 R_RISCV_JAL       0000000000000000 global_2 + 0

   The decoding of unwind sections for machine type RISC-V is not currently supported.

   Symbol table '.symtab' contains 8 entries:
      Num:    Value          Size Type    Bind   Vis      Ndx Name
        0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND
        1: 0000000000000000     0 SECTION LOCAL  DEFAULT    1 .text
        2: 0000000000000000     0 SECTION LOCAL  DEFAULT    3 .data
        3: 0000000000000000     0 SECTION LOCAL  DEFAULT    4 .bss
        4: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT    1 $xrv64i2p1_m2p0_[...]
        5: 0000000000000000     0 SECTION LOCAL  DEFAULT    5 .riscv.attributes
        6: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND global_1
        7: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND global_2
   ```
   So `- global_1` has been completely ignored, and a JAL relocation has been emitted somehow.
   Looking more closely with `objdump -d` returns:
   ```
   0000000000000000 <.text>:
      0:	00111463          	bne	sp,ra,8 <.text+0x8>
      4:	ffdff06f          	j	0 <.text>
   ```
   So it has been somehow converted to a jump. Maybe I don't have enough context to understand the
   reason of such choice, but to me that seems just weird.
7. Some relocation operators should be rejected because their semantics would not apply, yet they
   are accepted, for example:
   ```asm
   lui x1, %pcrel_hi(global_1)
   ```
