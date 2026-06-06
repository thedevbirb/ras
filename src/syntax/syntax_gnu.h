#ifndef SYNTAX_GNU_H
#define SYNTAX_GNU_H

// Remove quotes.
internal String8
token_string_content(String8 token_string)
{

	String8 result = {0};
	result = String8__skip(token_string, 1);
	result = String8__chop(result, 1);
	return result;
}

#endif // SYNTAX_GNU_H
