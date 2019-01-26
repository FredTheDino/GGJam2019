#ifndef __BLOCK_GRAPHICS__
#define __BLOCK_GRAPHICS__
//
// Note:
// There are cirtain variables in the shader
// that the is assumed to be there. If they 
// are named incorrectly or not pressent, the
// rendering will look wierd.
//

struct Camera
{
	// General transform
	Vec2 position;
	f32 zoom;
	f32 rotation;

	// These are for the camera shake.
	f32 shake_timer;
	f32 shake_stress;
};

struct Context
{
	GLuint vertex_array;
	GLuint vertex_buffer;

	AssetID shader_id;
};

// Make this into a global struct
#define MAX_NUM_DEBUG_LINES 10000
#define MAX_NUM_DEBUG_POINTS 5000
struct DebugContext
{
	bool initalized;
	u32 num_debug_lines;
	f32 debug_lines[MAX_NUM_DEBUG_LINES];
	u32 num_debug_points;
	f32 debug_points[MAX_NUM_DEBUG_POINTS];
	AssetID shader_id;
} debug_context;

//
// System
//

// Initalize the graphics API.
Context initalize_graphics(AssetID id);
// Free the graphics API
void destroy_graphics(Context *gfx);


//
// Drawing 
//

// Shake the camera by some ammount.
void shake_camera(f32 shake);

// Start a new frame
void frame(Context *gfx, Clock clock, Camera *camera);
void frame(Clock clock);

// Draw a region of a texture as a sprite
void draw_sprite(Context gfx, Texture texture, 
		Rect region, Vec2 position, Vec2 scale, 
		f32 rotation, Vec4 tint=V4(1, 1, 1, 1));
void draw_sprite(Texture texture, 
		Rect region, Vec2 position, Vec2 scale, 
		f32 rotation, Vec4 tint=V4(1, 1, 1, 1));
// Draw the complete texture as a sprite
void draw_sprite(Context gfx, Texture texture, 
		Vec2 position, Vec2 scale, f32 rotation, Vec4 tint=V4(1, 1, 1, 1));
void draw_sprite(Texture texture, 
		Vec2 position, Vec2 scale, f32 rotation, Vec4 tint=V4(1, 1, 1, 1));

void draw_text(AssetID font_id, const char *text, Vec2 position, f32 size, Vec4 color, f32 width=0.5f, f32 edge=0.15f);

Rect generate_sprite(u32 id, u32 tiles_x, u32 tiles_y)
{
	f32 w = (1.0f / (f32) tiles_x);
	f32 h = (1.0f / (f32) tiles_y);
	f32 u = (id % tiles_x) * w;
	f32 v = (id / tiles_y) * h;
	Rect rect = {u, v, w, h};
	return rect;
}

//
// Debug
//

// Queue a line with different colors at different ends.
void debug_line(Vec2 to, Vec4 to_color, Vec2 from, Vec4 from_color);
// Queue a line.
void debug_line(Vec2 to, Vec2 from, Vec4 color=V4(0.5f, 0.75f, 0.3f, 1.0f));
// Queue a point.
void debug_point(Vec2 p, Vec4 color=V4(0.5f, 0.75f, 0.3f, 1.0f));
// Render all the qued debug draws and render them using this camera.
void debug_draw(Camera *camera);
// Render the debug view using the default camera
void debug_draw();

// 
// Shaders
//

// Create the shader program asset
Asset compile_progam(const char *file);
// Update the shader program asset
Asset recompile_program(Asset asset);
// Free up the shader program asset
void destroy_program(Asset asset);

#endif
