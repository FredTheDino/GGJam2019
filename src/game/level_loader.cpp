enum TileType
{
	TT_EMPTY,
	TT_GROUND_TILE,
	TT_NUM_TILES,
};

struct Level
{
	List<BodyID> bodies;
};

Player level_load(const char *path)
{
	using namespace JSON;
	Player player = create_player();
	Level level;
	level.bodies = create_list<BodyID>(10);
	const char *file = read_entire_file(path);
	Value value = parse_object(file);
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
			player.body_id->position = pos;
			printf("player @ x: %f, y %f\n", pos.x, pos.y);
		}
		else
		{
			BodyID id = create_body(0xFF, 0);
			id->position = pos + dim / 2;
			id->scale = dim;
			printf("block @ x: %f, y %f, dx: %f, dy %f\n", pos.x, pos.y, dim.x, dim.y);
		}
	}
	return player;
}

