#ifndef WRITE_OBJECT_H
#define WRITE_OBJECT_H

internal U64
write_object_file
(
        Arena           *arena,
        Diagnostics     *diagnostics,
        Expressions     *expressions,
        Symbols_Table   *symbols_table,
        Options         *options,
        S32              file_descriptor_out
);

#endif // WRITE_OBJECT_H

