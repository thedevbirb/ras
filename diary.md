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

A RELAX relocation is not emitted when there is no symbol, but only an addend. For example `call 0`
won't emit RELAX even if `.option relax` is set.

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
- https://github.com/riscv-non-isa/riscv-asm-manual

While this resources are helpful, sometimes implementations don't really match it. An example: if
relaxation is enabled, `call` should emit a `R_RISCV_RELAX`. Fine, however in gnu gas `call 0`
doesn't emit one. Why? You have to look at source code anyway.

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

Ok while choosing riscv asm for its simplicity has been a good choice, if I want to link an actual
executable to test is a damn pita.

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
3. In some instructions, like LI, gas seems to accept only an expression that reduces to a constant
   at parse time. However, this is not really true. While:
   ```asm
   li rd, label2 - label1
   ```
   Doesn't assemble, assigning that difference to a symbol `FOO` and then using `FOO` assembles.
   Sometimes, this is really confusing.
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

# General logs:


Wed Apr 22 11:02:53 CEST 2026

It's hard. It's genuinely hard. It's hard to allocate time where you don't code, but you design and
understand, while maintaining the rigour and not chase the productiveness feeling. In practice,
spending time designing and understanding is the most productive thing you can do, because design
time is cheap, coding is inevitably slower, regardless of AI


Fri May 22 16:01:36 CEST 2026

Updating again this diary after I decided to take a break and look into the job market. Happy to be
back again to coding. I want to complete this project, and while not add every feature, I want it to
be extensible and forward-thinking. Examples: macro support and include directives even if not
implemented right away should be thought of.

Now, on my c_layer I also have very powerful primitives that will help me to overcome some of the
previous challenges, such as handling verious unbounded collections like symbol/section table etc.

For the lexer I don't know whether I should keep the tokens streams or not. If I keep it, a
thought regarding include directives: I should probably save tokens in a linked list of Token_Xar,
where each node corresponds to the current file.
Suppose I have a.s that includes b.s. Then my chain would look like Tokens_A -> Tokens_B -> Token_A.
If I don't keep it, like llvm mc and gas, it means the parser invokes it every time to get the next
token.
I think for a line-based assembler it makes sense, and the lexer becomes so generic it is easy to
retrofit also for other projects.

I still need to go back to design again. It's very hard.

Sun May 24 12:38:38 CEST 2026

Ok the lexer now yields one token at a time. This has some downstream consequences on the parser. In
particular, the parser will invoke the lexer to parser statements and fill every information while
doing so (string table, symbol table, etc). As such, the parser will have its own context containing
such tables that are preserved across calls.
The parser returns when the input has ended or when an #include-like directive is found. As such,
there should be a "parser driver" (in LLVM there is the concept or a source manager) that opens
input files and preserves call stacks. In our case, it should manage the stack of lexers, mmap
files, and resume.
While I might not support such directives, and so no parser driver, it will still influence the
parser API.

Similarly, I should take into account macro usage:
https://ftp.gnu.org/old-gnu/Manuals/gas-2.9.1/html_node/as_107.html, although I won't support it for
now. First, it implies keeping a small map of macro names with macro information, provided to the
parser context. Then, the parser should switch to an alternate input source, provided in the macro
information itself. Then, how to handle recursive expansion? Again, as a Lexer stack.
I can think of the Lexer cursor as my current stack frame.

Mon May 25 11:40:08 CEST 2026

Pivoting again with the design. I wonder whether I should do fragments as well. Keeping an in-memory
IR for both tokens and statements sounded "simple" but at the cost of high memory usage and its
quite unnecessary. I mainly wanted to expose the ability to create such list of IR elements, but for
a binary using it is overkill.
Again, as with lexing, parsing could return one statement at a time.

Diagnostics are an interesting design problem because it creates tension with creating an
efficient program. Both can coexist, but they tend to go in different directions. On one hand,
diagnostics benefit from knowing as much as possible: one of the first design principles I employed
in the assembler is to _never_ lose intermediate information between steps; it has already been
computed, and not keeping it is a waste. At the same time, in an efficient program you want to do
the minimum necessary and not use too much memory. Diagnostics often require a lot of context, hence
memory. Again, I think both can be achieved but it's not trivial.

I have to fix APIs, or this design process never ends. I think it makes sense to have the some logic
components being very pure i.e. CPU only sprints, with particular control flow delegated elsewhere.
Let's start with the lexer. The lexer should be the dumbest pipe ever. Read from an input, return
a token and/or an error. The token information contains an the index offset where it has been found
in the source, its size in bytes, and the token kind. For a token, an error enumeration is enough
information to tell where something went wrong.
Both token and error should be set on error. Why? Because when an error has been encountered we were
trying to lex a token, and we report how far we went along with what token it was. The starting and
ending of the partial token naturally encodes the underline-with-caret portion of text we want to
highlight.

Parsing is way more articulated, because a lot of side-effects can happen. A reasonable mental model
is that a parser produces statements, and reading statements produces _some_ side-effects.
I don't want know whether we want ALL side-effects to happen. Let's think for example about include
directives. That's not the job of the parser I think to open the new file. However, it's part of the
parser job to start filling the various tables, and ensure the produced statement is well-formed.

Tue May 26 11:44:28 CEST 2026

Today I'm tackling the challenge of making a completely non-recursive pratt parser. I think it's a
nice one, and something I could write a blog + live coding video of it.
Then, I'm thinking about moving risc-v specific-stuff in a separate folder, and scaffolding a first
interface. Again, I don't plan to support any other backend anytime soon, but I think it's good
practice to think about which stuff is architecture or OS specific.

The core of the non-recursive pratt parser is done, and it works. Very happy with it.

Wed May 27 12:08:10 CEST 2026

Scratch arenas, declared in thread-local storage are very cool. With them, you can essentially get
more customizable stacks along with the one we're stuck with. I was a bit unsure about having
something that allocates not visible in a function signature, but this is the wrong framing: we
don't mark function that use stack memory in a special way. It's implicit. For scratch arenas, it
should be the same: it's just memory we can always use at any time, no matter what, and that will be
freed at the end of the scope. It's up to the caller to set it up so that it can be already
reserved or with whatever guarantee might be needed.

I've thought a lot about diagnostics. Compiler diagnostics are a true art. There is very good
engineering behind it. I decided that I want to have a Source_Manager (the name sounds so OOP,
bleah) that tracks where each source of text, that can be a file, a macro, or whatever buffer, is
defined.
Tokens will have logical offsets, and sources will have a starting logical offset. This would allow
the code of being agnostic of what happens behind the buffers at very little cost.

