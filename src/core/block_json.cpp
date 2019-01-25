#include "block_json.h"

#define STREAM_ERROR(s, msg, ...)\
	{\
	print("!! ERROR (%d:%d): ", (s).line, (s).character);\
	print(msg, ##__VA_ARGS__);\
	print("\n");\
	return {ValueType::J_UNDEFINED};\
	}\

#define STREAM_ERROR_EOF(s)\
	STREAM_ERROR(s, "Unexpected EOF");

namespace JSON
{
	bool Stream::step()
	{
		offset++;
		char c = get();
		if (c == '\0')
		{
			offset--;
			return true;
		}
		if (c == '\r')
		{
			return step();
		}
		else if (c == '\n')
		{
			character = 1;
			line++;
		}
		else
		{
			character++;
		}
		return false;
	}

	bool Stream::next()
	{
		while (is_whitespace(get()))
		{
			bool end_of_file = step();
			if (end_of_file)
				return true;
		}

		return get() == '\0';
	}

	u32 string_hash(const char *string)
	{
		// one_at_a_time hash from Jenkins.
		u32 hash = 234;
		const char *c = string;
		while (*c)
		{
			hash += *c;
			hash += hash << 10;
			hash ^= hash >> 6;
			c++;
		}
		hash += hash << 3;
		hash ^= hash >> 11;
		hash += hash << 15;
		return hash;
	}

	u32 Value::length() const
	{
		// This only makes sense for objects and arrays.
		if (type == ValueType::J_OBJECT)
			return object.num_values;
		if (type == ValueType::J_ARRAY)
			return array.num_values;
		ASSERT(false);
		return 0;
	}

	Value Value::operator[] (u32 i) const
	{
		ASSERT(i <= length());
		return array.values[i];
	}

	Value Value::operator[] (const char *key) const
	{
		ASSERT(type == ValueType::J_OBJECT);
		u64 hash = string_hash(key);
#if 1 // A binary search that didn't really work, somehow?
		for (u32 i = 0; i < length(); i++)
		{
			if (hash == object.hashes[i].string.hash)
			{
				return object.values[i];
			}
		}
		return {};
#else
		u32 low = 0;
		u32 high = length();
		while (low < high)
		{
			u32 p = (low + high) / 2;
			if (object.hashes[p].string.hash < hash)
			{
				low = p - 1;
				continue;
			}
			else if (object.hashes[p].string.hash > hash)
			{
				high = p + 1;
				continue;
			}
			else
			{
				return object.values[p];
			}
		}
		return {};
#endif
	}

	Stream stream_from_string(const char *string)
	{
		Stream stream = {};
		stream.line = 1;
		stream.character = 1;
		stream.data = string;
		return stream;
	}

	void print_indent(u32 indent)
	{
		for (u32 i = 0; i < indent; i++)
			print("  ");
	}

	void pretty_print(Value value, u32 indent)
	{
		print_indent(indent);
		switch (value.type)
		{
			case ValueType::J_UNDEFINED:
				print("undefined\n");
				return;
			case ValueType::J_NULL:
				print("null\n");
				return;
			case ValueType::J_STRING:
				print("\"%s\"\n", value.string.data);
				return;
			case ValueType::J_NUMBER:
				print("%f\n", value.number);
				return;
			case ValueType::J_OBJECT:
				print("{\n");
				for (u32 i = 0; i < value.length(); i++)
				{
					print_indent(indent+1);
					print("\"%s\" :\n", value.object.hashes[i].string.data);
					pretty_print(value.object.values[i], indent+2);
				}
				print_indent(indent);
				print("}\n");
				return;
			case ValueType::J_ARRAY:
				print("[\n");
				for (u32 i = 0; i < value.length(); i++)
				{
					pretty_print(value[i], indent+1);
				}
				print_indent(indent);
				print("]\n");
				return;
			case ValueType::J_BOOL:
				if (value.boolean)
					print("true\n");
				else
					print("false\n");
				return;
		};
	}

	Value parse_array(Stream *stream)
	{
		stream->next();
		List<Value> values = create_list<Value>(1);
		if (stream->get() != '[')
			STREAM_ERROR(*stream, "Invalid start to array, expected '[', but got '%c'", stream->get());
		while (true)
		{
			if (stream->step())
				STREAM_ERROR_EOF(*stream);

			char c = stream->get();
			if (c == ']')
			{
				stream->step();
				break;
			}

			stream->next();
			Value value = parse_value(stream);
			values.append(value);

			stream->next();
			c = stream->get();
			if (c == ',')
			{
				stream->step();
				continue;
			}
			else if (c == ']')
			{
				stream->step();
				break;
			}
			else
				STREAM_ERROR(*stream, "Invalid array, expected ']' or ',', got '%c'\n", c);
		}
		Value result = {ValueType::J_ARRAY};
		result.array.num_values = values.length;
		result.array.values = values.data;
		return result;
	}

	Value parse_object(const char *string)
	{
		Stream stream = stream_from_string(string);
		return parse_object(&stream);
	}

	Value parse_object(Stream *stream)
	{
		static u32 counter = 0; 
		counter++;
		if (stream->next())
			STREAM_ERROR_EOF(*stream);

		if ('{' != stream->get())
			STREAM_ERROR(*stream, "Expected '{' at start of object");

		List<Value> hashes = create_list<Value>(10);
		List<Value> values = create_list<Value>(10);
		while (true)
		{
			stream->step();
			if (stream->next())
				STREAM_ERROR_EOF(*stream);

			char c = stream->get();
			if (c == '}')
			{
				break;
			}
			else if (c == '\"')
			{
				Value key = parse_string(stream);
				if (key.type == ValueType::J_NULL)
					STREAM_ERROR(*stream, "Invalid key or malformed string\n");
				ASSERT(key.type == ValueType::J_STRING);
				if (stream->next())
					STREAM_ERROR_EOF(*stream);
				if (stream->get() != ':')
					STREAM_ERROR(*stream, "Invalid object, expected ':', but got '%c'", stream->get());
				stream->step();
				stream->next();
				Value value = parse_value(stream);

				u32 hash = key.string.hash;
				// Insert it sorted into the list
				for (u32 i = 0; i <= hashes.length; i++)
				{
					if (i == hashes.length || hashes[i].string.hash < key.string.hash)
					{
						hashes.insert(i, key);
						values.insert(i, value);
						break;
					}
				}
			} 
			else 
			{
				STREAM_ERROR(*stream, "Unexpected symbol, expected a string");
			}
		}

		stream->step();
		Value result = {ValueType::J_OBJECT};
		result.object.num_values = hashes.length;
		result.object.hashes = hashes.data;
		result.object.values = values.data;
		return result;
	}

	Value parse_value(Stream *stream)
	{
		if (stream->next())
			STREAM_ERROR_EOF(*stream);
		char c = stream->get();
		switch (c)
		{
			case '{':
				return parse_object(stream);
			case '[':
				return parse_array(stream);
			case '\"':
				return parse_string(stream);
			default:
				return parse_number(stream);
			case 't': // Assume true.
				{
					for (u32 i = 0; i < 4; i++)
						if (stream->step())
							STREAM_ERROR_EOF(*stream);
					Value value = {ValueType::J_BOOL};
					value.boolean = true;
					return value;
				}
			case 'f': // Assume false.
				{
					for (u32 i = 0; i < 4; i++)
						if (stream->step())
							STREAM_ERROR_EOF(*stream);
					Value value = {ValueType::J_BOOL};
					value.boolean = false;
					return value;
				}
			case 'n': // Assume null.
				{
					for (u32 i = 0; i < 4; i++)
						if (stream->step())
							STREAM_ERROR_EOF(*stream);
					return {ValueType::J_NULL};
				}
		}
	}

	Value parse_number(Stream *stream)
	{
		// TODO: Parse scientific notation, currently it doesn't have it, but
		// it might need to be added.
		if (stream->next())
			STREAM_ERROR_EOF(*stream);
		bool negative;
		if (stream->get() == '-')
		{
			negative = true;
			stream->step();
		}
		else
		{
			negative = false;
		}

		u32 whole = 0;
		u32 divisor = 1;
		u32 fraction = 0;
		do 
		{
			char c = stream->get();
			if ('0' <= c && c <= '9')
			{
				whole *= 10;
				whole += (c - '0');
			}
			else if (c == '.')
			{
				break;
			}
			else
			{
				goto JSON_STRING_LEAVE;
			}
		}
		while (!stream->step());
		stream->step();
		
		do 
		{
			char c = stream->get();
			if ('0' <= c && c <= '9')
			{
				divisor *= 10;
				fraction *= 10;
				fraction += (c - '0');
			}
			else
			{
				break;
			}
		}
		while (!stream->step());

JSON_STRING_LEAVE:
		f64 n = (whole + ((f64) fraction / (f64) divisor));
		n = negative ? -n : n;
		Value value = {ValueType::J_NUMBER};
		value.number = n;
		return value;
	}

	Value parse_string(Stream *stream)
	{
		if (stream->next())
			STREAM_ERROR_EOF(*stream);
		if (stream->get() != '\"')
			STREAM_ERROR(*stream, "Invalid string, expected '\"' as first character in string");
		// NOTE: Maybe count the string beforehand?
		List<char> string = create_list<char>(20);
		while (true)
		{
			if (stream->step())
				STREAM_ERROR_EOF(*stream);
			char c = stream->get();
			if (c == '\"')
				break;
			if (c == '\\')
			{
				stream->step();
				c = stream->get();
				switch (c)
				{
					case '\"':
						c = '\"';
						break;
					case '\\':
						c = '\\';
						break;
					case '/':
						c = '/';
						break;
					case 'b':
						c = '\b';
						break;
					case 'f':
						c = '\f';
						break;
					case 'n':
						c = '\n';
						break;
					case 'r':
						c = '\r';
						break;
					case 't':
						c = '\t';
						break;
					case 'u':
						STREAM_ERROR(*stream, "Unicode symbols are not supported");
						break;
				}
			} 
			string.append(c);
		}
		string.append('\0');
		stream->step();
		Value value = {ValueType::J_STRING};
		value.string.length = string.length;
		value.string.data = string.data;
		value.string.hash = string_hash(string.data);
		return value;
	}
};
