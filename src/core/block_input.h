#ifndef __BLOCK_INPUT__
#define __BLOCK_INPUT__

enum Device
{
	KEYBOARD,
	GAMEPAD_BUTTON,
	GAMEPAD_AXIS,
	MOUSE
};

#define IS_DOWN_THRESHOLD 0.3f
#define GAMEPAD_DEADZONE 0.1f

struct Binding
{
	Device device;
	union
	{
		s32 button;
		s32 key;
	};

	f32 scale;

	u64 id;

	u64 input_id;
	char *input_key;

	Binding *next; // For linking them up.

	bool operator== (Binding other) const
	{
		return device == other.device && button == other.button;
	}

	bool operator!= (Binding other) const
	{
		return !(other == *this);
	}
};

struct Input
{
	const char *key;
	u64 id;
	u32 presses; // The number of inputs holding this down.

	f32 previous_value;
	f32 value;

	Input *next;

	bool operator== (Input other) const
	{
		return str_eq(key, other.key);
	}
};

struct InputManager
{
	// Could have an extra buffer here, where we store collisions
	// And we could stretch that buffer and have linked lists
	// to solve collisions. Should perform better than Robin
	// Hood, right? Right
	List<Input> inputs;
	List<Binding> bindings;
	List<SDL_GameController *> controllers;

	s32 mouse_x, mouse_y;
};

// Lowercase letters for normal letters, check the SDL2 documentation
// if there are questions.
#define KEY(s) {KEYBOARD, ( SDLK_ ## s )}
// Controller buttons
#define CBUTTON(s) {GAMEPAD_BUTTON, ( SDL_CONTROLLER_BUTTON_ ## s )}
// Controller axis
#define CAXIS(s) {GAMEPAD_AXIS, ( SDL_CONTROLLER_AXIS_ ## s )}
// Mouse buttons: LEFT, RIGHT, MIDDLE.
#define MOUSE(s) {MOUSE, ( SDL_BUTTON_ ## s )}

//
// Mouse
//

// Output normalized mouse coordinates for the registerd mouse, 
// the coordinates are ranged from (-1, -1) to (1, 1).
Vec2 normalized_mouse_coords(InputManager input);

//
// Input management
//

// Initalization
InputManager initalize_input();
// Free up all the memory used by the manager.
void destroy_input(InputManager input);
// Remove all bindings to keys, controller inputs and mouse buttons.
void clear_input_bidings(Input *input);

//
// Hashing
//

// Hash a binding.
u64 binding_hash(InputManager input, Binding b);
// Hash a string name for the input.
u64 input_hash(InputManager input, const char *key);

//
// Input bindings
//

// Search for the input with the same name.
Input *find_input(InputManager input, const char *key);
// Search for the input bound to this binding.
Input *find_input(InputManager input, Binding binding);
// Search for the binding matching this binding.
Binding *find_binding(InputManager input, Binding binding);

// Register a new input with the specified name.
void add_input(InputManager input, const char *key);
// Register a new binding to the input named "key".
void add_binding(InputManager input, Binding binding, const char *key, f32 scale=1.0f);

//
// Helper functions for handleing the input.
//

// Registers an axis as pressed in the specified direction.
void move_axis(InputManager *input, Binding binding, s16 axis_value);
// Registers a button as pressed.
void press_button(InputManager *input, Binding binding);
// Registers a button as released.
void release_button(InputManager *input, Binding binding);

// Fetch all the SDL2 events and parse them into input data.
void update_input(InputManager *input);

// Returns true if the value for the key is larger then "
bool down(InputManager input, const char *key);
// Returns true if the button is not down.
bool up(InputManager input, const char *key);
// Returns true if the button was released on the previouse frame
// and down this frame.
bool pressed(InputManager input, const char *key);
// Returns true if the button was down on the previouse frame
// and released this frame.
bool released(InputManager input, const char *key);
// Returns the value of the input. E.g. axis direction.
f32 value(InputManager input, const char *key);

// For when you just want it to assume things.
bool down(const char *key) { return down(*game.input, key); }
bool up(const char *key) { return up(*game.input, key); }
bool pressed(const char *key) { return pressed(*game.input, key); }
bool released(const char *key) { return released(*game.input, key); }
f32 value(const char *key) { return value(*game.input, key); }

#endif