I also discovered about a maybe better way for a String8 primitive which is compatible also with C
strings, and requires creating them with always a null terminator, and pointing them to the
beginning of source, with the previous position providing the header. The memory cost is negligible.
I'm not sure however whether to make it the default string type here.

Thu May 28 17:33:42 CEST 2026

Done a bit of diagnostic refactory, inspired partly to what llvm does. In general if you remove C++
madness llvm is generally packed with good ideas.
Once again I find very important the principle of treating errors as values as much as possible and
to avoid as much as possible creating custom codepaths for errors. In something like a parser errors
can happen so damn often and it's way better to track the error, fill with zero/sentinel values and
go forward until you manage the error where you want. I think it's fine to assert states! But don't
pollute codepaths.
Another lesson, which I don't follow often though, it's to not find a nice API immediately. Write
code, write logic, think in terms of data. Then, and only then, find the right interfaces or
abstractions that remove code deduplication, if necessary.

I feel like I'm stealing ideas, but in practice I don't know whether I can get around fragments.
They're just too handy as a concept, since there is some data you can write right away and parsing
time and other were you want to add placeholders.

Fri May 29 10:53:29 CEST 2026

Studying fragments has been interesting. In order for them to really work, they need to encode a
statement IR. The reason is, a fragment must capture information about a variable-size
statement/instruction, so that it can be re-evaluated during relaxation. As such, if I want a
statement IR it would look like a fragment prototype. So I think I can have both.
GNU as fragments include a `fr_literal` field which is the bytes that will contain the encoding.
Those are allocated based on the instruction worst size, and then written / modified as new
information is obtained. For example a large `beq` becoming a `bne` is overwritten by flipping the
`funct3` bit and so on. GAS `frag` structure is probably a good starting point.

Write the most procedural, data oriented and dumb code you can possibly code. It will be the most
fast, readable, efficient and reusable you will write. Semantics and meaning come from usage. Don't
see hierarchies where there aren't meant to be. The program needs to transform data, that's it. The
rest is cruft to adhere to a mental model, of which the program, your goal, and your users, don't
care about.

Thu Jun  4 14:29:43 CEST 2026

Yesteday I completed part of the refactoring towards more procedural code. Now I'm reading about
fragments in GNU as. Fragments are organized as a linked-list, and divided per section, meaning that
there is a linked-list per section. Moreover, each section has its own arena so that memory is
contiguous during relaxation, where each section frag is read a lot of times.

I'm understanding more about how to use arena allocators. In particular I should strive to create an
arena for data of the same type, for example an arena for the symbol table, an arena for the
sections, an arena for expressions etc. This makes data more contiguous and at the same time makes
lifetime management fairly clear. This is something I kinda nailed in the beginning by accident but
then I deleted it.

It's much easier to create the perfect data structure to solve a problem, and much easier to use.
For example, if a linked list is allocated from a single arena, then elements will be as contiguous
as in an array, with no pointer chasing penalties, because you can iterate sequentially. It grows
precisely as you need without reallocation costs or overestimated capacity. And you can still get
O(1) by saving pointers instead of indexes in case your problem allows you to.

Fri Jun  5 14:22:05 CEST 2026

I'm implementing a lot of GNU as ideas right now, and while some of that C code might look arcaic is
really packed with good ideas.

Sat Jun  6 15:39:24 CEST 2026

I'm on track, and I feel productive. It's good.

Mon Jun  8 10:32:52 CEST 2026

A very interesting challenge of an assembler, or a compiler, it to being able to manage invalid
input as you process, recover from it while tracking errors and continuing. From this point of view,
you have no other choice that treating errors as data and values, and avoid as much as you can
different codepaths for errors, but rather having the same codepaths plus an additional error
tracker. Later, where appropriate, or in a place where doing more control flow is suited, you handle
them.

GNU as fragments are just a brilliant concept that works wonderfully well, because it is the perfect
data structure created for a problem.

New unexpected, but rational behaviour found. The `.set` directive creates snapshots of a symbols if
redefined multiple times, however it still points to the last definition. Example:
```asm
.set BASE, .
# other stuff 1
.set BASE, .
```
If code in between references the first occurrence `BASE` than that symbol node pointer is used and
kept. It also holds for forward references.

Tue Jun  9 13:21:12 CEST 2026

Found a bug in GNU as:
```asm
label2:
call foo
label1:
.align label2-label1
```

Leads to
```text
• ⟳ 13ms ~/personal/assembler ❯ riscv64-unknown-elf-as ./examples/test.s -o ./examples/test.o                                                                                                                                                                     main
./examples/test.s: Assembler messages:
./examples/test.s:4: Warning: alignment too large: 63 assumed

riscv64-unknown-elf-as: out of memory allocating 9223372036854841460 bytes
```

I have a better view of constant vs absolute vs unresolved:

1. A difference between labels of the same fragment (therefore also same section) can be folded into
   a constant safely
2. A difference between labels from different fragments, meaning that some expandible instruction is
   in between, would result into an absolute expression, and after relaxation is completed it is
   folded into a constant.
3. A difference between labels from different sections, or between symbols from the undefined
   section, is unresolved.

Note that all of this isn't related to local vs global, which is merely a matter of visibility.

Also, .set etc accept only an absolute expression, according to the definition above. So .set baz,
foo-bar where foo and bar are globals within .text section is okay, but .set baz, foo-pronto where
pronto is not defined it's not accepted because it would be unresolved.

So constant/absolute and unresolved must be determined by checking the symbols.

Wed Jun 17 18:46:31 CEST 2026

Long break due to family. Added initial .align support. I need to think about how to test the
assembler. Recent posts by Ryan Fleury made me way less bullish about unit testing and alike. While
an assembler can be indeed very edge-case oriented, it makes sense to be skeptical about generating
huge amount of edge-testing code. It's something that is often overlook but writing the right way to
test a specific software is important, and every software is different so something different should
be built each time. Similar to writing the perfect data structures to resolve your problem and not
rely on glueing together off the shelf solutions.

Thu Jun 18 15:07:58 CEST 2026

Starting to look into basic arithmetic I instruction support in this new iteration. I see that GNU
as sometimes can be overly strict. Consider the example:

```asm
label1:
nop
label2:
addi x1, x0, label2-label1
```

The example above doesn't assemble with the following error (GNU assembler (GNU Binutils) 2.45):

