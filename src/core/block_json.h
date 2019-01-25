#ifndef __BLOCK_JSON__
#define __BLOCK_JSON__
namespace JSON
{
	struct Stream
	{
		const char *data;
		u32 offset;
		u32 line;
		u32 character;

		// The current character under the stream.
		inline char get() const { return data[offset]; }
		// Go to the next character and return true if end of stream.
		bool step();
		// Go to the next non-whitespace character. 
		bool next();
	};

	enum class ValueType
	{
		J_UNDEFINED,
		J_NULL,
		J_STRING,
		J_NUMBER,
		J_OBJECT,
		J_ARRAY,
		J_BOOL,
	};

	struct Value
	{
		ValueType type;

		u32 length() const;
		Value operator[] (u32 i) const;
		Value operator[] (const char *key) const;

		operator f32()
		{
			ASSERT(type == ValueType::J_NUMBER);
			return (f32) number;
		}

		operator f64()
		{
			ASSERT(type == ValueType::J_NUMBER);
			return number;
		}

		operator bool() const
		{
			if (type == ValueType::J_NULL || type == ValueType::J_UNDEFINED)
				return false;
			if (type == ValueType::J_BOOL)
				return boolean;
			return true;
		}

		operator char *() const
		{
			ASSERT(type == ValueType::J_STRING);
			return (char *) string.data;
		}

		operator const char *() const
		{
			ASSERT(type == ValueType::J_STRING);
			return string.data;
		}

		union
		{
			// Objects
			struct {
				u32 num_values;
				Value *hashes;
				Value *values;

			} object;
			// Arrays
			struct {
				u32 num_values;
				Value *values;
			} array;
			// Strings
			struct {
				u32 hash;
				u32 length;
				const char *data;
			} string;
			// Numbers
			f64 number;
			// Bools
			bool boolean;
		};
	};

	// A crude debug log for writing out parsed Values.
	void pretty_print(Value value, u32 ident=0);

	// NOTE: This should maybe be moved to the block_string.h header.
	//
	// Turn a string of characters into a number, dependent on the 
	// bytes in the string.
	u32 string_hash(const char *string);

	// Parse what the json standard calls a value.
	Value parse_object(const char *string);
	Value parse_object(Stream *stream);

	// Parse different kinds of values. A meta construct.
	Value parse_value(Stream *stream);
	Value parse_string(Stream *stream);
	Value parse_number(Stream *stream);
	Value parse_array(Stream *stream);
	Value parse_boolean(Stream *stream);
}

#endif
