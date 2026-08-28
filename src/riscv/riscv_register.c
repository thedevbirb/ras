internal const Register *
Register_List__lookup(Register_List register_list, String8 string, B32 embedded_is)
{
        U64 index = 0;
        const Register *result = 0;
        for (;;)
        {
                B32 break_should = result || index >= register_list.count;
                if (break_should)
                {
                        break;
                }
                const Register *current = &register_list.data[index];
                B32 found = String8__match_exact(current->name, string);
                if (found)
                {
                        result = current;
                }
                index += 1;
        }
        assert_always_m(!result || index <= register_list.count);

        if (embedded_is && result->number >= 16)
        {
                result = 0;
        }

        return result;
}

internal const Register_CS *
Register_CS_List__lookup(Register_CS_List csr_register_list, String8 string)
{
        U64 index = 0;
        const Register_CS *result = 0;
        for (;;)
        {
                B32 break_should = result || index >= csr_register_list.count;
                if (break_should)
                {
                        break;
                }
                const Register_CS *current = &csr_register_list.data[index];
                B32 found = String8__match_exact(current->name, string);
                if (found)
                {
                        result = current;
                }
                index += 1;
        }
        assert_always_m(!result || index <= csr_register_list.count);

        return result;
}
