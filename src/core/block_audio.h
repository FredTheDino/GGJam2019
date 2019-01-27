#ifndef __BLOCK_ADUIO__ 
#define __BLOCK_ADUIO__
#define AUDIO_FREQ 48000

// NOTE: This assumes that the audio format
// is 2-channel audio with f32 encoding.
struct AudioStream
{
	f32 left;
	f32 right;
};

struct AudioSource
{
	bool playing;
    bool loop;

	bool positional;
	Vec3 position;

	f32 volume;
	// Both pitch and sample are floats so we can 
	// have fractional samples and
	// change the pitch mid playing.
	// It's gonna be great!
	f32 pitch;
	f32 sample; 

	AssetID sound;
};

#define MAX_SOURCES 64
struct AudioManager
{
	RandomState rng;
	f32 master_volume;

	Vec3 position;
	Quat orientation;

	u32 last_used;
	AudioSource sources[MAX_SOURCES];
} audio; // TODO: Maybe not have this be global.

void initalize_audio()
{
	audio = {};
	audio.orientation = toQ(V3(0, 0, 0), 0);
	audio.rng = seed(982323410213ul);
	audio.master_volume = 1.0f;
}

// The SDL callback
void audio_loop(void *userdata, u8 *byte_stream, s32 num_bytes);

// The sound is simply played
u64 play_sound(AssetID sound, f32 volume=1.0f, f32 pitch=1.0f, bool loop=false);

// The sound is played at the position
u64 play_sound_at(AssetID sound, Vec3 pos, f32 volume=1.0f, f32 pitch=1.0f);

// The sound is randomized a bit
u64 play_sound_perturbed(AssetID sound, f32 volume=1.0f, f32 pitch=1.0f, f32 volume_deviation=0.05f, f32 pitch_deviation=0.05f);

// The sound is randomized a bit and played at the postion
u64 play_sound_perturbed_at(AssetID sound, Vec3 pos, f32 volume=1.0f, f32 pitch=1.0f, f32 volume_deviation=0.05f, f32 pitch_deviation=0.05f);

// Stops the sound form playing
void stop_sound(u64 source_id);

//
// Used to read in the WAV header.
//
struct WAVHeader
{
	char riff[4];
	s32 size;
	char wave[4];

	// FMT chunk
	char fmt[4];
	s32 fmt_size;
	s16 format;
	s16 channels;
	s32 sample_rate;
	s32 byte_rate;
	s16 block_align;
	s16 bitdepth;

};

struct WAVChunk
{
	char type[4];
	s32 size;
};

// Load the specified asset as a sound. This only works on WAV right now.
Asset load_sound(const char *path);

// Destroy the sound asset.
void destroy_sound(Asset asset);

// Reload the sound asset.
void reload_sound(Asset *asset);

#endif
