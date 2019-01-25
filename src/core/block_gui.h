/*
 * TODO: 
 *     - Refactor this, and then refactor the rest of the code to use namespaces.
 *     - Rewrite it to use pixels for dimensions and positions.
 */

/*
 * This is a good start for the GUI, but it's honestly poorly made and 
 * is really har dto align. This needs to be designed more.
 */

#if 0
#define EM 0.04f
#define FONT_SIZE (10 * EM)
#define LINE_SPACING (2.2 * EM)

namespace GUI
{
	enum class GUIType
	{
		LABEL,
		BUTTON,
		SLIDER,
	};

	struct ID
	{
		// In what section of the GUI the element is
		u32 section;
		// The unique identifier.
		u32 uid;

		bool operator== (ID other) const
		{
			return uid == other.uid && section == other.section;
		}
	};

	struct Element {
		ID id;
		GUIType type;
		f32 t;
		union {
			struct {
				const char *text; 
				Vec2 text_position;
				Vec2 position;
				Vec2 dimension;
			} button;
			struct {
				const char *text; 
				Vec2 text_position;
			} label;
			struct {
				const char *text; 
				Vec2 text_position;
				const char *number; 
				Vec2 number_position;
				Vec2 knob_position;
				Vec2 position;
				Vec2 dimension;
			} slider;
		};
		Element *next;
	};

	struct Link
	{
		Element *data;
		Link *next;
	};

	struct Section
	{
		bool active;
		bool shown;
		Vec2 position;
		f32 width;
		f32 offset;
		Link *head;
		Link *tail;
	};

	struct GUI
	{
		ID active;
		ID hot;
		u32 section;

		Vec2 anchor;

		Vec2 mouse;
		bool down;
		bool click;

		AssetID font;

#define NUM_GUI_BUCKETS 64
		Element *elements[NUM_GUI_BUCKETS];

		u32 num_sections;
		Section sections[NUM_GUI_BUCKETS];

		u32 num_links;
		Link link_pool[NUM_GUI_BUCKETS];
	};

	GUI initalize_gui(AssetID font)
	{
		GUI gui = {};
		gui.font = font;
		return gui;
	}

	void destroy_gui(GUI *gui)
	{
		for (u32 i = 0; i < NUM_GUI_BUCKETS; i++)
		{
			Element *e = gui->elements[i];
			while (e)
			{
				Element *old = e;
				e = e->next;
				pop_memory(old);
			}
		}
	}


	void append_element_to_section(GUI *gui, Section *section, Element *element)
	{
		Link *link = gui->link_pool + (gui->num_links++);
		ASSERT(gui->num_links < NUM_GUI_BUCKETS);
		link->data = element;
		link->next = NULL;
		// This branch can be removed if the header part of 
		// all sections is an element, currently it kinda is 
		// but this is kept here for completeness
		if (section->head)
		{
			section->tail->next = link;
			section->tail = link;
		}
		else
		{
			section->head = link;
			section->tail = link;
		}
	}

	void new_line(Section *section)
	{
		section->offset += LINE_SPACING;
	}

	inline u32 hash(ID id)
	{
		return ((id.section << 4) ^ id.uid) % NUM_GUI_BUCKETS;
	}

	Element *fetch_element_ptr(GUI *gui, ID id)
	{
		u32 i = hash(id);
		Element **curr = gui->elements + i;
		while (*curr)
		{
			if ((*curr)->id == id)
				return *curr;
			curr = &(*curr)->next;
		}
		*curr = push_struct(Element);
		**curr = {};
		(*curr)->next = NULL;
		return *curr;
	}

	bool section(GUI *gui, u32 id)
	{
		ASSERT(id < NUM_GUI_BUCKETS);
		gui->section = id;
		Section *section = gui->sections + id;

		f32 header_height = EM * 0.5f;
		Vec2 dimension = V2(section->width / 2, header_height);
		Vec2 position = V2(dimension.x, dimension.y - LINE_SPACING);
		ID header_id = {id, (u32) -1};
		Element *e = fetch_element_ptr(gui, header_id);
		e->id = header_id;
		e->type = GUIType::BUTTON;
		e->button.dimension = dimension;
		e->button.position = position;

		Vec2 mouse = gui->mouse - section->position;
		if (point_in(mouse, e->button.position, e->button.dimension))
		{
			gui->hot = e->id;
		}
		
		section->active = true;
		section->head = NULL;
		section->tail = NULL;
		section->offset = 0;
		section->width = 0;

		append_element_to_section(gui, section, e);

		return true;
	}

	// Start the frame
	void begin(GUI *gui, Vec2 mouse, bool click, bool down)
	{
		gui->num_links = 0;
		gui->num_sections = 0;
		gui->anchor = V2(-0.9, -0.9);
		gui->mouse = mouse;
		gui->click = click;
		gui->down = down;
		gui->hot = {(u32) -1, (u32) -1};
		gui->section = 0;
		section(gui, gui->section);
	}

	bool button(GUI *gui, u32 uid, const char *text)
	{
		Section *section = gui->sections + gui->section;

		f32 padding = 0.25 * EM;
		f32 length = messure_text(gui->font, text, FONT_SIZE) * game.aspect_ratio;
		ID id = {gui->section, uid};
		Element *e = fetch_element_ptr(gui, id);

		e->id = id;
		e->type = GUIType::BUTTON;
		e->button.text = text;
		e->button.dimension = V2(length / 2 + padding, EM);
		e->button.position = V2(e->button.dimension.x, section->offset);
		e->button.text_position = e->button.position;
		e->button.text_position.x -= length / 2 - padding;
		e->button.text_position.y -= e->button.dimension.y / 2;

		append_element_to_section(gui, section, e);
		section->width = maximum(section->width, e->button.position.x + e->button.dimension.x);
		new_line(section);

		if (point_in(gui->mouse, section->position + e->button.position, e->button.dimension))
		{
			gui->hot = e->id;
			return gui->click;
		}
		return false;
	}

