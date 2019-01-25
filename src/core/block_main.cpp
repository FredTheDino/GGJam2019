#ifdef WIN32
#pragma warning(push, 0)
#endif

#define GAME_TITLE "Game Title"

// SDL
#include <SDL2/SDL.h>

// OpenGL
#include <glad/glad.h>
#include <glad/glad.c>

#ifdef WIN32
#pragma warning(pop)
#endif

#include "block_main.h"
#include "block_id.h"
#include "block_timer.h"

#include "block_memory.h"
#include "block_memory.cpp"

#include "block_string.h"

#include "block_list.h"

#include "block_random.h"

#include "block_json.h"
#include "block_json.cpp"

#include "block_input.h"
#include "block_input.cpp"

#include "block_physics.h"

#include "block_hotloader.h"
#include "block_graphics.h"
#include "block_sprite.h"
#include "block_texture.h"
#include "block_font.h"
#include "block_audio.cpp"
#include "block_level.h"
#include "block_particles.h"

#include "block_level.cpp"
#include "block_sprite.cpp"
#include "block_graphics.cpp"
#include "block_hotloader.cpp"

#include "block_physics.cpp"

#include "../game/player.cpp"

void initalize_libraries()
{
	//
	// Initalize SDL
	//
	auto error = SDL_Init(SDL_INIT_EVERYTHING);
	ASSERT(error == 0);

	//
	// Setup a window.
	//
	s32 window_width = 800;
	s32 window_height = 800;
	bool vsync = true;
	game.window = SDL_CreateWindow(GAME_TITLE, 
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			window_width, window_height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	ASSERT(game.window);

	// 
	// OpenGL
	//
	game.gl_context = SDL_GL_CreateContext(game.window);
	ASSERT(game.gl_context);
	ASSERT(gladLoadGL());
	SDL_GL_SetSwapInterval(vsync);

	// Resize the OpenGL and SDL viewports.
	set_viewport_size(window_width, window_height);

	//
	// Setup the audio
	//
	SDL_AudioSpec want, have;

	want = {};
	want.freq = AUDIO_FREQ;
	want.format = AUDIO_F32;
	want.channels = 2;
	want.samples = 4096;
	want.callback = audio_loop;
	// TODO: This might need to be switchable
	game.device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0); 
	ASSERT(game.device) 
	ASSERT(have.freq == want.freq);
	ASSERT(have.channels == want.channels);
	ASSERT(have.format == want.format);

	initalize_audio();
	SDL_PauseAudioDevice(game.device, 0);

#if 0
	print("|| OpenGL Version: %d.%d\n", GLVersion.major, GLVersion.minor);
#endif

}

void destroy_libraries()
{
	SDL_CloseAudioDevice(game.device);
	SDL_Quit();
}

void update_game_clock(Clock *clock)
{
	f64 current_time = MS_TO_SEC(SDL_GetTicks());
	clock->delta = current_time - clock->time;
	clock->time = current_time;
}

void run()
{
	// 
	// Initalization
	//
	initalize_temp_memory();
	initalize_libraries();
	initalize_asset_manager();
	PhysicsWorld physics_world = initalize_world();
	game.world = &physics_world;

	{
		List<Vec2> points = create_list<Vec2>(4);

		points.append(V2(-0.5f,  0.5f));
		points.append(V2( 0.5f,  0.5f));
		points.append(V2( 0.5f, -0.5f));
		points.append(V2(-0.5f, -0.5f));

		_default_rect = register_shape(points);

		destroy_list(&points);
	}

	// 
	// Graphcis
	//
	Context sprite_context = initalize_graphics(load_asset(AFT_SHADER, "res/sprite.glsl"));
	game.context = &sprite_context;

	Context text_context = initalize_graphics(load_asset(AFT_SHADER, "res/text.glsl"));
	game.text_context = &text_context;

	Camera main_camera = {};
	game.camera = &main_camera;
	game.camera->shake_stress = 0.05f;
	game.camera->zoom = 0.1f;
	game.camera->rotation = 0.0f;

	// 
	// Input
	//
	InputManager input = initalize_input();
	game.input = &input;

	add_input(input, "quit");
	add_binding(input, KEY(ESCAPE), "quit");
	add_input(input, "camera_shake");
	add_binding(input, KEY(e), "camera_shake");
	add_binding(input, CBUTTON(X), "camera_shake");

	add_input(input, "x-move");
	add_binding(input, KEY(d),  1, "x-move");
	add_binding(input, KEY(a), -1, "x-move");
	add_binding(input, CAXIS(LEFTX), "x-move");

	add_input(input, "y-move");
	add_binding(input, KEY(w), -1, "y-move");
	add_binding(input, KEY(s),  1, "y-move");
	add_binding(input, CAXIS(LEFTY), "y-move");

	add_input(input, "shoot");
	add_binding(input, KEY(SPACE), "shoot");
	add_binding(input, CBUTTON(Y), "shoot");

	add_input(input, "reset_size");
	add_binding(input, KEY(g), "reset_size");

	add_input(input, "click");
	add_binding(input, KEY(o), "click");
	add_binding(input, MOUSE(LEFT), "click");

	add_input(input, "show");
	add_binding(input, KEY(q), "show");

	game.running = 1;
	while (game.running)
	{
		swap_temp_memory();
		update_game_clock(&game.clock);
		//count_fps();

		check_assets_for_reload();

		update_input(&input);

		// Update
		{
			// Physics update.
			update_world(game.clock.delta);
		}

		// Draw
		{
			// Start a new frame
			frame(game.clock);

			f32 scale = 0.5f;
			f32 rotation = 0.0f; //game.clock.time;

			// Debug draw the physics
			debug_draw_world();
			// Draw debug primitives.
			debug_draw();

			// Tell SDL to update the graphics.
			SDL_GL_SwapWindow(game.window);
		}
	}

	// #LetTheOSDoIt.
#if 1
	destroy_graphics(game.context);
	destroy_input(game.input);
	destroy_world(game.world);
	destroy_asset_manager();
	destroy_libraries();
	destroy_temp_memory();
#endif

	debug_check_memory();
}

