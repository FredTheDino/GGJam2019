RandomState rng;

Context initalize_graphics(AssetID id)
{
	Context gfx;
	gfx.shader_id = id;
	rng = seed(8765456789);
	const f32 w = 1.0f;
	f32 square[] = {
		-w, -w,
		 w, -w,
		 w,  w,
		-w,  w
	};

	glGenVertexArrays(1, &gfx.vertex_array);
	glBindVertexArray(gfx.vertex_array);

	glGenBuffers(1, &gfx.vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, gfx.vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, gfx.vertex_buffer);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void *) 0);

	return gfx;
}

void destroy_graphics(Context *gfx)
{
	glDeleteVertexArrays(1, &gfx->vertex_array);
	glDeleteProgram(gfx->vertex_buffer);
}

void frame(Context *gfx, Camera *camera, Clock clock)
{
	// TODO: This is slow, we should have a global context
	// which caches the current state so we don't have to set 
	// it. But it's marginal.
	glUseProgram(find_asset(gfx->shader_id).shader.program);

	camera->shake_timer = maximum(0.0, camera->shake_timer - clock.delta);
	f32 x = clamp(0.0f, 1.0f, camera->shake_timer * 0.88f);
	f32 shake_strength = -x * x * (x - 1.5) * 2;
	shake_strength = minimum(shake_strength, camera->shake_stress);
	ASSERT(shake_strength >= 0.0f);

	Vec2 shake_position = camera->position + random_unit_vec2(&rng) * shake_strength;
	f32 shake_rotation = camera->rotation + random_real_in_range(&rng, -0.2f, 0.2f) * shake_strength;
	f32 shake_zoom = camera->zoom + random_real_in_range(&rng, -0.1f, 0.0f) * shake_strength;

	// TODO: We should however cache this, so we don't have to write
	// super long calls all the time.
	GLuint program = find_asset(gfx->shader_id).shader.program;
	glUseProgram(program);
#define u_loc(attr) (glGetUniformLocation(program, attr))
	glUniform1f(u_loc("time"), clock.time);
	glUniform1f(u_loc("aspect_ratio"), game.aspect_ratio);
	glUniform2f(u_loc("camera_position"), shake_position.x, shake_position.y);
	glUniform1f(u_loc("camera_rotation"), shake_rotation);
	glUniform1f(u_loc("camera_zoom"), shake_zoom);
	glClearColor(0.3f, 0.1f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void frame(Clock clock)
{
	frame((Context *) game.context, (Camera *) game.camera, clock);
}

void draw_sprite(Context gfx, Texture texture, 
		Rect region, Vec2 position, Vec2 scale, f32 rotation, Vec4 tint)
{
	GLuint program = find_asset(gfx.shader_id).shader.program;
	glUseProgram(program);
	bind_texture(texture, 0);
	glUniform2f(u_loc("min_uv"), region.min.x, region.min.y);
	glUniform2f(u_loc("dim_uv"), region.dim.x, region.dim.y);
	glUniform4f(u_loc("tint"), tint.x, tint.y, tint.z, tint.w);
	glUniform1i(u_loc("sprite_texture"), 0);
	glUniform2f(u_loc("position"), position.x, position.y);
	glUniform2f(u_loc("scale"), scale.x * 0.5f, scale.y * 0.5);
	glUniform1f(u_loc("rotation"), rotation);
	glBindVertexArray(gfx.vertex_array);
	glDrawArrays(GL_QUADS, 0, 4);
}

void draw_sprite(Texture texture, Rect region, 
		Vec2 position, Vec2 scale, f32 rotation, Vec4 tint)
{
	draw_sprite((Context) *game.context, (Texture) texture, 
			(Rect) region, (Vec2) position, (Vec2) scale, (f32) rotation, tint);
}

void draw_sprite(Context gfx, Texture texture, 
		Vec2 position, Vec2 scale, f32 rotation, Vec4 tint)
{
	draw_sprite(gfx, texture, R(0, 0, 1, 1), position, scale, rotation, tint);
}

void draw_sprite(Texture texture, Vec2 position, 
		Vec2 scale, f32 rotation, Vec4 tint)
{
	draw_sprite(texture, R(0, 0, 1, 1), position, scale, rotation, tint);
}

void debug_line(Vec2 to, Vec4 to_color, Vec2 from, Vec4 from_color)
{
	debug_context.debug_lines[debug_context.num_debug_lines++] = to.x;
	debug_context.debug_lines[debug_context.num_debug_lines++] = to.y;
	debug_context.debug_lines[debug_context.num_debug_lines++] = to_color.x;
	debug_context.debug_lines[debug_context.num_debug_lines++] = to_color.y;
	debug_context.debug_lines[debug_context.num_debug_lines++] = to_color.z;
	debug_context.debug_lines[debug_context.num_debug_lines++] = to_color.w;
	debug_context.debug_lines[debug_context.num_debug_lines++] = from.x;
	debug_context.debug_lines[debug_context.num_debug_lines++] = from.y;
	debug_context.debug_lines[debug_context.num_debug_lines++] = from_color.x;
	debug_context.debug_lines[debug_context.num_debug_lines++] = from_color.y;
	debug_context.debug_lines[debug_context.num_debug_lines++] = from_color.z;
	debug_context.debug_lines[debug_context.num_debug_lines++] = from_color.w;

	ASSERT(debug_context.num_debug_lines < MAX_NUM_DEBUG_LINES);
}

void debug_line(Vec2 to, Vec2 from, Vec4 color)
{
	debug_context.debug_lines[debug_context.num_debug_lines++] = to.x;
	debug_context.debug_lines[debug_context.num_debug_lines++] = to.y;
	debug_context.debug_lines[debug_context.num_debug_lines++] = color.x;
	debug_context.debug_lines[debug_context.num_debug_lines++] = color.y;
	debug_context.debug_lines[debug_context.num_debug_lines++] = color.z;
	debug_context.debug_lines[debug_context.num_debug_lines++] = color.w;
	debug_context.debug_lines[debug_context.num_debug_lines++] = from.x;
	debug_context.debug_lines[debug_context.num_debug_lines++] = from.y;
	debug_context.debug_lines[debug_context.num_debug_lines++] = color.x;
	debug_context.debug_lines[debug_context.num_debug_lines++] = color.y;
	debug_context.debug_lines[debug_context.num_debug_lines++] = color.z;
	debug_context.debug_lines[debug_context.num_debug_lines++] = color.w;

	ASSERT(debug_context.num_debug_lines < MAX_NUM_DEBUG_LINES);
}

void debug_point(Vec2 p, Vec4 color)
{
	// I know this is dumb. I don't care, I love it.
	debug_context.debug_points[debug_context.num_debug_points++] = p.x;
	debug_context.debug_points[debug_context.num_debug_points++] = p.y;
	debug_context.debug_points[debug_context.num_debug_points++] = color.x;
	debug_context.debug_points[debug_context.num_debug_points++] = color.y;
	debug_context.debug_points[debug_context.num_debug_points++] = color.z;
	debug_context.debug_points[debug_context.num_debug_points++] = color.w;

	ASSERT(debug_context.num_debug_points < MAX_NUM_DEBUG_POINTS);
}

void debug_draw(Camera *camera)
{
	if (!debug_context.initalized)
	{
		debug_context.shader_id = load_asset(AFT_SHADER, "res/debug.glsl");
		debug_context.initalized = true;
	}

	// Setup shader.
	//f32 scale_factor = maximum(2.0f / camera->zoom, 0.05f);
	f32 scale_factor = 2.3f;
	glLineWidth(0.5f * scale_factor);
	glPointSize(1.0f * scale_factor);

	GLuint program = find_asset(debug_context.shader_id).shader.program;
	glUseProgram(program);
	// Transform
	glUniform2f(u_loc("position"), 0, 0);
	glUniform2f(u_loc("scale"), 1, 1);
	glUniform1f(u_loc("rotation"), 0);
	// Camera
#if 1
	glUniform2f(u_loc("camera_position"), camera->position.x, camera->position.y);
	glUniform1f(u_loc("camera_rotation"), camera->rotation);
	glUniform1f(u_loc("camera_zoom"), camera->zoom);
#endif
	glUniform1f(u_loc("aspect_ratio"), game.aspect_ratio);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * debug_context.num_debug_lines, debug_context.debug_lines, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * (2 + 4), (void *) (0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(f32) * (2 + 4), (void *) (sizeof(f32) * 2));

	glDrawArrays(GL_LINES, 0, debug_context.num_debug_lines / 6);
	glDeleteBuffers(1, &vbo);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * debug_context.num_debug_points, debug_context.debug_points, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * (2 + 4), (void *) (0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(f32) * (2 + 4), (void *) (sizeof(f32) * 2));

	glDrawArrays(GL_POINTS, 0, debug_context.num_debug_points / 6);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);

	debug_context.num_debug_lines = 0;
	debug_context.num_debug_points = 0;
}

void debug_draw()
{
	debug_draw(game.camera);
}

struct Vertex
{
	f32 x, y, u, v;
};

f32 messure_text(AssetID font_id, const char *_text, f32 size)
{
	f32 length = 0.0;
	if (!_text) return length;

	Font font = find_asset(font_id).font;
	char *text = (char *) _text;
	char curr, prev = '\0'; // Only asign to prev. 
	while (true)
	{
		curr = *(text++);
		if (!curr) break;

		f32 kerning = find_kerning(font, prev, curr);
		length += kerning;

		Glyph glyph = font.glyphs[curr];

		length += glyph.advance;
		prev = curr;
	}

	return length;
}

void draw_text(AssetID font_id, const char *text, Vec2 position, f32 size, Vec4 color, f32 width, f32 edge)
{
	// NOTE: You can alternatively generate a new VBO and VAO so we onlt send down one
	// draw call with all the text in one. But I don't know if this is faster.
	List<f32> verticies = create_list<f32>(sizeof(f32) * 4 * 6 * str_len(text));
	f32 offset = 0.0;
	Font font = find_asset(font_id).font;
	char *c = (char *) text;
	char curr, prev = '\0'; // Only asign to prev. 
	while (true)
	{
		curr = *(c++);
		if (!curr) break;

		f32 kerning = find_kerning(font, prev, curr);
		const f32 spacing = 1.0f;
		offset += kerning * spacing;

		Glyph glyph = font.glyphs[curr];
		Rect rect = {V2(glyph.x, glyph.y), V2(glyph.w, glyph.h)};

		//
		// V2---V3
		// |     |
		// |     |
		// V1---V4
		//

		// V1
		verticies.append(glyph.x_offset + offset);	
		verticies.append(glyph.y_offset); 			
		verticies.append(rect.min.x); 				
		verticies.append(rect.min.y); 			

		// V2
		verticies.append(glyph.x_offset + offset);
		verticies.append(glyph.y_offset + rect.dim.y);
		verticies.append(rect.min.x);
		verticies.append(rect.min.y + rect.dim.y);

		// V3
		verticies.append(glyph.x_offset + rect.dim.x + offset);
		verticies.append(glyph.y_offset + rect.dim.y);
		verticies.append(rect.min.x + rect.dim.x);
		verticies.append(rect.min.y + rect.dim.y);

		// V3
		verticies.append(glyph.x_offset + rect.dim.x + offset);
		verticies.append(glyph.y_offset + rect.dim.y);
		verticies.append(rect.min.x + rect.dim.x);
		verticies.append(rect.min.y + rect.dim.y);

		// V4
		verticies.append(glyph.x_offset + rect.dim.x + offset);
		verticies.append(glyph.y_offset);
		verticies.append(rect.min.x + rect.dim.x);
		verticies.append(rect.min.y);

		// V1
		verticies.append(glyph.x_offset + offset);	
		verticies.append(glyph.y_offset); 			
		verticies.append(rect.min.x); 				
		verticies.append(rect.min.y); 			

		offset += glyph.advance * spacing;
		prev = curr;
	}

	Texture texture = find_asset(font.texture).texture;
	GLuint program = find_asset(game.text_context->shader_id).shader.program;
	f32 aspect = game.aspect_ratio * (texture.width / texture.height);
	glUseProgram(program);

	bind_texture(texture, 0);
	glUniform1i(u_loc("font_atlas"), 0);

	glUniform1f(u_loc("width"), width);
	glUniform1f(u_loc("edge"), edge);
	glUniform4f(u_loc("color"), color.x, color.y, color.z, color.w);
	glUniform2f(u_loc("position"), position.x, position.y);
	glUniform2f(u_loc("scale"), aspect * size, size);
	glUniform1f(u_loc("rotation"), 0);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticies[0]) * verticies.length, verticies.data, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 4, (void *) (0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 4, (void *) (sizeof(f32) * 2));

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawArrays(GL_TRIANGLES, 0, verticies.length / 4);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	destroy_list(&verticies);
}

