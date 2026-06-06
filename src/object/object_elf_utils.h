#ifndef OBJECT_ELF_UTILS_H
#define OBJECT_ELF_UTILS_H

#define ELF_Section_Header_Flags__cstring  "aeowxEGMST"
global const String8 ELF_Section_Header_Flags__string8 = String8__literal(ELF_Section_Header_Flags__cstring);

// TODO: incomplete compared to what GNU as does.
internal ELF_Section_Header_Flags
ELF_Section_Header_Flags__parse
(
	String8 string
)
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

internal U32
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

#endif // OBJECT_ELF_UTILS_H

