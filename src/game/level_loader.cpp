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

void level_load(const char *path)
{
	using namespace JSON;
	Level level;
	level.bodies = create_list<BodyID>(10);
	const char *file = read_entire_file(path);
	Value value = parse_object(file);
	Value objects = value["layers"][(u32) 0]["objects"];
	for (u32 i = 0; i < objects.length(); i++) 
	{
		Vec2 pos = V2(objects[i]["x"], objects[i]["y"]) 
			/ value["tilewidth"];
		Vec2 dim = V2(objects[i]["width"], objects[i]["height"])
			/ value["tilewidth"];
		BodyID id = create_body(0xFF, 0);
		id->position = pos - (dim / 2); 
		id->position = V2(id->position.x, -id->position.y);
		id->scale = dim;
	}
}

