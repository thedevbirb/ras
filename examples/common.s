# ============================================================
#  RISC-V Assembly: Common Directives & Instructions
# ============================================================

# ---------- Section directives ----------
        .section .data                # Switched to writable data section

# ---------- Data directives ----------
msg:    .string "Hello, RISC-V!\n"   # Null-terminated string
val:    .word   42                    # 32-bit integer
arr:    .half   1, 2, 3              # Array of 16-bit half-words
byte1:  .byte   0xFF                 # Single byte
buf:    .space  64                   # Reserve 64 bytes (zeroed)
pi:     .float  3.14                 # 32-bit float

# ---------- Alignment directive ----------
        .align  2                    # Align next datum to 2^2 = 4-byte boundary

# ---------- Text (code) section ----------
        .section .text
        .globl  _start               # Export symbol to linker

_start:
# --- Arithmetic ---
        li      a0, 10               # Load immediate (pseudo-inst)
        li      a1, 20
        add     a2, a0, a1           # a2 = a0 + a1
        addi    a3, a2, 5            # a3 = a2 + 5
        sub     a4, a3, a0           # a4 = a3 - a0
        mul     a5, a0, a1           # a5 = a0 * a1  (M extension)

# --- Memory access ---
        la      t0, val              # Load address of 'val' (pseudo-inst)
        lw      t1, 0(t0)            # Load word from memory
        addi    t1, t1, 1
        sw      t1, 0(t0)            # Store word back

# --- Branching ---
        beq     a0, a1, equal        # Branch if a0 == a1
        blt     a0, a1, less         # Branch if a0 < a1
        j       done                 # Unconditional jump (pseudo-inst)

equal:
        li      a6, 1
        j       done
less:
        li      a6, -1

done:
# --- Function call / return ---
        jal     ra, my_func          # Jump and link (call)
        j       exit

# ---------- Local function ----------
        .type   my_func, @function   # Mark symbol as a function
my_func:
        addi    sp, sp, -16          # Allocate stack frame
        sw      ra, 12(sp)           # Save return address
        sw      s0, 8(sp)            # Save callee-saved register

        mv      s0, a0               # Move (pseudo-inst)
        slli    s0, s0, 2            # Shift left logical by 2

        lw      s0, 8(sp)            # Restore s0
        lw      ra, 12(sp)           # Restore ra
        addi    sp, sp, 16           # Deallocate frame
        ret                          # Return (pseudo for jalr zero, ra, 0)

# ---------- Exit via ecall ----------
exit:
        li      a7, 93               # Linux syscall number for exit
        li      a0, 0                # Exit code 0
        ecall                        # Invoke system call

# ---------- End directive (optional) ----------
        .end
