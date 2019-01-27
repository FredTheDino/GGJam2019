enum TileType
{
	TT_EMPTY,
	TT_GROUND_TILE,
	TT_NUM_TILES,
};

struct Level
{
	List<BodyID> bodies;
	TileMap map;
};

Player *level_load(const char *path, Level *level)
{
	using namespace JSON;
	// Player p = create_player();
	Player *player = create_player();
	level->map = create_tilemap(spritesheet);
	level->bodies = create_list<BodyID>(10);
	const char *file = read_entire_file(path);
	Value value = parse_object(file);
	Value tileset = value["tilesets"][(u32)0];

	// Just assume the first layer is objects. (Smart.)
	Value objects = value["layers"][(u32) 0]["objects"];
	for (u32 i = 0; i < objects.length(); i++) 
	{
		Vec2 pos = V2(objects[i]["x"], objects[i]["y"]) 
			/ value["tilewidth"];
		pos.y *= -1;
		Vec2 dim = V2(objects[i]["width"], objects[i]["height"])
			/ value["tilewidth"];
		dim.y *= -1;
		if (dim.x == 0 && dim.y == 0)
		{
			player->body_id->position = pos;
		}
		else
		{
			BodyID id = create_body(0xFF, 0);
			id->position = pos + dim / 2;
			id->scale = dim;
		}
	}
	objects = value["layers"][(u32) 1]["chunks"];
	List<Vec4> verts = create_list<Vec4>(16*16*4);
	for (u32 i = 0; i < objects.length(); i++) 
	{
		Value chunk = objects[i];
		Vec2 p = V2(chunk["x"], -(f32) chunk["y"]);
		u32 width  = (u32) chunk["width"].number;
		u32 height = (u32) chunk["height"].number;
		verts.clear();
		for (u32 j = 0; j < chunk["data"].length(); j++)
		{
			u32 tile = (f32) chunk["data"][j];
			if (!tile) continue;
			tile--;
			u32 x = j % width;
			u32 y = j / width;
			Rect r = SPRITE(tile);
#if 0
			f32 u = (tile % 32) / 32.0f;
			f32 v = (tile / 32) / 32.0f;/ 32.0f;
#endif

			verts.append(V4(x    , y    , r.x      , r.y));
			verts.append(V4(x    , y + 1, r.x      , r.y + r.h));
			verts.append(V4(x + 1, y + 1, r.x + r.w, r.y + r.h));
			verts.append(V4(x + 1, y    , r.x + r.w, r.y));
		}
		add_chunk(&level->map, p, verts);
	}

	// Done with it.
	destroy_object(value);
	return player;
}

