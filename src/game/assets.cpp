
AssetID pixel;
AssetID spritesheet;
AssetID audio_pew;
AssetID audio_splat;

void load_assets() {
    // Textures
	pixel       = load_asset(AFT_TEXTURE,   "res/simple.png");
	spritesheet = load_asset(AFT_TEXTURE, "res/sprite_sheet.png");
    
    // Audio
    audio_pew   = load_asset(AFT_SOUND,     "res/sound/pew_mono.wav");
    audio_splat = load_asset(AFT_SOUND,     "res/sound/splat_mono.wav");
}
