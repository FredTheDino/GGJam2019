#ifndef __BLOCK_SPRITE__
#define __BLOCK_SPRITE__

// NOTE, TODO:
// Do I really need this? It's a bit janky and probably 
// should be deprecated.

enum AnimationLoopType
{
	ANIMATION_LOOP,
	ANIMATION_STOP,
	ANIMATION_PING, // Unsure
	ANIMATION_GOTO
};

#define MAX_NUM_FRAMES 32

struct Animation
{
	// Maximum is 32,
	// but they can be linked.
	AnimationLoopType loop_type;
	u32 link_to;
	u32 num_frames;
	u32 frames[MAX_NUM_FRAMES];
	const char *name;
};

struct AnimationState
{
	u32 atlas_id;

	u32 current_frame;
	u32 current_animation;

	f32 counter;
	f32 frame_time;
};

AnimationState create_animation_state(AssetID, const char *, f32);

#endif
