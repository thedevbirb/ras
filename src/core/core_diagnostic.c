internal Diagnostics *
Diagnostics__new(Arena *arena)
{
        Diagnostics *result = Arena__push_struct_m(arena, Diagnostics);
        Diagnostic  *dummy  = Arena__push_struct_m(arena, Diagnostic);

        *result = (Diagnostics)
        {
                .arena = arena,
                .dummy = dummy,
                .errors_max = Diagnostics__errors_max_default,
        };
        return result;
}

internal void
set_color(FILE *f, Diagnostic_Style s)
{
        if (s.bold)
        {
                fprintf(f, "\x1B[1;%um", s.color);
        }
        else
        {
                fprintf(f, "\x1B[%um", s.color);
        }
}

internal void
reset_color(FILE *f)
{
        fprintf(f, "\x1B[0m");
}

internal void
print_recap_line(U8 *name, U32 line_number, U32 column_number, Diagnostic_Kind kind, DG code)
{
        set_color(stderr, Diagnostic_Style__default_bold);
        fprintf(stderr, "%s:%d:%d:", name, line_number, column_number);
        reset_color(stderr);
        fprintf(stderr, " ");
        set_color(stderr, diagnostic_styles[kind]);
        if (kind == Diagnostic_Kind__Note)
        {
                fprintf(stderr, "%s", diagnostic_labels[kind]);
        }
        else
        {
                fprintf(stderr, "%s[E%05d]", diagnostic_labels[kind], code);
        }
        reset_color(stderr);
        set_color(stderr, Diagnostic_Style__default_bold);
        fprintf(stderr, ": ");
        reset_color(stderr);
}

internal void
print_raw_message(Diagnostic_Kind kind, String8 message)
{
        set_color(stderr, Diagnostic_Style__default_bold);
        fprintf(stderr, "ras");
        reset_color(stderr);
        fprintf(stderr, " ");
        set_color(stderr, diagnostic_styles[kind]);
        fprintf(stderr, "%s:", diagnostic_labels[kind]);
        reset_color(stderr);
        fprintf(stderr, " ");
        set_color(stderr, Diagnostic_Style__default_bold);
        fprintf(stderr, "%*s\n",  String8__varg(message));
        reset_color(stderr);
        return;
}

// TODO(feature): support multiple sources.
internal void
Diagnostic__print(Diagnostic *diagnostic, Source *source, Arena *arena)
{
        // Fill those indexes lazily if it hasn't been done already.
        if (!source->line_start_indexes)
        {
                U32 index   = 0;
                U32 counter = 1;
                for (;;)
                {
                        if (index >= source->count)
                        {
                                break;
                        }
                        counter += source->data[index] == '\n';
                        index   += 1;
                }

                source->line_start_indexes = Arena__push_array_m(arena, U32, counter);
                source->line_start_count   = counter;

                index   = 0;
                counter = 1;
                for (;;)
                {
                        if (index >= source->count)
                        {
                                break;
                        }
                        if (source->data[index] == '\n')
                        {
                                source->line_start_indexes[counter] = index + 1;
                                counter += 1;
                        }
                        index += 1;
                }
        }

        U32 row_index        = floor_search(source->line_start_indexes, source->line_start_count, diagnostic->location);
        U32 line_start_index = source->line_start_indexes[row_index];
        U32 column_index     = diagnostic->location - source->start_offset_logical - line_start_index;
        U32 line_number      = row_index    + 1;
        U32 column_number    = column_index + 1;

        print_recap_line(source->name, line_number, column_number, diagnostic->kind, diagnostic->code);
        if (diagnostic->kind <= Diagnostic_Kind__Warning)
        {
                set_color(stderr, Diagnostic_Style__default_bold);
                fprintf(stderr, "%s\n",  diagnostic->message.data);
                reset_color(stderr);
        }
        else
        {
                fprintf(stderr, "%s\n",  diagnostic->message.data);
        }
        fprintf(stderr, "%5d | ", line_number);

        // Print the actual line
        U8 *line = &source->data[line_start_index];
        U32 newline_index = 0;
        U32 index = 0;
        for (;;)
        {
                U8 character = line[index];
                character = character == '\t' ? ' ' : character;
                fputc(character, stderr);
                if (character == '\n')
                {
                        newline_index = index;
                        break;
                }
                index += 1;
        }

        // Print the caret and the ranges.
        fprintf(stderr, "      | ");
        index = 0;
        U8 character = ' ';
        set_color(stderr, (Diagnostic_Style){ .color = Diagnostic_ANSI_Color_Green, .bold = 1 });
        for (;;)
        {
                if (index == column_index)
                {
                        character = '^';
                }

                for (U32 j = 0; character == ' ' && j < array_count_m(diagnostic->ranges); j++)
                {
                        // Ranges are locations
                        Range1_U32 range = diagnostic->ranges[j];
                        if (Range1_U32__contains(range, source->start_offset_logical + line_start_index + index))
                        {
                                character = '~';
                        }
                }


                fputc(character, stderr);

                if (index == newline_index)
                {
                        break;
                }

                index += 1;
                character = ' ';
        }
        reset_color(stderr);
        fprintf(stderr, "\n");

        return;
}

internal Diagnostic *
Diagnostics__push(Diagnostics *diagnostics, DG code)
{
        Diagnostic *result = diagnostics->dummy;
        DG_Info info = DG_Info__table[code];

        B32 emit = !diagnostics->count_max || diagnostics->count < diagnostics->count_max;
        if (info.severity == Diagnostic_Kind__Error)
        {
                emit = emit && (!diagnostics->errors_max || diagnostics->errors_count < diagnostics->errors_max)
                            && (!diagnostics->limit_max || diagnostics->limit_count < diagnostics->limit_max);

                if (emit)
                {
                        diagnostics->limit_count += 1;
                        diagnostics->errors_count += 1;
                }
        }

        if (emit)
        {
                result = Arena__push_struct_m(diagnostics->arena, Diagnostic);
                DLL_push_back_m(diagnostics->first, diagnostics->last, result);

                diagnostics->count += 1;
        }

        result->code    = code;
        result->kind    = info.severity;
        result->message = info.message;

        return result;
}
