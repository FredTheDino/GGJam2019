#ifndef __BLOCK_HOTLOADER__
#define __BLOCK_HOTLOADER__

// Note: Could I forward declare the asset data 
// type somehow? Maybe with template wierdness?
// Because this is very noisy.

// Graphics
struct Shader
{
	GLuint program;
};

// Texture
struct Texture
{
	u32 id;
	s32 width, height, components;
};

// Forward declaration.
struct Animation;
struct SpriteAtlas
{
	u64 num_sub_sprites;
	Rect *sub_sprites;
	u64 num_animations;
	Animation *animations;
};

struct AudioStream;
struct SoundBuffer
{
	// All SoundBuffers are either mono or stero.
	bool is_stero;
	union
	{
		f32 *raw;
		f32 *mono;
		AudioStream *stero;
	};
	u64 sample_rate;
	u64 num_samples; // num_data_points / num_channels
};

// Font stuff.
struct Glyph
{
	// Texture cordinates.
	f32 x, y, w, h;

	f32 x_offset, y_offset;
	f32 advance;
};

struct Kerning
{
	u8 first, second;
	f32 amount;

	Kerning *next;
};

struct Font
{
	List<Glyph> glyphs;
	Kerning *kernings;
	AssetID texture;
};

struct LevelObject;
struct Level
{
	// Maybe nothing in here?
	int _;
};

// Uses AssetID
enum AssetFileType
{
	AFT_SHADER,
	AFT_TEXTURE, // Only the image.
	AFT_ATLAS, // Has two parts.
	AFT_SOUND,
	AFT_FONT,
	AFT_LEVEL,

	NUM_ASSET_TYPES
};

struct Asset
{
	// C++ couldn't figure this one out.
	Asset() {};
	AssetFileType type;
	u64 last_edit;
	Timer::Timer reload_timer;
	const char *path; // Key

	// TODO: Store the BlockID, so we can check for lifetime.
	union
	{
		Shader shader;
		Texture texture;
		SpriteAtlas atlas;
		SoundBuffer sound;
		Font font;
		Level level;
	};
};

struct AssetType
{
	AssetFileType type;
	Asset (*load)(const char *);
	void (*reload)(Asset *);
	void (*destroy)(Asset);
};

struct AssetManager
{
	AssetType asset_types[NUM_ASSET_TYPES];
	u64 size_assets;
	u64 num_assets;
	Asset *assets;
} manager;

//
// Meta
//

// Initalize the system
void initalize_asset_manager();
// Free all the memory from the system.
void destroy_asset_manager();
// Reload all assets that needs to be reloaded.
void check_assets_for_reload();
// Register a new type of asset, used internally.
void register_asset_type(AssetFileType type, 
		Asset (*load)(const char *), 
		void (*reload)(Asset *), 
		void (*destroy)(Asset));

//
// Assets
//

// Get the asset type of an asset.
inline 
AssetType asset_type(Asset *asset);

// Get the asset type from an asset id
inline
AssetType asset_type(AssetFileType type_id);
// Load the asset of the specified type
AssetID load_asset(AssetFileType file_type, const char *path);

// Find an asset with the specified path and type.
Asset find_asset(AssetFileType type, const char *path);
// Loop up an asset based on the index, not recomended since
// it doesn't check for life time. Use of this function is 
// deprecated.
Asset find_asset(u32 pos);
// Look up an asset based on a handle, a much better idea.
Asset find_asset(AssetID id);

Asset AssetID::operator* ()
{
	return find_asset(*this);
}

#endif
