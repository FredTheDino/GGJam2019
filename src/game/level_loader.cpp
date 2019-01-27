enum TileType
{
	TT_EMPTY,
	TT_GROUND_TILE,
	TT_NUM_TILES,
};

struct EndPoint
{
	Vec2 position;
	const char *next_level;
};

struct Level
{
	bool initalized;

	List<BodyID> bodies;
	TileMap map;

	List<Shot*> shots;
	List<Jello*> jellos;
	List<Pickup*> pickups;
	List<KillFloor*> killfloors;

	Player *player;

	EndPoint end;
};


void draw_end(EndPoint end)
{
	draw_sprite(32 * 5, end.position, V2(1, 1), game.clock.time * 0.5);
}

bool on_end_overlap(Body *self, Body *other, Overlap overlap);

void level_load(const char *path, Level *level)
{
	using namespace JSON;

	if (level->initalized)
	{
		clear_world(game.world);

		for (u32 i = 0; i < level->jellos.length; i++)
		{
			pop_memory(level->jellos[i]);
		}
		level->jellos.clear();

		for (u32 i = 0; i < level->pickups.length; i++)
		{
			pop_memory(level->pickups[i]);
		}
		level->pickups.clear();

		for (u32 i = 0; i < level->shots.length; i++)
		{
			pop_memory(level->shots[i]);
		}
		level->shots.clear();

		for (u32 i = 0; i < level->killfloors.length; i++)
		{
			pop_memory(level->killfloors[i]);
		}
		level->killfloors.clear();

		pop_memory((void *) level->end.next_level);

		pop_memory(level->player);
	}
	
	level->initalized = true;
	level->player = create_player();
	level->shots = create_list<Shot*>(5); 

	level->jellos = create_list<Jello*>(20); 
	level->pickups = create_list<Pickup*>(10);
	level->killfloors = create_list<KillFloor*>(20);
	level->map = create_tilemap(spritesheet);
	level->bodies = create_list<BodyID>(10);
	const char *file = read_entire_file(path);
	ASSERT(file);
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
		if (objects[i]["name"].string.data[0] == 'p')
		{
			level->player->body_id->position = pos;
			level->player->respawn_pos = pos;
		}
		else if (objects[i]["name"].string.data[0] == 't')
		{
			print("todo-spawn tbone\n");
		}
		else if (objects[i]["name"].string.data[0] == 'c')
		{
			create_pickup(&level->pickups, pos, CARROT);

		}
		else if (objects[i]["name"].string.data[0] == 'j')
		{
			create_pickup(&level->pickups, pos, JELLO);
		}
		else if (objects[i]["name"].string.data[0] == 'o')
		{
			create_pickup(&level->pickups, pos, ONION);
		}
		else if (objects[i]["name"].string.data[0] == 'k')
		{
			create_killfloor(&level->killfloors, pos + V2(0.45f, -0.5f));
		}
		else if (objects[i]["name"].string.data[0] == 'e')
		{
			level->end.position = pos;
			level->end.next_level = str_copy(objects[i]["properties"][(u32)0]["value"]);
			print("next: %s\n", level->end.next_level);
			BodyID body = create_body(0xFF, 0, 0, 0, true);
			body->position = level->end.position;
			body->self = level;
			body->overlap = on_end_overlap;
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
	pop_memory((void *) file);
	destroy_object(value);
}

bool on_end_overlap(Body *self, Body *other, Overlap overlap)
{
	if (other->type != PLAYER_TYPE)
		return true;

	Level *level = (Level *) self->self;
	print("Loading level %s\n", level->end.next_level);
	level_load(level->end.next_level, level);
	return true;
}

