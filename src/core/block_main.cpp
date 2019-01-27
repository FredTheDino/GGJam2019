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

#define GRAVITY 20
#define PLAYER_TYPE 1
#define SHOT_TYPE 2
#define JELLO_TYPE 3
#define PICKUP_TYPE 4
#define KILLFLOOR_TYPE 6

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
#include "block_tilemap.h"

#include "block_physics.cpp"

u32 deaths = 0;

#include "../game/assets.cpp"

#include "../game/end.cpp"
#include "../game/shoot.h"
#include "../game/jello.h"
#include "../game/particles.h"
#include "../game/player.cpp"
#include "../game/shoot.cpp"
#include "../game/jello.cpp"
#include "../game/pickup.cpp"
#include "../game/kill_floor.cpp"
#include "../game/level_loader.cpp"
#include "../game/enemy.cpp"
#include "../game/particles.cpp"

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
	want.samples = 1024;
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
	clock->delta = maximum((f32) (current_time - clock->time), 1.0f / 30.0f);
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

	particle_system = create_particle_system(pixel);

	{ 
		List<Vec2> points = create_list<Vec2>(4);

		points.append(V2(-0.5f,  0.5f));
		points.append(V2( 0.5f,  0.5f));
		points.append(V2( 0.5f, -0.5f));
		points.append(V2(-0.5f, -0.5f));

		_default_rect = register_shape(points);

		destroy_list(&points);
	}

	load_assets();

	//List<Shot*> shots = create_list<Shot*>(5); 
	//List<Jello*> jellos = create_list<Jello*>(20); 
	//List<Pickup*> pickups = create_list<Pickup*>(10); 

	Level level = {};
	level_load("res/map2.json", &level);
	Enemy *enemy = create_enemy(level.player->body_id->position + V2(2, 1));

	//pickups.append(create_pickup(&pickups, player->body_id->position + V2(10, 0), CARROT));
	//pickups.append(create_pickup(&pickups, player->body_id->position + V2(3, 0), JELLO));

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
	game.camera->zoom = 0.08;
	game.camera->rotation = 0.0f;

	float snow_time = 0;
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

	add_input(input, "right");
	add_binding(input, KEY(d),  1, "right");
	add_input(input, "left");
	add_binding(input, KEY(a), -1, "left");

	add_input(input, "y-move");
	add_binding(input, KEY(w), -1, "y-move");
	add_binding(input, KEY(s),  1, "y-move");
	add_binding(input, CAXIS(LEFTY), "y-move");

	add_input(input, "jump");
	add_binding(input, KEY(SPACE), "jump");

	add_input(input, "shoot");
	add_binding(input, KEY(c), "shoot");

	add_input(input, "reset_size");
	add_binding(input, KEY(g), "reset_size");

	add_input(input, "click");
	add_binding(input, KEY(o), "click");
	add_binding(input, MOUSE(LEFT), "click");

	add_input(input, "show");
	add_binding(input, KEY(q), "show");

	update_game_clock(&game.clock);
	game.running = 1;
	while (game.running)
	{
		Player *player = level.player;

		swap_temp_memory();
		update_game_clock(&game.clock);
		//count_fps();

		check_assets_for_reload();

		update_input(&input);

		// Update
		{
			// Snow particles
			{
				snow_time += game.clock.delta;
				if (snow_time > 0.10) {
					snow_time = 0;
					Particle p = {};
					p.position = player->body_id->position + V2(random_real_in_range(&rnd, -50.0f, 50.0f), random_real_in_range(&rnd, 10.0f, 30.0f));
					p.lifetime = 60;
					p.from_color = V4(1, 1, 1, 0.8f);
					p.to_color = V4(1, 1, 1, 0.8f);
					p.scale = V2(0.1f, 0.1f);
					p.is_sine = true;
					p.gravity = GRAVITY/5000.0f;
					add_particle(&particle_system, p);
				}
			}

			if (pressed("quit")) 
			{
				game.running = 0;
			}

			update_particles(&particle_system, game.clock.delta);
			player_update(player, game.clock.delta);
			update_shots(&level.shots, game.clock.delta);
			update_jellos(&level.jellos, game.clock.delta);

			// Handle input
			if (pressed("shoot")) {
				player_shoot(player, &level.shots, &level.jellos);
			}

			// Check if we died
			if (player->body_id->position.y < -100) {
				player_respawn(player);
			}

            enemy_update(enemy, game.clock.delta);
			
			// Physics update.
			update_world(game.clock.delta);
			if (player != level.player)
				continue;
		}

		// Draw
		{
			// Start a new frame
			frame(game.clock);

			draw_end(level.end);

			draw_particles(&particle_system);
			player_draw(player);
			enemy_draw(enemy);
			shots_draw(&level.shots);
			pickups_draw(&level.pickups);
			jellos_draw(&level.jellos);
			
			debug_line(V2(1, 1), V2(-1, -1));
			debug_line(V2(-1, 1), V2(1, -1));

			draw_tilemap(game.context, &level.map);

			// Debug draw the physics
			if (value("show"))
			{
				debug_draw_world();
				// Draw debug primitives.
				debug_draw();
			}

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

	// debug_check_memory();
}

