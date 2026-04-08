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

TODO: the parser builds statements as it goes line by line, it should have a statement builder that
can be used during expression parsing to already catch some errors.

# relaxation

Now there is a tough part, where you evaluate every expression and compute sizes and offsets for
each section. It's surprising that this requires a fixed point iterative algorithm with a
potentially very large number of iterations over all the program statements.

TODO: limit what instructions and directives can appear in .bss section.
TODO: handling and evaluation of relocation operators, plus emitting relocation entries somewhere.

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
symbol_b`) so that it can be written with relocations if needed.


# Random

Just found out that there are some good docs that I just skipped :) https://sourceware.org/cgit/binutils-htdocs/commit/?id=30b032c8ecd7b53d995058be3faf6c031e229de5

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

