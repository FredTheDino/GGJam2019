InputManager initalize_input()
{
	// TODO, scale these bad boys
	u64 table_size = 512;
	InputManager input = {};
	input.inputs = create_list<Input>(table_size);
	input.inputs.length = input.inputs.capacity;
	zero_array(input.inputs + 0, Input, input.inputs.length);
	input.inputs[0].id = 1; // So we set the ID if it's taken.

	input.bindings = create_list<Binding>(table_size);
	input.bindings.length = input.bindings.capacity;
	zero_array(input.bindings + 0, Binding, input.bindings.length);
	input.bindings[0].id = 1; // So we set the ID if it's taken.

	input.controllers = create_list<SDL_GameController *>(32); // Randomly choosen.
	input.controllers.length = input.controllers.capacity;

	return input;
}

void destroy_input(InputManager *input)
{
	for (u64 i = 0; i < input->inputs.length; i++)
	{
		for (Input *current = (input->inputs + i)->next;
			current;
			current = current->next)
		{
			pop_memory(current);
		}
	}
	destroy_list(&input->inputs);

	for (u64 i = 0; i < input->bindings.length; i++)
	{
		for (Binding *current = (input->bindings + i)->next;
			current;
			current = current->next)
		{
			pop_memory(current);
		}
	}
	destroy_list(&input->bindings);
	destroy_list(&input->controllers);
	*input = {};
}

void clear_input_bidings(InputManager *input)
{
	for (u64 i = 0; i < input->bindings.length; i++)
	{
		for (Binding *current = (input->bindings + i)->next;
			current;
			current = current->next)
		{
			pop_memory(current);
		}
	}
	zero_array(input->bindings.data, Binding, input->bindings.length);
	input->bindings[0].id = 1; // So we set the ID if it's taken.
}

u64 binding_hash(InputManager input, Binding b)
{
	// TODO: Better hash
	u64 hash = (b.button << 2) | (b.device);
	return hash % input.bindings.length;
}

u64 input_hash(InputManager input, const char *key)
{
	// one_at_a_time hash from Jenkins.
	u64 hash = 234;
	const char *c = key;
	while (*c)
	{
		hash += *c;
		hash += hash << 10;
		hash ^= hash >> 6;
		c++;
	}
	hash += hash << 3;
	hash ^= hash >> 11;
	hash += hash << 15;
	return hash % input.inputs.length;
}

Input *find_input(InputManager input, const char *key)
{
	u64 pos = input_hash(input, key);
	for (Input *current = input.inputs + pos; 
		current; 
		current = current->next)
	{

		if (str_eq(current->key, key))
		{
			return current;
		}
	}
	return NULL;
}

Input *find_input(InputManager input, Binding binding)
{
	Binding *b = find_binding(input, binding);
	if (b)
	{
		for (Input *current = input.inputs + b->input_id; 
			current; 
			current = current->next)
		{
			if (str_eq(current->key, b->input_key))
			{
				return current;
			}
		}
	}
	return NULL;
}

Binding *find_binding(InputManager input, Binding binding)
{

	u64 pos = binding_hash(input, binding);

	if (pos != input.bindings[pos].id)
		return NULL;
	if (input.bindings[pos] != binding)
		return NULL;

	for (Binding *current = input.bindings + pos;
		current;
		current = current->next)
	{
		if (*current == binding)
			return current;
	}
	return NULL;
}

void add_input(InputManager input, const char *key)
{
	u64 pos = input_hash(input, key);
	Input i = {key, pos};

	if (input.inputs[pos].id == pos)
	{
		i.next = input.inputs[pos].next;
		input.inputs[pos].next = push_struct_and_copy(Input, i);
	}
	else
	{
		input.inputs[pos] = i;
	}
}

void add_binding(InputManager input, Binding binding, const char *key, f32 scale)
{
	Input *i = find_input(input, key);
	ASSERT(i);

	u64 pos = binding_hash(input, binding);

	binding.id = pos;
	binding.scale = scale;

	binding.input_id = i->id;
	binding.input_key = (char *) i->key;

	// Assert that we don't get collisions.
	if (input.bindings[pos].id == pos)
	{
		//Insert it into the linked list.
		binding.next = input.bindings[pos].next;
		input.bindings[pos].next = push_struct_and_copy(Binding, binding);;
	}
	else
	{
		binding.next = 0;
		input.bindings[pos] = binding;
	}
}

void add_binding(InputManager input, Binding binding, f32 scale, const char *key)
{
	add_binding(input, binding, key, scale);
}

