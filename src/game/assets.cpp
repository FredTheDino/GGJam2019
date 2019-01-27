
AssetID pixel;
AssetID spritesheet;
AssetID audio_pew;
AssetID audio_splat;
AssetID audio_pickup;
AssetID audio_hop;

void load_assets() {
    // Textures
	pixel       = load_asset(AFT_TEXTURE,   "res/simple.png");
	spritesheet = load_asset(AFT_TEXTURE,   "res/sprite_sheet.png");
    
    // Audio
    audio_pew   = load_asset(AFT_SOUND,     "res/sound/pew_alt_mono.wav");
    audio_splat = load_asset(AFT_SOUND,     "res/sound/splat_mono.wav");
    audio_pickup= load_asset(AFT_SOUND,     "res/sound/pickup_mono.wav");
    audio_hop   = load_asset(AFT_SOUND,     "res/sound/hop_mono.wav");
}
