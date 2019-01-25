#ifndef __BLOCK_LEVEL__
#define __BLOCK_LEVEL__

// NOTE: This operates on the gloal game struct,
// this might be a bad idea. I don't know, let's
// find out!

struct LevelObject
{
	ShapeID shape;	
	Vec2 position;
	Vec2 scale;
	f32 rotation;
};

// Loads a level to memory.
Asset load_level(const char *path);
// Updates the loaded level.
void reload_level(Asset *asset);
// Clean up after the level.
void destroy_level(Asset asset);

#endif

