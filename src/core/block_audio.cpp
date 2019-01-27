#include "block_audio.h"

//
// Helper functions for the callback
// 
f32 lerp_sample(f32 low, f32 high)
{
	// Note this should be a lerp, but I doubt
	// anyone will notice if this is an average.
	//
	// Isn't this really what the speaker is doing
	// anyways?
	return (low + high) / 2.0f;
}

u64 current_sample(SoundBuffer buffer, f32 sample)
{
	u64 normalized_sample = (u64) (sample + 0.5);
	// Make sure we don't go outside the buffer.
	if (buffer.num_samples <= normalized_sample)
		return buffer.num_samples - 1;
	return normalized_sample;
}

//
// SDL callback
//
void audio_loop(void *userdata, u8 *byte_stream, s32 num_bytes)
{
	// Zero out the buffer so we have silence.
	zero_array(byte_stream, u8, num_bytes);

	if (audio.master_volume == 0.0f)
		return; // Nothing to play.

	AudioStream *stream = (AudioStream *) byte_stream;
	f32 num_samples = (f32) num_bytes / sizeof(AudioStream);

	Vec3 ear_axis = audio.orientation * V3(1.0, 0.0, 0.0);
	ASSERT(ear_axis.x > 0.95f);
	for (u32 source_id = 0; source_id < MAX_SOURCES; source_id++)
	{
		AudioSource *source = audio.sources + source_id;
		if (!source->playing) continue; // Check that we have something to play

		SoundBuffer buffer = find_asset(source->sound).sound;

		f32 sample_rate = (f32) buffer.sample_rate;
		f32 time_to_buffer_rate = (sample_rate / ((f32) AUDIO_FREQ));
		f32 sample_advance = source->pitch * time_to_buffer_rate;
		if (buffer.is_stero)
		{
			// Stereo
			for (u32 i = 0; i < num_samples; i++)
			{
				if (source->sample > buffer.num_samples)
				{
                    if(source->loop){
                        source->sample = 0;
                    } else {
					    source->playing = false;
					    break;
                    }
				}
				AudioStream low  = buffer.stero[current_sample(buffer, source->sample)];
				source->sample += sample_advance;
				AudioStream high = buffer.stero[current_sample(buffer, source->sample)];
				stream[i].left  += lerp_sample(low.left, high.left) * audio.master_volume;
				stream[i].right += lerp_sample(high.right, high.right) * audio.master_volume;
			}
		}
		else
		{
			// Mono
			f32 left, right;
			if (source->positional)
			{
				f32 falloff = 1.0f / length(source->position - audio.position);
				Vec3 direction = (source->position - audio.position) * falloff;
				f32 angle_falloff = dot(direction, ear_axis);
				f32 shared_scaler = source->volume * maximum(falloff * 2.0, 1.0) * audio.master_volume;
				left  = (1.0 - angle_falloff) * 0.5 * shared_scaler;
				left  = clamp(0.0f, 1.0f, left);
				right = (angle_falloff) * 0.5 * shared_scaler;
				right = clamp(0.0f, 1.0f, right);

			}
			else
			{
				left  = audio.master_volume * source->volume;
				right = audio.master_volume * source->volume;
			}

			for (u32 i = 0; i < num_samples; i++)
			{
				if (source->sample > buffer.num_samples)
				{
					source->playing = false;
					break;
				}
				f32 low  = buffer.mono[current_sample(buffer, source->sample)];
				source->sample += sample_advance;
				f32 high = buffer.mono[current_sample(buffer, source->sample)];
				f32 sample = lerp_sample(low, high);

				stream[i].left  += sample * left;
				stream[i].right += sample * right;
			}
		}
	}
}

u64 play_sound(AssetID sound, f32 volume, f32 pitch, bool loop)
{
	ASSERT(volume);
	ASSERT(pitch);
	for (u32 i = 0; i < MAX_SOURCES; i++)
	{
		u64 source_id = (audio.last_used + i) % MAX_SOURCES;
		AudioSource *source = audio.sources + source_id;
		if (source->playing)
			continue;
		source->playing = true;
        source->loop = loop;
		source->sound   = sound;
		source->volume  = volume;
		source->pitch   = pitch;
		source->sample  = 0;
		audio.last_used = source_id;
		return source_id;
	}
	print("Failed to play sound, all sources are taken\n");
	return -1;
}

u64 play_sound_at(AssetID sound, Vec3 pos, f32 volume, f32 pitch)
{
	ASSERT(volume);
	ASSERT(pitch);
	for (u32 i = 0; i < MAX_SOURCES; i++)
	{
		u64 source_id = (audio.last_used + i) % MAX_SOURCES;
		AudioSource *source = audio.sources + source_id;
		if (source->playing)
			continue;
        source->loop        = false;
		source->playing 	= true;
		source->positional 	= true;
		source->position = pos;
		source->sound    = sound;
		source->volume   = volume;
		source->pitch    = pitch;
		source->sample   = 0;
		return source_id;
	}
	print("Failed to play sound, all sources are taken\n");
	return -1;
}