```text
Error: illegal operands `addi x1,x0,label2-label1'
```

However those are local labels in the same fragment, so their difference is always constant
regardless of linking, and should equal 4. GNU as internally probably checks that it's either
a constant, a symbol in the absolute section, or requires that the `%lo` relocation operator is
provided. In the first two cases, it then checks that it fits in 12 bits, otherwise it errors.
If `%lo` is provided and the symbol is a constant, then there is a direct resolution which doesn't
emit a relocation.

TIL that GNU as features a mini-DSL of format characters that specify how instructions should be
parsed. For example, `lw` is encoded as `d,o(s)` which means "destination register, offset (source
register). Offset can be optional, however. Not a big surprise that, as extensions were added over
time, this has become a bit overloaded. I don't think reasoning over that giant `switch` at
`riscv_ip` is easy. I think some form of it makes sense, though. I don't know whether format
chars are it.

Reading about the `.insn` directive and that is another DSL embedded in the assembler :D. It
essentially translates to the format char DSL for later parsing, which is nice. I will not support
it but it helps knowing about it.

Tables should go in .c files instead of header files. My bad.

Fri Jun 19 12:07:38 CEST 2026

Once again, I'm surprised how many smart solutions there are in GNU gas. I'm getting a bit more fond
of the format chars for encoding arbitrary instructions. It may not be perfect, but then I see the
shit ton of instructions, edge case etc GAS handles. Pure respect! It also encodes different
variants for the same instruction quite elegantly.

I'm considering my own encoding table variant, but I want to use String16 with enumerations instead
of chars. I think less shenanigans are involved although you get a bigger table.

Sat Jun 20 14:26:18 CEST 2026

Probably writing good old fashioned advice, but go slow to go fast. Maybe I would have been faster
if I forced myself to read most of GNU as code before starting at all.

Decided to go again over GNU as with a debugger while reading a single line and see what it does
with `addi x1, x0, %lo(1234)`. Commit used: `fb7b22bbeac9f26043f3aa92228c6ade7e2ea232`. Log:

1. Identify it's an instruction because there is an identifier at the beginning of the line.
2. Using the opcode hash table, return the pointer to the first entry in the encoding table. It
   might be of an invalid extension, no worries about that.
3. Iter until a valid encoding can be tried, taking into account provided information with
   `riscv_multi_subset_supports`.
4. Start encoding using the format char `d,s,j`.
5. When `j` is found, the possible immediate relocation is immediately set to a low 12 bit I type
   variant. It will be overwritten. However, also a `p` which indicates all the possible strings after the `%` sign for
   I-type instruction (5 variants) is set to `percent_op_itype`.
6. Jump to `alu_op` label to parse a "small" expression. So when parsing the possible relocation
   types are already known. The result will be written into an output pointer, which is NOT
   zero-initialized like other pieces of memory.
7. Inside `my_getSmallExpression` if first tries to parse relocation with the `parse_relocation`
   function being invoked. It returns true is a matching relocation is found and writes the result
   to `imm_reloc`. Essentially the first part of this instruction finds the start of the expression
   itself, taking into account possible relocation operators. This first part sets also whether
   we've found a relocation operator as bool (size_t though) and returns it.
    a. Then it checks the content inside the parenthesis cannot be a register, good call.
    b. Then the actual parse expression function is invoked: `my_getExpression`, with the beginning
    of the expression, in this case `(1234)`. This is called "crux".
8. Inside `my_getExpression` there is a difference between "deferred" expressions or not. I don't
   know exactly what it means yet but in practice when this is called it is true if `force_reloc`
   is set, which is the case when the relocation would be `BFD_RELOC_RISCV_GOT_HI20`.
9. Now, the main `expr` routine is called, defined in `expr.c` so mostly agnostic of the
   target. Notice that `expression` is actually a macro defined in `expr.h` which sets the mode to
   "normal". The pratt parser code starts.
10. Once we get out of `my_getSmallExpression`, we do NOT try to normalize already to a 12-bit
    number _because the relocation operator is still not resolved_. In this case we simply exit
    after some checks.
11. NOTE: the `expr` routine has _NO relocation operator support_. This means _ANY relocation
    operator MUST be at the beginning of an expression_. For example `addi x1, x0, 1 + %lo(1234)` is
    invalid but `addi x1, x0, %lo(1234) + 1` is valid. Another detail is that since `expr` is called
    with the input `(1234) + 1`, so it is immediately resolved with value `1235`, meaning that a
    relocation operator in practice doesn't have any precendence, and it would have been equivalent
    to write `%lo(1234 + 1)`.
12. After all of this, `append_insn` is called to write the encoding bytes into the appropriate
    fragment. There is a custom branch in case a relocation is met, distinguishing with different
    relocation types.
    a. Apart for some special relocation types, an `howto` structure is filled from a table, which
    contains all informations on how to deal with a particular relocation. See `elfxx-riscv.c`
    `howto_table`.
    b. A fixup is created, the size of the fixup is the size of the relocation which, for instructions,
    is 4 bytes by default, like the default instruction size, and the relocation type is provided as
    well. Since the expression is constant `1234`, the `fr_offset` field is set with that value.
    Also the relocation type of the fixup is set.
    c. Noteworthy that the fixup information is attached to the instruction created as a pointer via
    the `fixp` field. Lastly, depending on the relocation type, a new fragment must be created.
13. The input is done as well as the assembly pass. Now we enter into the `write_object_file`
    procedure, which, among over things, runs functions over all sections using
    `bfd_map_over_sections`. The following operations are done:
    a. `renumber_sections`,
    b. `chain_frchains_together`,
    c. `relax_seg`,
    d. `size_seg`,
    e. `adjust_reloc_syms`,
    f. `fix_segment`,
    g. `write_relocs`,
    h. `compress_debug`,
    i. `write_contents`.
14. Note that in `fix_segment` we apply the fixups. In our case we have a fixup with the LO12
    relocation type, however `fx_addsy` is NULL, because no symbol is provided, so the fixup can be
    resolved right away without filling the `reloc_list` that would be written in the final object
    file. However, in case `fx_addsy` is set, `reloc_list` is still not touched. This means it's
    probably NOT used within this architecture.
14. Finally, we `bfd_map_over_sections` with the relaxation procedure, by calling
    `relax_seg/relax_segment`. We start iterating over individual fragments. In this case even with
    our relocation there isn't any alignment or fill requirement so there isn't any special work
    done. However some aligning work is still done, and I don't understand it yet.
15. In the `write_relocs` section, we start indeed to track write relocations. Since the global
    `reloc_list` isn't used, the way relocations are found is by tracking which fixups aren't "done"
    yet. Information about the relocation is taken from the fixup field, and it's finally written in
    the appropriate object file section buffer.

Tue Jun 23 16:34:06 CEST 2026

Previous analysis of GNU as lifecycle has been very helpful because it gives a rough track on the
things that should be done, in what order and with which information. I feel more comfortable know
trying to trace down a path.

Tue Jun 30 16:39:44 CEST 2026

I'm noticing that GNU as does this pessimistic-by-default pattern where it assumes the pessimistic
case (for example an instruction encoding) and then simplifies it later. The reason for doing this
is to simplify codepaths: the simplification process (which is necessary) already contains the logic
or jumps that should be done by eagerly checking whether you're in the worse case or not.
This is not to say you should always assume the worst case, but in this context there isn't a
meaningful improvement in performance by doing pessimitic-by-default, and it results in less code
which is arguably better.

Wed Jul  1 12:10:44 CEST 2026

I should strive to avoid any kind of global variable and nil-terminated list, unless in very
specific cases (e.g. linked lists).

I'm acknowledging the assembler is looking more and more as a GNU as rewrite in a lot of parts. I
don't know whether it's good or bad. However, while with way less features, I'm getting something
(imo) more readable and library friendly, with better error handling and diagnostics. I also notice
that once you opt into the fragment architecture then a lot of the downstream code looks similar.

Another usefulness of actually reading the source code is that now I can precisely pinpoint where
GAS quirks happen, for example that `beq t0, t1, label2-label1` completely skips the label
difference creating something unexpected (not necessarily right or wrong). This happens in the
`append_insn` when `add_relaxable_insn` is called, where expression information is provided but the
`x_op_symbol` is skipped.
I understand _why_ that is a tricky situation: the immediate might be beyond a 12-bit range leading
to instruction expansion. Tracking the label difference properly would mean creating a fixup, but a
fixup (at least for how they're defined in GNU as) is a patch for some bytes within a frag, and in
case it is not resolved, it becomes a relocation. Fixups require relaxation to complete first, since
in this way distances can be safely computed. Creating a fixups for a relaxable instruction like a
branch could lead to after relaxation which should NOT happen.
I still think it can be handled a bit better, with two possible approaches:

1. You check that `x_op_symbol` is null, and error otherwise OR
2. Since GNU as always expands a branch to a branch + jal, created the fixup directly in the jal.

Fri Jul  3 14:24:18 CEST 2026

Yesterday I felt slow, today I feel fast and effective. You never know how it is!
Now I'm in a very good spot regarding assembling. Still no logic for actually writing to the object
file but I'm getting closer.
I have to re-do some of the local label / dot symbol management related to fragments.

Maybe I've written already about it but the various `.local`/`.weak`/`.global` directives do only:
1. Create the symbol if doesn't exist
2. Try to impose the binding, _without_ demoting.
AND _nothing else_. Because that is NOT a declaration, which must happen explicitly by writing
`<label>:`.

Have to start handling the dot. The requirements are the following:
1. track a "global" dot symbol, which gets updated on demand very often
2. when the dot symbol is met in an expression, allocate a clone symbol of the global dot at that
   instance, which isn't saved into the symbol hash table and has a name `.L0\001` to avoid
   collisions.
3. in fixups we track the "value" of the dot along with its fragment
4. The "value" of the dot is the current fragment fixed offset

Sun Jul  5 18:42:15 CEST 2026

The dot symbol clones should be saved in the same arenas, however by not setting an hash for it we
make it virtually non-existent.

Mon Jul  6 09:18:34 CEST 2026

Now it's time to re-add support for local numeric labels. There is some accounting to be done to
track the number of instances created. Moreover, GNU as doesn't impose limits on how high the
numeric label is, so 9999b is perfectly fine for example. It is implemented as a two arrays: one
tracks the index in which a numeric label appears and is linearly scanned, and after getting the
index we read the second array containing the number of instances.
These arrays are dynamically resized as needed. A small optimization done is that the first ten
numeric labels (0..9) are guaranteed to be the in the first ten entries of the array to ensure quick
lookup.

Following GNU as, I should treat all numbers inside expressions as unsigned unless a specific
negation sign is prefixed. I like the idea, I should then check what implications does it have when
you're close to evaluation.

TODO is also dealing with recursive definitions.

It's very hard to design good APIs, because it's a tradeoff between composability and how many
details you want to expose to the caller. For example, when creating a local numeric label, it
should be set right away with the current section value. Should the function then take a `Section`
as parameter or not? I know that I'm the main consumer of this code, but one of the goals of this
assembler is to not be a nightmare to use as library code, or at least provide a decent starting
point.
Actually, straightforward composability wins.

Jotting down some ideas regarding lexer utilities. In general I'd like to support moving the cursor
back and forth, but some care must be taken to ensure consistency with the "previous" token.
1. Jump back to previous token, explicitly provided -> take a "previous" as input, set it, call advance to set current.
2. Track the latest EOS (newline, semicolon) found; when gargabe is found you jump back to that so
   you start the next line properly.
In general the `previous` field is used rarely in a token cursor, I could drop it for simplicity and
let call sites handle it explicitly.

Ok next in priority is probably a robust handling of the various .equ directives because there is a
lot to learn there, especially due to the relationship between symbols and expressions and deferring
evaluation.

Tue Jul  7 08:57:27 CEST 2026

In GNU as, the .eqv directory completely defers even the dot valuation, so when is met instead of
taking a snapshot of it, the global dot symbol is used instead.

I've to add an `expression_index` field to a symbol, in case a symbol is defined as an expression via
`.set`-like directives. I'm still not sure whether using those indexes instead of pointers has been
a good idea or not.
An `expression_index` results in a smaller `Expression_Node` size, however I need to carry around
the expressions xar everywhere, while with pointers I'd mainly need the root and I can traverse
left-hand side and right-hand size freely. Pointers are already very pervasive in the rest of the
code, it's not like I'm using indexes everywhere. Even inside `Expression_Node` itself I have
left/right pointers for the symbols. I would still allocate them in a separate arena, so I would
need to carry around it to create new symbols. However, traversal would be free of it.
Not high priority, but a thought.

Detecting symbol recursive definitions should probably be done at a later stage of evaluation. For
example, in GNU as, the following doesn't loop:
```asm
.set foo, bar
.set bar, foo
.set foo, 5
.set bar, 5
```

Running with a more complete.s example shows a lot to do still, in terms of basic instruction
support, error handling etc.

Wed Jul  8 14:14:12 CEST 2026

Sometimes the syntax of GNU as can be unexpected, again

```asm
li, t0, t1
```

This produces a relocation because t1 is treated as a symbol and not recognized as a register.

I think I can finally start to look into actually creating the object file, now the assembler
supports a good range of instructions and it's able to ingest a decent program, basically
RV32I/RV64I is supported along with common pseudo-instructions or different formats. It is still a
RV64I assembler though, there are some places to fix in case of a 32-bit target.

In essence, I need to go over and study `write_object_file` from GNU as.

Thu Jul  9 09:35:28 CEST 2026 / Fri Jul 10 10:27:35 CEST 2026

Right after the assembly pass is done, GNU as calls `md_finish`/`riscv_md_finish`, which fills the
content of the `.riscv.attributes` section if required by configuration. I can skip this for now.
Then, if the "relax" option is enabled (which is the default on GNU as), it iters over all section
and calls `riscv_insert_uleb128_fixes`. Given I don't support the `.uleb128` directive yet, I'll
skip this too, but I should keep it in mind. Then, we mainly go inside `write_object_file`.

These will be notes about the `write_object_file` function, which are a longer version of the
initial stub written the 20th of June. It tries to be reasonably exhaustive, while skipping some
details because otherwise it is of no good use.

1. `subseg_finish`: For each section (actually, `stdoutput->section`?), calls `subsegs_finish_section`.
    This latter function is essentially housekeeping for ending sections. For each section, it
    appends a terminal alignment fragment (of which alignment value will be discussed afterwards),
    followed by an empty, sealed frag i.e. `rs_fill` of size zero via `frag_wane`, so that all
    sections end in a uniform shape like `[align_frag][fill-zero frag]` and it can be relied upon.

    The alignment value can be degenerate i.e. set to zero or 1, unless the following hold:

    1. A backend might define `SUB_SEGMENT_ALIGN`, giving its custom value and it can do whatever it
       wants. If that's not the case, and the target defines instead `HANDLE_ALIGN` (that is the
       case for riscv) then the code section gets its defined alignment (usually 2), padding with
       NOPs. This makes the tail of executable sections always diassemblable and execution-safe.
    2. The section is either a `SEC_MERGE | SEC_STRINGS`. The former flag denotes a section which
       content creates a table of fixed size entries of size `sh_entsize`, that can be deduplicated
       by the linker based on their hashed content. Usually `SEC_STRINGS` implies `SEC_MERGE` and in
       such case the `sh_entsize` field expresses the encoding used for the the strings. 1 (byte) of
       C/UTF-8 strings, 2/4 for UTF-16/UTF-32. The linker treats specially the deduplication process
       in case of `SEC_STRINGS`.
       For these section, we compute the _largest_ power-of-two factor of `sh_entsize` and the we
       align based on that value. This process is needed because a section alignment requirement is
       different from its entry stride, and we want to ensure the linker won't read past the
       intended number of bytes and instead reads complete entries. It might be that `sh_entsize`
       isn't a power of two, in such case this would be a "best effort": all machinery is intended
       for power-of-twos, and supporting other type of alignments isn't an option. Later steps warn
       whether the section size isn't a multiple of `sh_entsize`, and the linker would not perform a
       deduplication step for `SEC_MERGE` sections.

    Long explanation! But a lot of concepts here. Note that at this stage, adding warnings about
    section size is still premature since this is pre-relaxation etc.

2. `riscv_pre_output_hook`: contains logic related to `.cfi_remember_state` directive, which I don't
   support so for now I'll skip it.

3. `chain_frchains_together` is called after removing GNU as internal expression and register
   sections: as the name suggest, it chains together the fragments contained in `frchainS` from
   various _subsections_. However, I haven't added support for sub-sections (they're _mostly_
   unused, but I might be wrong), so this step can be skipped. This is actually done in
   `chain_frchains_together_1`, then the caller sets toggles the global `frags_chained`.

4. relaxation/`relax_seg`/`relax_segment`: the core relaxation logic for sections. Other than its
   side-effects, a boolean returns whether it had some effect at all, and it tracks the number of
   passes done. There two main loops here: the outer one, which runs indefinitely, This is an
   endless loop that runs until there is no change with the previous iteration. On _each iteration_,
   we call _for every section_ the `relax_seg` function, which calls the inner `relax_segment`
   function which performs the actual relaxation and checks whether the addresses of fragments in a
   section changed.
   The reason for this double outer loop is the following: cross-section symbols. That is, a section
   might contain references (symbols) defined in other sections e.g. label distances. That means
   addresses within one section may influence the layout of another, so it is required an iteration
   where _all_ section addresses don't change to make sure relaxation is completed.
   Example of cross-section references:
   ```asm
    .text
    start:
        beq   $t0, $t1, far   ; short branch, may grow to long
        # other stuff
    far:
        # other stuff
    end:

    .eh_frame
        .uleb128 end - start  ; size of the function above
   ```
   While I don't support neither `.eh_frame` or `.uleb128` it still gives the idea why this should
   be done.

   Inside `relax_segment`, we iter over the fragments multiple times. There is one special, initial
   iteration that is needed to compute a first estimate (minimum) addresses of the fragments, then
   there is the relaxation loop where we iter multiple times over them. To understand why this
   separation is needed, remember that to relax a fragment correctly we need to know the addresses
   of subsequent fragments (e.g. jump distance). Without a first estimate of the layout, this would
   be impossible!

   For the first loop, we keep track of a cumulative `int region` and `address` that is increased
   while processing the fragments (TODO: expand on `region`). Then the steps are:
   1. Set the current `region` and `address`; bump `address` by the amount of fixed bytes (`fr_fix`)
      within the fragment.
   2. Process the fragment type, and increment the `address` accordingly, some noteworthy types:
      1. For `rs_align`-like fragments we compute the offset (distance) from the current value of
         `address` to the required alignment. If it overcomes the maximum amount, it is set to zero
         (which seems weird to me?). Then, we check and error in case the offset isn't a multiple of
         the expect variable size `fr_var`. We then bump `address` by `offset` and increment `region`.
      2. For `rs_machine_dependent` (e.g. branch) with a symbol (in ras, an expression) attached,
         the `resolve_symbol_value` function is called. It's a lengthy function treated separately.
         I think of it as expression evaluation, with more checks and details.
         Lastly, we increment `address` by the estimation of the relaxed instruction. In case of the
         RISC-V backend, it is calculated the size of branch based on current symbol information
         (`relaxed_branch_length`), and `fr_var` is updated accordingly.
   The previous process computes the fragment addresses, but relaxation loop hasn't started. Now it does.
   Very interestingly, the maximum number of iterations is O(n^2) of the number of fragments
   (`frag_count * frag_count`), capping to O(n) in case of overflows.
   We start iterating again over fragments. We keep track of the cumulative bytes stretched
   `stretch`. Compared to the previous loop, with gave a tentative address to the fragments, now:
   1. We skip `rs_fill`, because it doesn't grow/relax.
   2. For `rs_align` fragments, we track the signed difference between previous and current (based
      on fragment address) alignment bytes needed, establishing how much the fragment needs to grow
      in this iteration. Because it may be that know, taking into account current stretching, less
      bytes of alignment padding might be needed! Then special handling of `.leb128` directives.
   3. The `rs_org` is finally handled. For now I'll ignore it.
   4. The `rs_space`/`rs_space_nop`, is now handled, while previously skipped at the very first
      iteration. The symbol for the repeat count is read, and expected to be an absolute symbol
      (i.e., constant), and giving warning on negative values of it.
   5. The `rs_machine_dependent` is handled by calling the target `md_relax_frag` which in case of
      the RISC-V backend is `riscv_relax_frag` that computes the `relaxed_branch_length` based on
      current information. As with the initial iteration, `fragP->fr_var` is updated but the
      difference from previous iteration is also returned.
   Other relaxation types are handled, but are skipped for now. The algorithm loops until an
   iteration result in no `growth` (neither negative or positive, convergence) or until we cap the
   number of iterations. Lastly, we track whether some addresses have changed at all by looking at
   `last_fr_address` field.

   After relaxation has completed, it marks the global `finalize_syms` as true, meaning that now
   symbols are considered frozen and cannot be changed. It makes sense because otherwise addresses
   can change again.

5. `size_seg`: after relaxation, we can compute the correct fragment size and section _for each
   section_ using this function. The following steps are performed:
   1. For every fragment, call `cvt_frag_to_fill` ("convert fragment to fill fragment"). As the name
      suggests, it mutably changes every fragment into a `rs_fill` variant of fixed size. This is
      done in slightly different ways, depending again on the fragment type:
      1. For `rs_align`/`rs_space` and similar, we call the machine-dependent code `HANDLE_ALIGN`,
         which maps to `riscv_handle_align`. This inserts `nop` padding in case of `rs_align_code`.
      2. For `rs_machine_dependent`, in case of RISC-V the
         `md_convert_frag`/`md_convert_frag_branch` functions are called. This expands a branch into
         the actual instructions. See content for details, but it also handles remaining fixups.
      Different cases end up with adding to `fr_fix` the value of `fr_var`, changing to a `rs_fill`
      and zero other values, so that the fragment is officially sealed and fixed.
   2. The section size is computed by walking the fragment list and taking the address (offset) of
      the last fragment, plus its fixed byte size.
   3. The section is also marked with `SEC_HAS_CONTENTS` in case its size is non-zero and some other
      conditions apply.
   4. NOTE: some architectures require section alignment, but it's not the case for RISC-V, which
      `#define md_section_align(sec, size) size`, so a partion of the bottom code of the function is
      skipped. EDIT (Mon Jul 20 12:58:24 CEST 2026), this makes also the option `no-pad-section`
      useless on the target)