void move_axis(InputManager *input, Binding binding, s16 axis_value)
{
	Binding *b = find_binding(*input, binding);
	if (!b) return;
	Input *i = find_input(*input, *b);
	if (!i) return;
	f32 value = (f32) axis_value / 32768;
	if (in_range(-GAMEPAD_DEADZONE, GAMEPAD_DEADZONE, value))
		i->value = 0.0f;
	else
		i->value = value * b->scale;
}

void press_button(InputManager *input, Binding binding)
{
	Binding *b = find_binding(*input, binding);
	if (!b) return;
	Input *i = find_input(*input, *b);
	if (!i) return;
	i->presses++;
	i->value = b->scale;
}

void release_button(InputManager *input, Binding binding)
{
	Input *i = find_input(*input, binding);
	if (i)
	{
		i->presses--;
		if (i->presses == 0)
			i->value = 0.0f;
	}
}

void update_input(InputManager *input)
{
	for (u64 index = 0; index < input->inputs.length; index++)
	{
		input->inputs[index].previous_value = input->inputs[index].value;
	}

	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		if (e.type == SDL_WINDOWEVENT)
		{
			if (e.window.event == SDL_WINDOWEVENT_CLOSE)
			{
				// Skipp the cermoni
				game.running = false;
			}

			if (e.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				s32 width = e.window.data1;
				s32 height = e.window.data2;
				set_viewport_size(width, height);
			}

			if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
			{
				s32 width = e.window.data1;
				s32 height = e.window.data2;
				set_viewport_size(width, height);
			}
		}

		// Mouse
		{
			if (e.type == SDL_MOUSEMOTION)
			{
				input->mouse_x = e.motion.x;
				input->mouse_y = e.motion.y;
			}

			if (e.type == SDL_MOUSEBUTTONDOWN)
			{
				Binding binding = {MOUSE, e.button.button};
				press_button(input, binding);
			}

			if (e.type == SDL_MOUSEBUTTONUP)
			{
				Binding binding = {MOUSE, e.button.button};
				release_button(input, binding);
			}
		}
		
		// Controller
		{
			if (e.type == SDL_CONTROLLERDEVICEADDED)
			{
				u64 controller_id = e.cdevice.which;
				print("|| Controller connected (%d)\n", controller_id);
				input->controllers[controller_id] = SDL_GameControllerOpen(controller_id);
			}
				
			if (e.type == SDL_CONTROLLERDEVICEREMOVED)
			{
				u64 controller_id = e.cdevice.which;
				print("|| Controller disconnected (%d)\n", controller_id);
				SDL_GameControllerClose(input->controllers[controller_id]);
			}

			if (e.type == SDL_CONTROLLERAXISMOTION)
			{
				Binding binding = {GAMEPAD_AXIS, e.caxis.axis};
				move_axis(input, binding, e.caxis.value);
			}

			if (e.type == SDL_CONTROLLERBUTTONDOWN)
			{
				Binding binding = {GAMEPAD_BUTTON, e.cbutton.button};
				press_button(input, binding);
			}

			if (e.type == SDL_CONTROLLERBUTTONUP)
			{
				Binding binding = {GAMEPAD_BUTTON, e.cbutton.button};
				release_button(input, binding);
			}
		}

		// Keys
		{
			if (e.type == SDL_KEYDOWN)
			{
				if (e.key.repeat) continue;
				Binding binding = {KEYBOARD, e.key.keysym.sym};
				press_button(input, binding);
			}

			if (e.type == SDL_KEYUP)
			{
				Binding binding = {KEYBOARD, e.key.keysym.sym};
				release_button(input, binding);
			}
		}
	}
}

// Basic usage functions
inline
bool down(InputManager input, const char *key)
{
	Input *i = find_input(input, key);
	return IS_DOWN_THRESHOLD < i->value;
}

inline
bool up(InputManager input, const char *key)
{
	return !down(input, key);
}

inline
bool pressed(InputManager input, const char *key)
{
	Input *i = find_input(input, key);
	return IS_DOWN_THRESHOLD < i->value && IS_DOWN_THRESHOLD > i->previous_value;
}

bool released(InputManager input, const char *key)
{
	Input *i = find_input(input, key);
	return IS_DOWN_THRESHOLD > i->value && IS_DOWN_THRESHOLD < i->previous_value;
}

f32 value(InputManager input, const char *key)
{
	Input *i = find_input(input, key);
	return i->value;
}

// Transforms
Vec2 normalized_mouse_coords(InputManager input)
{
	f32 x = (f32) input.mouse_x / (f32) game.width;
	f32 y = (f32) input.mouse_y / (f32) game.height;
	Vec2 result = (V2(x, y) - V2(0.5f, 0.5f)) * 2.0f;
	return result;
}

