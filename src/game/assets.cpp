#define RES_AUDIO "res/sound/"


AssetID pixel;
AssetID spritesheet;
AssetID audio_pew;
AssetID audio_splat;
AssetID audio_pickup;
AssetID audio_hop;
AssetID audio_death;
AssetID audio_bg;

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
	spritesheet = load_asset(AFT_TEXTURE,   "res/sprite_sheet.png");
    
    // Audio
    audio_pew   = load_asset(AFT_SOUND,     RES_AUDIO "pew_alt_mono.wav");
    audio_splat = load_asset(AFT_SOUND,     RES_AUDIO "splat_mono.wav");
    audio_pickup= load_asset(AFT_SOUND,     RES_AUDIO "pickup_mono.wav");
    audio_hop   = load_asset(AFT_SOUND,     RES_AUDIO "hop_mono.wav");
    audio_death = load_asset(AFT_SOUND,     RES_AUDIO "deth_short_mono.wav");
    audio_bg    = load_asset(AFT_SOUND,     RES_AUDIO "bg_music_mono.wav");
}