6. `create_object_attributes`: for ELF targets this function is invoked to create a custom section
   `.riscv.attributes`. This contains a lot of ELF-specific stuff so that code should be re-examined
   separately also while running a debugger to check values.

7. `resolve_symbol_value` is called on every _regular_ symbol in the chain.
   How `resolve_symbol_value` works: this function takes a symbol and returns its value, which is most
   often an address. Following steps are taken.

   1. If it is a local symbol, read its `value`. If the symbol is already resolved, return it.
      Otherwise, increase it with the associated fragment address value. If the `finalize_syms` flag is
      provided when calling this function, mark the symbol flag `resolved` and freeze the value.
      Returned the currently computed value.
   2. If not a local symbol but is it resolved, walk through the chain of `X_add_symbol` by adding the
      corresponding `X_add_number` until we hit a local symbol or a constant. Return zero if we find an
      unresolved symbol in between. This seems a weird path but here is some code that leads to that
      codepath:
      ```asm
      .equ base, 100          # base resolved; value = O_constant,       X_add_number=100
      .equ mid,   base + 5    # mid  resolved; value = O_symbol -> base, X_add_number=5
      .equ top,   mid   + 7   # top  resolved; value = O_symbol -> mid,  X_add_number=7

      .equ result, top        # forces resolve_symbol_value on top
      ```
   3. Track the segment of the symbol.
   4. We check whether the symbol is in a `resolving` state. If that's the case, and we're
      `finalize_syms`, we error because it means we've detected a loop we can't escape. This allows
      some early forms of recursive symbol definitions as long that they're resolved later, e.g.:
      ```asm
      .set foo, bar
      .set bar, foo
      .set foo, 5
      .set bar, 5
      ```
      Otherwise, mark the symbol as `resolving` and process its `X_op` field. For machine-dependent
      operations, specific code is run. While some details are omitted, it is very important to note
      that a lot evaluations resulting in adding the fragment address of where the symbol belongs.
      While this makes sense for labels, it might be weird for other symbols, but remember that if
      they're undefined or absolute, they are attached to a dummy fragment with address zero, so it's a
      no-op.
   5. Other operations require recursively calling `resolve_symbol_value`. Lastly, if the global
      `finalize_syms` is set, we set the final computed value.
   6. To be continued, some details are still missing.

   In some sense, this function is a loose equivalent to `expression_evaluate`, while doing some extra
   care. But I will address it later.

