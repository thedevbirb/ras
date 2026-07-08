#define RISCV_FAKE_LABEL_NAME ".L0 "

internal Symbol_Ref *
Symbols_Table__internal_label(Symbols_Table *symbols_table, Section *section)
{
        String8 name = String8__literal(RISCV_FAKE_LABEL_NAME);
        Symbol_Ref *result = Symbols_Table__create(symbols_table, name);
        Symbol_Ref__update_section(result, section);
        return result;
}
