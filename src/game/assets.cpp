
AssetID pixel;
AssetID gradient_test;
AssetID spritesheet;

void load_assets() {
	pixel = load_asset(AFT_TEXTURE, "res/simple.png");
	spritesheet = load_asset(AFT_TEXTURE, "res/sprite_sheet.png");
}