u64 play_sound_perturbed(AssetID sound, f32 volume, f32 pitch, f32 volume_deviation, f32 pitch_deviation)
{
	volume = volume - random_real_in_range(&audio.rng, -volume_deviation, volume_deviation);
	pitch  = pitch  - random_real_in_range(&audio.rng, -pitch_deviation, pitch_deviation);
	return play_sound(sound, volume, pitch);
}

u64 play_sound_perturbed_at(AssetID sound, Vec3 pos, f32 volume, f32 pitch, f32 volume_deviation, f32 pitch_deviation)
{
	volume = volume - random_real_in_range(&audio.rng, -volume_deviation, volume_deviation);
	pitch  = pitch  - random_real_in_range(&audio.rng, -pitch_deviation, pitch_deviation);
	return play_sound_at(sound, pos, volume, pitch);
}

void stop_sound(u64 source_id)
{
	if (source_id < MAX_SOURCES)
		audio.sources[source_id].playing = false;
}

//
// Sound asset
//
Asset load_sound(const char *path)
{
	void *file = (void *) read_entire_file(path);
	u8 *data = (u8 *) file;
	ASSERT(data);
	WAVHeader *header = (WAVHeader *) data;
	// Check the meta data and make sure we can read it.
	ASSERT(header->riff[0] == 'R');
	ASSERT(header->riff[1] == 'I');
	ASSERT(header->riff[2] == 'F');
	ASSERT(header->riff[3] == 'F');

	ASSERT(header->wave[0] == 'W');
	ASSERT(header->wave[1] == 'A');
	ASSERT(header->wave[2] == 'V');
	ASSERT(header->wave[3] == 'E');

	ASSERT(header->fmt[0] == 'f');
	ASSERT(header->fmt[1] == 'm');
	ASSERT(header->fmt[2] == 't');

	// 1 means uncompressed int, 3 means uncompressed float.
	ASSERT(header->format == 1 || header->format == 3);

	ASSERT(header->channels <= 2);
	ASSERT(0 < header->channels);

	// TODO: Need to make sure this is a valid file, 
	// so the sections can be wrong and we need to break
	// when we reach the end of the file.
	data += sizeof(WAVHeader);
	WAVChunk *chunk = (WAVChunk *) data;
	while (chunk->type[0] != 'd' ||
		   chunk->type[1] != 'a' ||
		   chunk->type[2] != 't' ||
		   chunk->type[3] != 'a')
	{
		ASSERT(*data != 0);
		data += sizeof(WAVChunk) + chunk->size;
		chunk = (WAVChunk *) data;
	}
	data += sizeof(WAVChunk);

	u32 sample_size = header->bitdepth / 8;
	u32 num_floats = chunk->size / sample_size;

	SoundBuffer buffer;
	buffer.is_stero = (header->channels == 2);
	buffer.sample_rate = header->sample_rate;
	buffer.num_samples = num_floats / header->channels;
	buffer.raw = push_array(f32, num_floats);

	f32 sample_scaler;
	if (header->format == 1)
	{
		switch (header->bitdepth)
		{
			// We assume that the audio format is signed.
			case 8:
				sample_scaler = 1.0f / ((f32) 0xEF);
				for (u32 i = 0; i < num_floats; i++)
					buffer.raw[i] = (f32) ((s8 *) data)[i] * sample_scaler;
				break;
			case 16:
				sample_scaler = 1.0f / ((f32) 0xEFFF);
				for (u32 i = 0; i < num_floats; i++)
					buffer.raw[i] = (f32) ((s16 *) data)[i] * sample_scaler;
				break;
			case 32:
				sample_scaler = 1.0f / ((f32) 0xEFFFFF);
				for (u32 i = 0; i < num_floats; i++)
					buffer.raw[i] = (f32) ((s16 *) data)[i] * sample_scaler;
				break;
			default:
				return {};
				break;
		}
	}
	else if (header->format == 3)
	{
		// We can support 64 bit audio. But having
		// that does feel like a waste.
		ASSERT(header->bitdepth == 32); 
		for (u32 i = 0; i < num_floats; i++)
			buffer.raw[i] = ((f32 *) data)[i];
	}

	pop_memory(file);
	Asset asset;
	asset.sound = buffer;
	return asset;
}

void destroy_sound(Asset asset)
{
	ASSERT(asset.type == AFT_SOUND);
	pop_memory(asset.sound.raw);
}

void reload_sound(Asset *asset)
{
	Asset replacement = load_sound(asset->path);
	if (replacement.sound.raw)
	{
		SDL_LockAudioDevice(game.device);
		destroy_sound(*asset);
		*asset = replacement;
		SDL_UnlockAudioDevice(game.device);
	}
}