8. `resolve_local_symbol_values`: this is called right after looping over _regular_ symbol. Note
   that in there, we might already resolve some local symbols references by regular symbol, however
   some of them might be left out here, and so this functions ensures all of them are processed.

9. `resolve_reloc_expr_symbols`: this is actually a no-op in most cases, unless the `.reloc`
   directive is supported. Hence why even if your code produces relocation, this does nothing.
   Relocation pass through the symbols machinery.

10. `fix_segment/fixup_segment`: called for every section. first, if the global `linkrelax`
    configuration is enabled, then basically return right away. Otherwise, iterate the fixup chain
    for the section and perform the following steps:
    1. If we have a sub symbol, then:
       1. If we have an add and sub symbol in the same section, and we don't have a forced relocation,
       compute the offset difference right away and save it in `fixP->fx_offset`. This can be safely
       after relaxation!
       2. else if we have a sub symbol from the absolute section, then we get its value, we flip its sign,
          and we save it as the number to _add_ (`fixP->fx_offset`).
       3. else if we have a sub symbol in the current section we're processing, we get its offset,
          and we transform the fixup into a PC-relative one `fixP->fx_pcrel = 1`.
    2. If we have an add symbol, then we almost perform the same steps as before, increasing the
       `fixP->fx_offset` by that value.
    3. `md_apply_fix` is finally called. Custom logic to check separately, but tries to check
       whether they're solvable. NOTE: a `RELAX` relocation here can be added if applicable by
       inserting right away a fixup into the chain with such relocation value.

