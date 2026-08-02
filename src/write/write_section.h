#ifndef WRITE_SECTION_H
#define WRITE_SECTION_H

// Finish the given section ensuring the last fragment in it has the tail `[alignment fragment][zero-fill fragment]`.
// This is done for two main reasons:
// 1. Ensure all sections have a consistent ending layout which can be relied upon.
// 2. Add final alignment to the sections that might need it. For an example, a code section should end up with a proper
//    alignment of NOPs to make execution and disassembly safe, while for table sections (`SEC_MERGE | SEC_STRINGS`)
//    it's good to ensure alignment so that there is no entry of invalid byte size and other tools (like a linker) don't
//    read over because they're assuming a certain size and less bytes have been written.
internal void
Section__finish(Section *section);

// Compute the total size of the instructions needed to relax the jump.
internal U8
Fragment__jump_instructions_total_size(Fragment *fragment, Section *section);

// Perform the relaxation algorithm on the given section.
internal B32
Section__relax(Section *section, Arena *arena, Diagnostics *diagnostics);

#endif // WRITE_SECTION_H

