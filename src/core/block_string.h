#ifndef __BLOCK_STRING__
#define __BLOCK_STRING__

// TODO: This parser needs to be made more competent.
//
// I would like to have actually good number parsing.

// A pointer to the function that gives a timestamp for when 
// the file was last edited.
u64 (*file_timestamp)(const char *file);
// Copy the whole file to memory.
const char *(*read_entire_file)(const char *file);

// A pointer to a pointer, this is honestly just silly.
typedef char ** CharStream;
#define to_stream(c) (char **) (&c)

struct Token
{
	const char *data;
	u32 length;
};

bool str_eq(const char *_a, const char *_b, u32 len=-1)
{
	u32 i = 0;
	char *a = (char *) _a;
	char *b = (char *) _b;
	while (i < len)
	{
		if (!*a || !*b) break;
		if (*(a++) != *(b++)) return false;
		i++;
	}
	return true;
}

bool str_eq(Token _a, Token _b)
{
	u32 i = 0;
	u32 len = minimum(_a.length, _b.length);
	char *a, *b;
	a = (char *) _a.data;
	b = (char *) _b.data;
	while (i < len)
	{
		if (*a == '\0') break;
		if (*b == '\0') break;
		if (*(a++) != *(b++)) return false;
		i++;
	}
	return true;
}

bool str_token_eq(Token _a, const char *b)
{
	u32 i = 0;
	u32 len = _a.length;
	char *a;
	a = (char *) _a.data;
	while (i < len)
	{
		if (*(a++) != *(b)) return false;
		if (*(b++) == '\0') return false;
		i++;
	}
	return true;
}

inline
u32 str_len(const char *str)
{
	u64 length = 0;
	while (str[length++] != '\0');
	return length - 1;
}

char *str_copy(const char *src)
{
	u32 len = str_len(src);
	char *s = (char *) src;
	char *out = push_array(char, len + 1);
	while (*s) 
		*(out++) = *(s++);
	*(out++) = *(s++);
	return (out - len - 1);
}

inline
char curr(CharStream stream)
{
	return (*(*stream));
}

inline 
void inc(CharStream stream)
{
	(*stream)++;
}

inline 
void skipp(CharStream stream, u32 n)
{
	(*stream) += n;
}

inline 
char at(CharStream stream, u64 index)
{
	return (*stream)[index];
}

bool is_whitespace(char c)
{
	//return c != '\0' && c <= ' ';
	u8 n = (u8) c;
	switch (n)
	{
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 32:
		case 133:
			return true;
		default:
			return false;
	}
}

bool is_char(char c)
{
	return ' ' < c;
}

bool is_number(char c)
{
	return '0' <= c && c <= '9';
}

bool is_letter(char c)
{
	return 'A' <= c && c <= 'z';
}

bool find(CharStream stream, char c)
{
	while (true)
	{
		char c = curr(stream);
		if (c == '=')
			return true;
		if (c == '\n' || c == '\0')
			return false;
		inc(stream);
	}
}

void eat_spaces(CharStream stream)
{
	while (is_whitespace(curr(stream))) (*stream)++;
}

s32 eat_int(CharStream stream)
{
	eat_spaces(stream);
	bool negative = curr(stream) == '-';
	if (negative) inc(stream);
	s32 accum = 0;
	while (is_number(curr(stream)))
	{
		accum *= 10;
		accum += curr(stream) - '0';
		inc(stream);
	}
	return negative ? -accum : accum;
}

Token eat_word(CharStream stream)
{
	eat_spaces(stream);
	Token word = {(*stream), 0};
	while (is_char((*stream)[word.length])) word.length++;
	*stream += word.length;
	return word;
}

void skipp_line(CharStream stream)
{
	char c = curr(stream);
	while (c)
	{
		if (c == '\n')
		{
			while (c == '\n' || c == '\r')
			{
				inc(stream);
				c = curr(stream);
			}
			return;
		}
		inc(stream);
		c = curr(stream);
	}
}

inline
void free_string(const char *content)
{
	pop_memory((void *) content);
}

const char *copy_token_to_string(Token token)
{
	char *str = push_array(char, token.length);
	for (u32 i = 0; i < token.length; i++)
	{
		str[i] = token.data[i];
	}
	return (const char *) str;
}

#endif