void rect(Vec2 position, Vec2 dimension, Vec4 color)
{
	GLuint program = find_asset(game.text_context->shader_id).shader.program;
	f32 aspect = game.aspect_ratio;
	glUseProgram(program);

	glUniform1f(u_loc("width"), 0.0f);
	glUniform1f(u_loc("edge"), 0.0f);
	glUniform4f(u_loc("color"), color.x, color.y, color.z, color.w);
	glUniform2f(u_loc("position"), position.x, position.y);
	glUniform2f(u_loc("scale"), aspect * dimension.x, dimension.y);
	glUniform1f(u_loc("rotation"), 0);

	glBindVertexArray(game.context->vertex_array);
	glDrawArrays(GL_QUADS, 0, 4);
}

#define COMPILATION_FAILED (-1)
Asset compile_progam(const char *file)
{
	const char *source = read_entire_file(file);
	if (!source)
	{
		Asset asset;
		asset.shader.program = COMPILATION_FAILED;
		return asset;
	}

	const char *preamble = "#version 130\n";
#define NUM_SHADER_PARTS 3
	const char *vert_source[NUM_SHADER_PARTS] = {preamble, "#define VERT\n", source};
	const char *frag_source[NUM_SHADER_PARTS] = {preamble, "#define FRAG\n", source};

	GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
	GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vert_shader, NUM_SHADER_PARTS, vert_source, NULL);
	glShaderSource(frag_shader, NUM_SHADER_PARTS, frag_source, NULL);
	free_string(source);

