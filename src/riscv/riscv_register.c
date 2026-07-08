internal Register
Register_List__lookup(Register_List register_list, String8 string)
{
        // TODO: I don't like this at all.
        U64 index = 0;
        B32 found = 0;
        for (;;)
        {
                B32 break_should = found || index >= register_list.count;
                if (break_should)
                {
                        break;
                }
                found = String8__match_exact(register_list.data[index].name, string);
                index += 1;
        }
        index -= (U64)(found && index > 0);
        assert_always_m(!found || index < register_list.count);
        Register result = found ? register_list.data[index] : Register__invalid;

        return result;
}
