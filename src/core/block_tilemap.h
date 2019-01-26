#ifndef __BLOCK_TILEMAP__
#define __BLOCK_TILEMAP__

#define CHUNK_WIDTH 16
#define CHUNK_HEIGHT 16

// 1 unit is one tile.

struct TileChunk
{
	Vec2 position;
	u32 draw_length;
	u32 buffer_object;
	u32 vertex_array;
};

struct TileMap
{
	AssetID spritesheet;
	List<TileChunk> chunks;
};

TileMap create_tilemap(AssetID spritesheet)
{
	TileMap map = {};
	map.spritesheet = spritesheet;
	map.chunks = create_list<TileChunk>(10);
	return map;
}

void add_chunk(TileMap *map, Vec2 position, List<Vec4> verts)
{
	// Each quad is 2 2d-coordinates.
	ASSERT(verts.length % 4 == 0);

	TileChunk chunk = {};
	chunk.position = position;
	chunk.draw_length = verts.length;
	glGenBuffers(1, &chunk.buffer_object);
	glGenVertexArrays(1, &chunk.vertex_array);

	glBindVertexArray(chunk.vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, chunk.buffer_object);
	glBufferData(GL_ARRAY_BUFFER, 
			verts.length * sizeof(Vec4), 
			verts + 0, 
			GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec4), (void *) (0 * sizeof(f32)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vec4), (void *) (2 * sizeof(f32)));

	map->chunks.append(chunk);

	glBindVertexArray(0);
}

void draw_tilemap(Context *gfx, TileMap *map, Vec2 point=V2(0, 0), f32 radius=0.0f)
{
	GLuint program = find_asset(gfx->shader_id).shader.program;
	glUseProgram(program);

#define u_loc(attr) (glGetUniformLocation(program, attr))
	Texture texture = find_asset(map->spritesheet).texture;
	bind_texture(texture, 0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (u32 i = 0; i < map->chunks.length; i++) 
	{
		TileChunk *chunk = map->chunks + i;
		if (length_squared(chunk->position - point) < radius || radius == 0.0f)
		{
			glUniform1i(u_loc("tilemap"), 1);
			glUniform2f(u_loc("position"), chunk->position.x, chunk->position.y);
			glUniform2f(u_loc("scale"), 1, -1);
			glUniform1f(u_loc("rotation"), 0);
			glUniform4f(u_loc("tint"), 1, 1, 1, 1);
			glUniform1i(u_loc("sprite_texture"), 0);

			glBindVertexArray(chunk->vertex_array);
			glDrawArrays(GL_QUADS, 0, chunk->draw_length);
		}
	}
}

#endif