11. Lastly, there is another pass on the symbol table that filters away all unwanted symbols from
    the object file.

Fri Jul 10 15:37:52 CEST 2026

Now that I'm closer to the end of reading GNU as, I can say again that some parts of it are very
clever, and in other parts there are a lot of patches. While there are some generalizations that
reduce the number of codepaths (e.g. made up sections and symbols to re-use some machinery) in other
parts there are so many branches. This is not necessary bad per se but I feel like it could be more
straightforward. While I'm reading about ELF object file through GNU as, I should read a bit better
about the format and its requirements from an official source, and then do what's needed for a basic
assembler.
I realize this assembler, being a first working version, would not be particularly good. I think it
has something good in it, but it can be miles better and cleaner. However this requires a lot of
knowledge and a non-trivial amount of time. There are so many choices to be made, so many details
and relationship with other tooling like compilers, linkers etc.. It's really a non-trivial piece of
software that is run everyday and does so much. My deepest respect for people writing this software!

Fri Jul 10 18:01:50 CEST 2026

TODO(refactor, high) revisit expression allocation strategies.

 Using expression indexes have the only advantage of occupying a small size. This however shouldn't be a concern until
 I have good reasons to. The rest is mostly bad consequences of this:

 1. For both writing AND reading expressions I need the `Expressions` bookkeeper struct. While I'm
    fine to have one for writing, I don't like that I need it for reading, and I would like to just
    read from a pointer. However that is mandatory now because an expression might have some left
    and right side to read and I have to deal read them from the array.
    This creates clanky interfaces sometimes, and reading from the a bookkeeper struct (a Xar)
    actually resulting in some additional hops.
 2. The bookkeeper struct is an eXtensible array, from the base layer. When I read about them I got
    really excited and I wanted to use them. They're great, but as with dynamic arrays, they need to
    be used in the right place at the right time. I feel this is isn't it.
    Since I want to use pointers, a chunk list (like with the symbols table) would probably be the
    best bet!

