internal ELF_Section_Header_Flags
ELF_Section_Header_Flags__parse(String8 string)
{
        U8  *data = string.data;
        U64  index = 0;
        ELF_Section_Header_Flags result = 0;

        for (;;)
        {
                if (index >= string.count)
                {
                        break;
                }

                switch (data[index])
                {
                case 'a':
                {
                  result |= ELF_Section_Header_Flags__ALLOC;
                } break;
                case 'e':
                {
                  result |= ELF_Section_Header_Flags__EXCLUDE;

                } break;
                case 'o':
                {
                  result |= ELF_Section_Header_Flags__LINK_ORDER;

                } break;
                case 'w':
                {
                  result |= ELF_Section_Header_Flags__WRITE;

                } break;
                case 'x':
                {
                  result |= ELF_Section_Header_Flags__EXECINSTR;

                } break;
                case 'G':
                {
                  result |= ELF_Section_Header_Flags__GROUP;

                } break;
                case 'M':
                {
                  result |= ELF_Section_Header_Flags__MERGE;

                } break;
                case 'S':
                {
                  result |= ELF_Section_Header_Flags__STRINGS;

                } break;
                case 'T':
                {
                  result |= ELF_Section_Header_Flags__TLS;

                } break;
                default:
                {
                        result = ELF_Section_Header_Flags__Invalid;
                } break;
                }

                index += 1;
        }

        return result;
}

internal ELF_Section_Header_Type
ELF_Section_Header_Type__from_String8(String8 string)
{
        ELF_Section_Header_Type result = 0;

        if      (String8__match_exact(string, String8__literal("progbits")))
        {
                result = ELF_Section_Header_Type__Program_Data;
        }
        else if (String8__match_exact(string, String8__literal("nobits")))
        {
                result = ELF_Section_Header_Type__No_Data;
        }
        else if (String8__match_exact(string, String8__literal("note")))
        {
                result = ELF_Section_Header_Type__Notes;
        }
        else if (String8__match_exact(string, String8__literal("init_array")))
        {
                result = ELF_Section_Header_Type__INIT_ARRAY;
        }
        else if (String8__match_exact(string, String8__literal("fini_array")))
        {
                result = ELF_Section_Header_Type__FINI_ARRAY;
        }
        else if (String8__match_exact(string, String8__literal("preinit_array")))
        {
                result = ELF_Section_Header_Type__PREINIT_ARRAY;
        }
        else
        {
                result = ELF_Section_Header_Type__Invalid;
        }

        return result;
}

internal U8
ELF_Symbol_Type__from_String8(String8 string)
{
        U8 result = 0;

        if (String8__match_exact(string, String8__literal("notype")))
        {
                result = ELF_Symbol_Type__None;
        }
        else if (String8__match_exact(string, String8__literal("object")))
        {
                result = ELF_Symbol_Type__Object;
        }
        else if (String8__match_exact(string, String8__literal("function")))
        {
                result = ELF_Symbol_Type__Function;
        }
        else if (String8__match_exact(string, String8__literal("section")))
        {
                result = ELF_Symbol_Type__Section;
        }
        else if (String8__match_exact(string, String8__literal("file")))
        {
                result = ELF_Symbol_Type__File;
        }
        else if (String8__match_exact(string, String8__literal("common")))
        {
                result = ELF_Symbol_Type__Common;
        }
        else if (String8__match_exact(string, String8__literal("tls_object")))
        {
                result = ELF_Symbol_Type__TLS;
        }
        else if (String8__match_exact(string, String8__literal("gnu_indirect_function")))
        {
                result = ELF_Symbol_Type__GNU_Indirect_Function;
        }

        return result;
}

internal void
ELF_identifier_fill(U8 identifier[ELF_id_size])
{
        identifier[ELF_ID_Magic__Index]     = ELF_ID_Magic__0;
        identifier[ELF_ID_Magic__Index + 1] = ELF_ID_Magic__1;
        identifier[ELF_ID_Magic__Index + 2] = ELF_ID_Magic__2;
        identifier[ELF_ID_Magic__Index + 3] = ELF_ID_Magic__3;

        identifier[ELF_ID_Class__Index]     = ELF_ID_Class__64;
        identifier[ELF_ID_Data__Index]      = ELF_ID_Data__2LSB;
        identifier[ELF_ID_Version__Index]   = ELF_ID_Version__Current;

        identifier[ELF_ID_OS_ABI__Index]    = ELF_ID_OS_ABI__Linux;

        return;
}
