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
