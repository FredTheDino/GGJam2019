
AssetID pixel;
AssetID spritesheet;
AssetID audio_pew;
AssetID audio_splat;

#define SPRITE(id) generate_sprite(id, 32, 32)

void draw_sprite(u32 sprite, Vec2 position, Vec2 scale=V2(1, 1), 
		f32 rotation=0, Vec4 tint=V4(1, 1, 1, 1))
{
	scale.y *= -1;
	Texture tex = find_asset(spritesheet).texture;
	Rect rect = SPRITE(sprite);
	draw_sprite(tex, rect, position, scale, rotation, tint);
}

void load_assets() {
    // Textures
	pixel       = load_asset(AFT_TEXTURE,   "res/simple.png");
	spritesheet = load_asset(AFT_TEXTURE, "res/sprite_sheet.png");
    
    // Audio
    audio_pew   = load_asset(AFT_SOUND,     "res/sound/pew_mono.wav");
    audio_splat = load_asset(AFT_SOUND,     "res/sound/splat_mono.wav");
}
