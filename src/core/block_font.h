#ifndef __BLOCK_FONT__
#define __BLOCK_FONT__
#define KERNING_TABLE_SIZE 500


u32 hash(u8 a, u8 b)
{
	// This is the cantor pairing function. 
	// I don't know how well it performce but
	// in theory it's a good idea.
	u32 hash = (((a + b) * (a + b + 1) >> 2) + b) % KERNING_TABLE_SIZE;
	ASSERT(hash < KERNING_TABLE_SIZE);
	return hash;
}

u32 hash(Kerning kerning)
{
	return hash(kerning.first, kerning.second);
}

f32 find_kerning(Font font, s8 a, s8 b)
{
	u32 key = hash(a, b);
	Kerning *curr = font.kernings + key;
	while (curr->first != a && curr->second != b)
	{
		curr = curr->next;
		if (!curr)
			return 0.0;
	}
	return curr->amount;
}

#define find_eq(s)\
	if (!find(s, '='))\
	{\
		print("Failed to load font file %s\n", path);\
		return {};\
	}\
	inc(contents);

// NOTE: This requires the width and height of the font texture.
Font parse_font(const char *path, AssetID texture)
{
	Font font;
	font.glyphs   = create_list<Glyph>(256);
	font.kernings = push_array(Kerning, KERNING_TABLE_SIZE);
	font.texture  = texture;
	zero_array(font.kernings, Kerning, KERNING_TABLE_SIZE);

	f32 width  = (f32) find_asset(texture).texture.width;
	f32 height = (f32) find_asset(texture).texture.height;

	const char *file = read_entire_file(path);
	CharStream contents = to_stream(file);
	while (curr(contents))
	{
		Token phrase = eat_word(contents);
		if (str_token_eq(phrase, "char"))
		{
			// It's a character
			// Skipp the first part, we don't care about the id.
			find_eq(contents);
			u8 id = eat_int(contents);

			// Read in the glyph data.
			Glyph g;
			find_eq(contents);
			g.x = ((f32) eat_int(contents)) / width;
			find_eq(contents);
			g.y = ((f32) eat_int(contents)) / height;
			find_eq(contents);
			g.w = ((f32) eat_int(contents)) / width;
			find_eq(contents);
			g.h = ((f32) eat_int(contents)) / height;
			find_eq(contents);
			g.x_offset = ((f32) eat_int(contents)) / width;
			find_eq(contents);
			g.y_offset = ((f32) eat_int(contents)) / height;
			find_eq(contents);
			g.advance = ((f32) eat_int(contents)) / width;

			ASSERT(id < font.glyphs.capacity);
			font.glyphs[id] = g;
		}
		else if (str_token_eq(phrase, "kerning"))
		{
			// It's kerning
			Kerning k = {};
			find_eq(contents);
			k.first = eat_int(contents);

			find_eq(contents);
			k.second = eat_int(contents);

			find_eq(contents);
			k.amount = eat_int(contents) / width;

			// Hastable insert
			u32 key = hash(k);
			// TODO: Rewrite this with ptr_ptr's
			if (font.kernings[key].amount)
			{
				// A key exists.
				Kerning *c = font.kernings + key;
				while (c->next)
					c = c->next;
				c->next = push_struct_and_copy(Kerning, k);
			}
			else
			{
				font.kernings[key] = k;
			}
		}
		skipp_line(contents);
	}
	return font;
}

Asset load_font(const char *fnt_path)
{
	Asset asset;
	char *tex_path = str_copy(fnt_path);
	{
		u32 len = str_len(tex_path);
		tex_path[len - 3] = 'p';
		tex_path[len - 2] = 'n';
		tex_path[len - 1] = 'g';
	}
	AssetID texture  = load_asset(AFT_TEXTURE, tex_path);
	pop_memory(tex_path);
	
	asset.font = parse_font(fnt_path, texture);
	return asset;
}

void reload_font(Asset *old)
{
	// NOTE: Since a font is both a bunch of font data
	// and a texture. The asset system should handle 
	// the reloading of the image if it's changed. And
	// if these changes are made simultaniousely the 
	// font file should be updated afterwards. But ideally
	// we would update the font if the image is changed. Since
	// the font depends on it when it comes to the sizes.
	// One alternative would be to recalculate the offsets
	// of the font everytime we use it. That would be slower 
	// though.
	
	// Just replace it.
	old->font = parse_font(old->path, old->font.texture);
}

void destroy_font(Asset asset)
{
	destroy_list<Glyph>(&asset.font.glyphs);
	for (u32 i = 0; i < KERNING_TABLE_SIZE; i++)
	{
		Kerning *k = asset.font.kernings[i].next;
		while (k)
		{
			Kerning *old = k;
			k = k->next;
			pop_memory(old);
		}
	}
	pop_memory(asset.font.kernings);
}

#endif
