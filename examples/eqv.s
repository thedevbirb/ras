# .eqv recomputes its value at every USE, so it picks up the current value
# of the symbols it references. .set/.equiv compute it at definition.
.eqv F, A + 1 # F equals 2
.set A, 1
.set F_1, F # F_1 equals 2
.set A, 2
.set F_2, F # F_2 equals 3

# G should equal 10, so it uses the original definition of F
.eqv G, F + 8
# Now G is used inside a non-forward expression, so it's fully expanded
# It should equal ((2 + 1) + 8) + 128 = 139
.set G_1, G + 128 # 16
