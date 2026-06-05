#ifndef OBJECT_ELF_UTILS_H
#define OBJECT_ELF_UTILS_H

global const String8 ELF_Section_Header_Flags__string = String8__literal("aeowxEGMST");

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

#endif // OBJECT_ELF_UTILS_H

