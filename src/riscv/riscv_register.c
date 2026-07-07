U8
register_lookup(String8 string)
{
        U8 register_value = register_invalid;
        S8 index    = 0;
        B32 found   = 0;
        S32 result  = -1;
        for (;;)
        {
                result = memory_match(string.data, (unsigned char *)register_map[index].name, string.count);
                found = result == 0;
                B32 break_should = index >= (S8)register_map_size || found;
                if (break_should)
                {
                        break;
                }
                index += 1;

        }
        register_value = found ? register_map[index].number : register_invalid;

        return register_value;
}
