# --- global commons (SHN_COMMON), default alignment ---
.comm   a, 8

# --- global common, explicit alignment (bytes, pow2) ---
.comm   b, 8, 4

# --- `.common` is a synonym for `.comm` ---
.common c, 16

# --- repeating `.comm` with the same size keeps one symbol ---
.comm   d, 8
.comm   d, 8

# --- `.local` + `.comm` -> .bss section symbol (LOCAL) ---
.local  e
.comm   e, 8

# --- second .bss symbol, sequential layout ---
.local  f
.comm   f, 24

# --- third .bss symbol, sequential layout ---
.local  g
.set    g, 2
.comm   g, 24

# --- `.globl` + `.comm` -> global SHN_COMMON ---
.globl  h
.comm   h, 8