	void label(GUI *gui, u32 uid, const char *text)
	{
		Section *section = gui->sections + gui->section;

		f32 length = messure_text(gui->font, text, FONT_SIZE);
		Vec2 dimension = V2(length / 2, EM);
		Vec2 position = V2(dimension.x, section->offset);

		ID id = {gui->section, uid};
		Element *e = fetch_element_ptr(gui, id);
		e->id = id;
		e->type = GUIType::LABEL;
		e->label.text = text;
		e->label.text_position = position;
		e->label.text_position.x -= length / 2 - 0.25 * EM;
		e->label.text_position.y -= dimension.y / 2;

		append_element_to_section(gui, section, e);
		section->width = maximum(section->width, position.x + dimension.x);
		new_line(section);
	}

#if 1
	bool slider(GUI *gui, u32 uid, const char* label, f32 min, f32 *value, f32 max)
	{
		Section *section = gui->sections + gui->section;
		// TODO: Temporary memory that's whiped every frame.
		f32 length = 5 * EM;
		char *number = temp_array(char, 32);
		// The number could be passed to the struct and layed out
		// in the draw function.
		format(number, "%.02f", *value);
		f32 number_width = messure_text(gui->font, number, FONT_SIZE);
		f32 label_width = messure_text(gui->font, label, FONT_SIZE);

		ID id = {gui->section, uid};
		Element *e = fetch_element_ptr(gui, id);
		e->id = id;
		e->type = GUIType::SLIDER;
	
		e->slider.dimension = V2(length, EM);
		e->slider.position = V2(length, section->offset);
		e->slider.number = number;
		e->slider.number_position = e->slider.position;
		e->slider.number_position.y -= e->slider.dimension.y / 2;
		e->slider.number_position.x -= number_width / 2;
		e->slider.text = label;
		e->slider.text_position = e->slider.position;
		e->slider.text_position.y -= e->slider.dimension.y / 2;
		e->slider.text_position.x += length + 0.5 * EM;

		append_element_to_section(gui, section, e);
		section->width = maximum(section->width, e->slider.text_position.x + length + 0.5f * EM);
		new_line(section);


		Vec2 p_min = e->slider.position - V2(length - 0.5 * EM, 0);
		Vec2 p_max = e->slider.position + V2(length - 0.5 * EM, 0);
		Vec2 axis = V2(1, 0); // Assumes it's the X-axis of the window.

		f32 old_value = *value;
		Vec2 mouse = gui->mouse - section->position;
		if (point_in(mouse, e->slider.position, e->slider.dimension + V2(EM, 0)))
		{
			if (gui->click)
				gui->active = e->id;

			gui->hot = e->id;
			if (gui->down && gui->active == e->id)
			{
				f32 normalized = (mouse.x - p_min.x) / (p_max.x - p_min.x);
				normalized = clamp(0.0f, 1.0f, normalized);
				*value = min + (normalized * (max - min));
			}
		}

		f32 blend = clamp(0.0f, 1.0f, (*value - min) / (max - min));
		e->slider.knob_position = lerp(p_max, blend, p_min);

		return old_value != *value;
	}
#endif

	void draw(GUI *gui)
	{
		Vec4 active_color = V4(0.50f, 0.40f, 0.90f, 0.75f);
		Vec4 normal_color = V4(0.50f, 0.50f, 0.50f, 0.75f);
		Vec4 text_color   = V4(0.00f, 0.00f, 0.00f, 0.75f);
		Vec4 background_color = normal_color * 0.5f;
		Vec4 section_color = normal_color * 0.5f;
		for (u32 i = 0; i < NUM_GUI_BUCKETS; i++)
		{
			Section *section = gui->sections + i;
			if (!section->active) continue;
			section->active = false;
			Vec2 pos, dim;
			dim = V2(section->width, section->offset) / 2;
			pos = section->position + dim - V2(0, EM);
			draw_rect(pos, dim, section_color);
			Link *link = section->head;
			for (; link; link = link->next)
			{
				Element *e = link->data;
				ASSERT(e);
				e->t = minimum(maximum(0.0, e->t - game.clock.delta), 1.0);
				if (gui->hot == e->id)
					e->t = 1.0f;
				Vec4 color = lerp(active_color, e->t, normal_color);

				switch (e->type)
				{
				case GUIType::BUTTON:
					draw_rect(section->position + e->button.position, e->button.dimension, color);
					if (e->button.text)
						draw_text(gui->font, e->button.text, section->position + e->button.text_position, FONT_SIZE, text_color);
					break;
				case GUIType::LABEL:
					if (e->label.text)
						draw_text(gui->font, e->label.text, section->position + e->label.text_position, FONT_SIZE, text_color);
					break;
				case GUIType::SLIDER:
					draw_rect(section->position + e->slider.position, e->slider.dimension, background_color);
					draw_rect(section->position + e->slider.knob_position, V2(0.5 * EM, EM), color);
					if (e->slider.number)
						draw_text(gui->font, e->slider.number, section->position + e->slider.number_position, FONT_SIZE, text_color);
					if (e->slider.text)
						draw_text(gui->font, e->slider.text, section->position + e->slider.text_position, FONT_SIZE, text_color);
					break;
				default:
					ASSERT(false);
				}
			}
		}
	}
}
#endif
