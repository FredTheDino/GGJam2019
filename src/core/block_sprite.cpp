#include "block_sprite.h"

Asset load_sprite_atlas(const char *path)
{
	// Temporary struct
	struct ParsedAnimation
	{
		AnimationLoopType loop_type;

		u32 num_frames;
		u32 frames[MAX_NUM_FRAMES];

		Token name;
		Token link_to;
	};

	const char *source = read_entire_file(path);
	ASSERT(source);
	const char *s = source;
	CharStream stream = to_stream(s);

	s32 width, height;
	u64 size_rects = 32;
	u64 size_animations = 10;
	SpriteAtlas atlas = {};
	atlas.sub_sprites = push_array(Rect, size_rects);
	ParsedAnimation *parsed_animations = push_array(ParsedAnimation, size_animations);
	do
	{
		char c = curr(stream);
		if ('@' == c) // Meta
		{
			// To be able to change them to texture coordinates.
			eat_word(stream);
			width = eat_int(stream);
			inc(stream);
			height = eat_int(stream);
		}
		else if (is_number(c)) // Sprites
		{
			f32 x, y, w, h;
			x = (f32) eat_int(stream) / width;
			y = (f32) eat_int(stream) / height;
			w = (f32) eat_int(stream) / width;
			h = (f32) eat_int(stream) / height;
			eat_spaces(stream);

			s32 tiles_x, tiles_y;
			if (is_number(curr(stream)))
			{
				tiles_x = eat_int(stream);
				inc(stream);
				tiles_y = eat_int(stream);
			}
			else
			{
				tiles_x = 1;
				tiles_y = 1;
			}

			u64 index = atlas.num_sub_sprites;
			atlas.num_sub_sprites += tiles_x * tiles_y;
			if (atlas.num_sub_sprites >= size_rects)
			{
				size_rects *= 2;
				atlas.sub_sprites = resize_array(atlas.sub_sprites, Rect, size_rects);
			}

			for (u64 oy = 0; oy < (u64) tiles_y; oy++)
			{
				for (u64 ox = 0; ox < (u64) tiles_x; ox++)
				{
					atlas.sub_sprites[index++] = R(x + w * ox, y + h * oy, w, h);
				}
			}
		}
		else if (is_letter(curr(stream)))
		{
			ParsedAnimation anim = {};
			anim.name = eat_word(stream);
			eat_spaces(stream);
			while (is_number(curr(stream)))
			{
				anim.frames[anim.num_frames++] = eat_int(stream);
				ASSERT(anim.num_frames < MAX_NUM_FRAMES);
				eat_spaces(stream);
			}

			const char *link_type = eat_word(stream).data;
			//const char *target;
			if (str_eq(link_type, "GOTO"))
			{
				anim.loop_type = ANIMATION_GOTO;
				anim.link_to = eat_word(stream);
			}
			else if (str_eq(link_type, "LOOP"))
			{
				anim.loop_type = ANIMATION_LOOP;
			}
			else if (str_eq(link_type, "STOP"))
			{
				anim.loop_type = ANIMATION_STOP;
			}
			else if (str_eq(link_type, "PING"))
			{
				anim.loop_type = ANIMATION_PING;
			}
			else
			{
				print("|| No loop type specified for animation \"%s\", STOP assumed\n", anim.name);
				anim.loop_type = ANIMATION_STOP;
			}

			// Append to list.
			u64 index = atlas.num_animations++;
			if (atlas.num_animations == size_animations)
			{
				size_animations *= 2;
				parsed_animations = resize_array(parsed_animations, ParsedAnimation, size_animations);
			}
			parsed_animations[index] = anim;
		}
		else if ('#' == c)
		{
			// Pass
		}
		skipp_line(stream);
	} while (curr(stream) != '\0');

	// Link all animations.
	atlas.animations = push_array(Animation, atlas.num_animations);
	{
		for (u64 i = 0; i < atlas.num_animations; i++)
		{
			ParsedAnimation p = parsed_animations[i];
			Animation a;

			// Linear search for a match
			if (p.loop_type == ANIMATION_GOTO)
			{
				a.link_to = -1;
				for (u64 j = 0; j < atlas.num_animations; j++)
				{
					if (str_eq(p.link_to, parsed_animations[j].name))
					{
						a.link_to = j;
						break;
					}
				}
				if (a.link_to == (u32) -1)
				{
					print("|| Failed to find animation of name \"%s\"\n", p.link_to);
					HALT_AND_CATCH_FIRE();
				}
			}

			a.loop_type = p.loop_type;
			a.num_frames = p.num_frames;
			a.name = copy_token_to_string(p.name);

			for (u32 k = 0; k < a.num_frames; k++)
				a.frames[k] = p.frames[k];
			atlas.animations[i] = a;
		}
	}
	
	pop_memory((void *) source);
	pop_memory(parsed_animations);

	Asset asset;
	asset.atlas = atlas;
	return asset;
}