There is a lot of synergies between symbols and expressions, and that is because symbols can be
defined as expressions, and not necessarily only as labels. Moreover, the left and right hand
side of expressions can be modelled by symbols.
Right now, symbols and expressions are allocated separately, but I wonder whether they should go
closer together somehow, or belong to the same allocator, and perhaps completely identify
expressions with symbols you don't have a name of.
GNU `as` kinda goes in this direction, and also creates a dedicated section for expression
symbols, however I'm wondering whether this definition of a symbol would work:
```c
typedef struct Symbol_2 Symbol_2;
struct Symbol_2
{
        Symbol_Flags     flags;
        Section         *section;
        Fragment        *fragment;
        Symbol_Ref      *symbol;
        Symbol_Ref      *symbol_operand;
        S64              integer_value;
        Expression_Kind  kind;
        // others, like locations etc, which are shared between
        // symbols and expressions.
};
```
Some more context is the following: GNU as distinguishes between a `struct local_symbol` and a
`struct symbol`. The former are created _only_ for _internal local labels_, that is labels that
start with symbols like `.L*`, generated internally for assembler usage and omitted from the
object file unless a flag is explicitly set. This is completely unrelated to the visiblity
(local, global, weak, etc.) of a symbols. Lastly, those symbols will only mainly contain their
value i.e., the offset in their section, which is a `usize`-like.
In contrast, full symbols hold:
1. An `expressionS` struct, which is ~24 bytes;
2. Two additonal `struct symbol` pointers because of double linked list bookkeeping and
   traversal. Since symbol traversal would be done using the `Symbols_Table` chunks, those two
   pointers would not be needed.
   These pointers are used to traverse the symbols table while skipping local symbols, so in a
   more efficient manner. However often the code requires scanning over the local symbols
   too (see `resolve_local_symbol_values`), which is implemented by iterating over every entry
   of the symbol table.
3. Object and target specific fields, which are unneeded for the RISC-V + ELF combo.

This is all to say that the advantage of separating between the two isn't _that_ big of a saving
(around 20 bytes or similar?), at least in this decade and for the targets I'm interest in, but
I understand every memory saving would be precious in the 90s.

Some rough implications of this:
1. Every `Expression_Node` would become a `Symbol_2` pointer, or whatever is this named.
2. These symbols, which should be omitted from the final object file, should be added inside an
   expression section so they can be easily skipped when needed.

Apart from this, more optimizations could be achieved for creating less full symbols: during
parsing, I could peek ahead some binary operations and eagerly collapse into a symbol plus number or
similar.

Sat Jul 11 16:24:25 CEST 2026

Support for the E extension is mainly limiting the amount of registers to 16, so during register
lookup if the number is greater or equal 16, you mark it as missing.

Mon Jul 13 09:45:59 CEST 2026

Regarding the argument about collapsing expressions and symbols together, I think the main drawback
is really polluting the symbol table chunks with at least one expression node per line. Even `addi
x1, x0, 1` would create a symbol in those chunks which are also used for the symbol table.
Otherwise, it would go in whatever space is reserved for storing expressions.

I'm still not very satisfied on the overall error handling strategy. Issues still arise from garbage
input thrown in which isn't correctly handled by the assembler. What I want is no special handling
at all. In practice I don't even want error handling: I should emit no-op placeholders when
something invalid is found (in a ZII fashion), _mark the error_ using diagnostic, and continue. It's
not easy to get exactly right but I think the code would be much easier then. Currently, I'm going
into this direction although it isn't always done very well.

Another non-trivial problem is not marking the same diagnostic error many times. For example, if the
next token to be lexed is invalid, calling `token_peek` and `token_next` will emit the same
diagnostic twice.

TODO: all diagnostic emitted should have a number associated to it.

Tue Jul 14 17:24:52 CEST 2026

Heads down on refactoring the `Fragment` primitive because it's driving me crazy. In general, I'm
writing more a target-first version with the goal of avoid many of GNU as compat problems.

Wed Jul 15 10:46:59 CEST 2026

Some basic notes:

- For the RISC-V target, but in practice many others, the `.nop` and `.nops` directives are
  essentially the same, and can be implemented in the following way:
  - Read a `size` expression, in bytes. If absolute, then directly emit _at least_ `size` bytes of
    NOP instructions. If not, create `rs_space_nop` relax state fragment. This is the only case
    where such relaxation state is used.
- For the RISC-V target, the `rs_align_code` is emitted in two places:
    1. When an `.align` directive is invoked in a code section, without a fill pattern provided.
    2. When adding final section padding during object file creation
- There are different directives for different alignment pattern sizes, e.g. `.balign`, `.balignw`
  (2 bytes), `.balignl` (4 bytes).
