.option norelax

label_1:
nop
label_2:
.set FOO, label_2 - label_1
addi x1, x0, FOO

