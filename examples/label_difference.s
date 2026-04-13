.option norelax

.globl global_1
.globl global_2

label_1:
addi x1, x0, 1
label_2:
.set FOO, (global_1 - global_2) + ((label_2 + 2) - (label_1 - 1))