#undef NUM_SHADER_PARTS

	glCompileShader(vert_shader);
	{
		GLuint shader = vert_shader;
		GLint is_compiled = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &is_compiled);
		if(is_compiled == GL_FALSE)
		{
			GLint max_length = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_length);

			// The max_length includes the NULL character
			GLchar *log = (GLchar *) push_memory(sizeof(char) * max_length);
			glGetShaderInfoLog(shader, max_length, &max_length, log);
			print("|| (VERT) GLSL %s\n", log);
			pop_memory(log);

			glDeleteShader(frag_shader);
			glDeleteShader(vert_shader);
			Asset asset;
			asset.shader.program = COMPILATION_FAILED;
			return asset;
		}
	}

	glCompileShader(frag_shader);
	{
		GLuint shader = frag_shader;
		GLint is_compiled = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &is_compiled);
		if(is_compiled == GL_FALSE)
		{
			GLint max_length = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_length);

			// The max_length includes the NULL character
			GLchar *log = (GLchar *) push_memory(sizeof(GLchar) * max_length);
			glGetShaderInfoLog(shader, max_length, &max_length, log);
			print("|| (FRAG) GLSL %s\n", log);
			pop_memory(log);

			glDeleteShader(frag_shader);
			glDeleteShader(vert_shader);
			Asset asset;
			asset.shader.program = COMPILATION_FAILED;
			return asset;
		}
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, vert_shader);
	glAttachShader(program, frag_shader);
	glLinkProgram(program);
	glDetachShader(program, vert_shader);
	glDetachShader(program, frag_shader);

	glDeleteShader(vert_shader);
	glDeleteShader(frag_shader);

	Asset asset;
	asset.shader.program = program;
	return asset;
}

void recompile_program(Asset *asset)
{
	ASSERT(asset->type == AFT_SHADER);

	Shader new_shader = compile_progam(asset->path).shader;
	if (new_shader.program != (u64) COMPILATION_FAILED)
	{
		// Silent success
		//print("|| Shader reloaded (%s@%d)\n", asset.path, asset.last_edit);
		asset->shader = new_shader;
	}
}

void destroy_program(Asset asset)
{
	ASSERT(asset.type == AFT_SHADER);

}
#undef COMPILATION_FAILED