- NOP versions are essentially two, a 32-bit one encoded as 0x00000013 and a 16-bit one 0x0001
- TIL that a risc-v processor with the C extension _can mix_ 32-bit and 16-bit instruction, I
  thought they were exclusive. After all, it is an _extension_ :)

Usage of the max alignment in `.align` directives is to ensure that for example a loop or function
starts at a cache-line friendly offset, however only if not too many bytes are required, otherwise
it's not worth it. Cool!

With this in mind, I think I can greatly simplify some of the fragment management, in particular
regarding relaxation states.

- 1. A fill variant which accepts a repeat expression. If not provided, it is intended to be zero.
     Eagerly evaluated at parse time in a `.fill` directive. In GNU as it seems to never happen that
     a fill frag is created with a literal constant value different than zero. If zeroed, it is a
     normal seal. (EDIT Mon Jul 20 09:59:54 CEST 2026: actually in `cvt_frag_to_fill` this is done
     :/).
- 2. An align variant.
- 3. Jump variant.
- 4. The `None` variant. In the end this is needed because we want to mark that a fragment should
     NOT accept more bytes because a variable pattern has already been provided, and would result in
     a nasty bug.

No code versions of the relaxation states, since those can be resolved right away by injecting the
two possible NOP patterns very simply.

Fri Jul 17 14:16:41 CEST 2026

Yesterday and today have been spent implementing the symbol resolution algorithm, in the same
non-recursive fashion of expression evaluation. It is, in every sense, a more generalized expression
evaluation where symbols, whose values are themselves expressions, can happen. Since so much of the
functionality is shared between the two, and the ubiquity of symbol definition within assembly, the
idea is to replace `expression_evaluate` with symbol resolution entirely, with simple flags to skip
symbol resolution if desired. In case only an expression is found, an ephemeral, stack allocated
symbol can be created and the expression is attached to it.
Well, I'm very close to a `expr` section lol

---

Small note on jump/branch immediates, which I did already known but not very precisely. On RISC-V
(but probably other architectures as well), it is not possible to jump on odd addresses, and a
minimum of 2-byte alignment boundary is assumed. Consider now a branch, which has 12-bits of
immediate field. This would allow to express a jump up to `1 << 12`, which is 4096. However, the
first bit of this number should always be zero (must be even!), otherwise it is invalid. The CPU is
optimized for this, and adds to the program the value of 12-bit immediate, multiplied by two. As
such, the true range expressed by a branch immediate is `1 << 13`, which is 8192. In the assembly
code, this translates to being able to write a signed range that must fit in 13 bits, i.e. `[-4096,
4096)` and the number must be even. GNU `as` checks this evenness way later, during fixup
resolution, asserting this with a round trip encoding/decoding that must match:

```c
// riscv.h:224
#define VALID_BTYPE_IMM(x) (EXTRACT_BTYPE_IMM(ENCODE_BTYPE_IMM(x)) == (x))
```

---

Some codepaths depends on the extensions available, most often the compressed extension. In GNU `as`
this is checked with one of the hundreds of globals, however in my implementation I should probably
pass around, very often a `RISCV_Extensions` struct or flags. I don't know whether some extensions
might have arguments or are simply toggles.

---

There are these annoying naming differences between "address" and "offset". Until linker perform its
job, in theory information regarding fragments are "offset" starting from zero, but I'm not sure if
such naming works well and is used consistently.

Sat Jul 18 15:55:22 CEST 2026

Apparently, even emitting bytes for alignment has some special treatment. It could be that the
programmer messes up alignment for a certain certain section, and special symbols inside the
symbols table are emitted so that some tools, like `objdump`, when disassembling, don't show bogus
data but are explicitly marked as invalid bytes. As such, GNU as doesn't immediately error on
finding them.
While there might be some historical reason I'm missing, I don't think it's an appropriate
behaviour. Fragments ending with an odd number of bytes can be flagged out immediately, with
appropriate locations. Same for fragments whose size is a multiple of two but not of four. This can
be addressed in various stages of the assembler, before or after relaxation.


Sun Jul 19 18:11:44 CEST 2026

At the cost with maybe _some_ compatibility with existing assemblers, I think the problem above can be addressed at
a statement level. Consider the following example:

```asm
.text
.byte 1
nop
```

This simple `.byte` directive can completely disagnaling the whole fragment (and possibly others),
making subsequent instructions invalid. This would be valid in case the C-extension:

```asm
.text
.byte 0x01, 0x00
c.nop
```

The above injects two `c.nop` instructions in the `.text` section, where in the first case the
encoding is written explicitly (I might have messed up the order though). With that said, we can
enforce the following rules in a _code_ section:

1. If alignment is 2 (e.g. compressed extension enabled), then a `.byte` directive MUST have an even
   number of expressions.
2. If alignment is 4, then a `.byte` directive MUST have a multiple of 4 number of expressions, and
   a `.short` directive a multiple of two.

Lastly, this can happen also with fill-like directives, which could inject a single byte.

Mon Jul 20 11:20:20 CEST 2026

Notice a detail in GNU as for riscv target: inside the `append_insn` function, if the relocation is
jump-like (that is, a `JAL`-like instruction), then a fixup is not created because it is considered
relaxable. However that is not really expandable/shrinkable _unless_ the compressed extension is
enabled, where a 4-byte jump can become a compressed 2-byte instruction.

---

I'm getting a bit more into creating helper macros for iterators to reduce a little bit of
verbosity. Examples:

```c
#define each_index(array, index)     (U64 index = 0, index < array_count_m(array); index += 1);
#define each_node(first, T, element) (T *element = first; !first; first = first->next);
```

And so on... Because some loops comply to this structure. But I want really consistent loops and not
having to juggle between a lot of them to understand the conditions.

---

The `.riscv.attributes` section is mandatory according to the psABI specification: https://riscv-non-isa.github.io/riscv-elf-psabi-doc/#_attributes

---

The fixup logic is very dense, and further complicates what does it mean to subtract two symbols. I
think there is a total of ~7 conditions or similar that establish whether it is well-defined or not,
because you need to take into account everything: sections, fragments, linker relaxation.

It will probably be one of the challenges, along with proper handling of relocations.

---

A thought I had last week or similar: I think writing a non-toy assembler, which produces proper
relocatable objects, is very non-trivial because of the amount of contextual knowledge you need to
have: you need to have some understanding of a lot of parts regarding the assembler itself, and also
the compiler toolchain in general. It's not like you can implement a certain feature in a isolated
manner, and complete complete the software by chaining together the features. Something like symbol
resolution brings every concept together at it's one of the first thing you actually came across.