void destroy_sprite_atlas(Asset asset)
{
	pop_memory(asset.atlas.animations);
	pop_memory(asset.atlas.sub_sprites);
	
	for (u32 i = 0; i < asset.atlas.num_animations; i++)
	{
		pop_memory((void *) asset.atlas.animations[i].name);
	}
}

void reload_sprite_atlas(Asset *asset)
{
	// TODO: Do some error checking, we assume it's correct.
	ASSERT(asset->type == AFT_ATLAS);
	SpriteAtlas atlas = load_sprite_atlas(asset->path).atlas;
	destroy_sprite_atlas(*asset);
	asset->atlas = atlas;
	print("|| Reloaded sprite atlas \"%s\"\n", asset->path);
}


u32 get_animation_id_by_name(SpriteAtlas atlas, const char *name)
{
	for (u32 i = 0; i < atlas.num_animations; i++)
	{
		if (str_eq(atlas.animations[i].name, name))
		{
			return i;
		}
	}
	return -1;
}

AnimationState create_animation_state(AssetID asset_id, const char *start_anim, f32 frame_time)
{
	AnimationState state = {};
	state.atlas_id = (u32) asset_id.pos;
	state.frame_time = frame_time;
	state.current_animation = get_animation_id_by_name(find_asset(asset_id).atlas, start_anim);
	// TODO: Should we really crash here?
	ASSERT(state.current_animation < 0xFFFFFF);
	return state;
}

void update_state(AnimationState *state, f32 delta)
{
	state->counter += delta;

	// NOTE: This is not a while since I want each frame to be 
	// at least one frame long.
	if (state->frame_time < state->counter)
	{
		state->counter -= state->frame_time;
		state->current_frame++;
	}
	
	SpriteAtlas atlas = find_asset(state->atlas_id).atlas;
	Animation anim = atlas.animations[state->current_animation];
	if (anim.num_frames <= state->current_frame)
	{
		// End of animation reached.
		if (anim.loop_type == ANIMATION_LOOP)
		{
			state->current_frame = 0;
		}
		else if (anim.loop_type == ANIMATION_GOTO)
		{
			state->current_frame = 0;
			state->current_animation = anim.link_to;
		}
		else if (anim.loop_type == ANIMATION_STOP)
		{
			state->current_frame = anim.num_frames - 1;
		}
		else
		{
			HALT_AND_CATCH_FIRE(); // This should not happen.
		}
	}
}

void draw_state(Context gfx, Texture texture, AnimationState state, Vec2 postion, Vec2 scale, f32 rotation)
{
	ASSERT(state.current_animation < 0xFFFFFF);
	ASSERT(state.current_frame < 0xFFFFFF);
	SpriteAtlas atlas = find_asset(state.atlas_id).atlas;
	Animation anim = atlas.animations[state.current_animation];
	ASSERT(state.current_frame < anim.num_frames);
	u32 sub_sprite = anim.frames[state.current_frame];
	ASSERT(sub_sprite < atlas.num_sub_sprites);
	Rect sprite_rect = atlas.sub_sprites[sub_sprite];
	draw_sprite(gfx, texture, sprite_rect, postion, scale, rotation);
}

