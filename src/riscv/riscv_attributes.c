internal B32
RISCV_Tag__is_ntbs(RISCV_Tag tag)
{
        B32 result = tag % 2 != 0;
        return result;
}

internal void
RISCV_Attributes_List__add(RISCV_Attributes_List *list, RISCV_Tag tag, U64 value_u, U8 *value_s)
{
        if (list->count < RISCV_Attributes_List__max)
        {
                RISCV_Attribute *attribute = &list->data[list->count];
                *attribute = (RISCV_Attribute)
                {
                        .tag = tag,
                        .value_u = value_u,
                        .value_s = value_s
                };
                list->count += 1;
        }

        return;
}
