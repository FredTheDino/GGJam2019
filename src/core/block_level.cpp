// NOTE: This is really, really, really slow.
// doing string compares is slow as is, all the
// values are hashed so there should be a way to
// find based on pre hashed values of the strings.

Asset load_level(const char *path)
{
#if 0
	const f32 scale_factor = 1.0f / 32.0f;
	const f32 w = 0.5f;
	Vec2 rect_points[] = {V2(-w, -w), V2( w, -w), V2( w,  w), V2(-w,  w)};
	ShapeID rect = register_shape(ARRAY_LENGTH(rect_points), rect_points);
			
	using namespace JSON;
	const char *file = read_entire_file(path);
	Value root = parse_object(file);
	Value layers = root["layers"];

	// Parse the level
	ASSERT(layers.type == ValueType::J_ARRAY);
	for (u32 i = 0; i < layers.length(); i++)
	{
		Value layer = layers[i];
		ASSERT(layer.type == ValueType::J_OBJECT);
		if (str_eq(layer["name"].string.data, "spawns"))
		{
			Value objects = layer["objects"];
			ASSERT(objects.type == ValueType::J_ARRAY);
			for (u32 j = 0; j < objects.length(); j++)
			{
				Value c = objects[j];
				if (str_eq(c["name"], "player"))
				{
					print("placeing player!\n");
					Vec2 position = V2(c["x"], c["y"]) * scale_factor;

					EntityID entity = create_player();
					BodyID body = create_body(rect, entity, 1, 10, 1.0f, 0.9f);
					center(body);
					find_entity_ptr(entity)->position = position;
				}
			}
		}

		if (str_eq(layer["name"], "collision"))
		{
			Value objects = layer["objects"];
			ASSERT(objects.type == ValueType::J_ARRAY);
			for (u32 j = 0; j < objects.length(); j++)
			{
				LevelObject object;
				Value c = objects[j];
				object.position = V2(c["x"], c["y"]) * scale_factor;
				object.rotation = c["rotation"];
				if (c["polygon"].type == ValueType::J_UNDEFINED)
				{
					// It's a rectangle
					object.scale = V2(c["width"], c["height"]) * scale_factor;
					object.position += object.scale * 0.5f;
					object.shape = rect;
				}
				else
				{
					// It's a polygon
					// TODO: Parse the polygon.
					object.scale = V2(1, 1);
					List<Vec2> points = create_list<Vec2>(c["polygon"].length());
					for (u32 i = 0; i < c["polygon"].length(); i++) {
						points.append(V2(
									c["polygon"][i]["x"], 
									c["polygon"][i]["y"]
									) * scale_factor);
					}
					object.shape = register_shape(points);
					destroy_list(&points);
				}

				EntityID entity = create_empty();
				BodyID body = create_body(object.shape, entity, 1, 0);
				center(body);

				Entity *e = find_entity_ptr(entity);
				e->position = object.position;
				e->rotation = object.rotation;
				e->scale = object.scale;

				Body *b = find_body_ptr(body);
				b->scale = object.scale;
			}
		}
	}

#endif
	Asset asset;
	asset.type = AFT_LEVEL;
	return {};
}

void reload_level(Asset *asset)
{
	ASSERT(asset->type == AFT_LEVEL);
	clear_world(game.world);
	//clear_entity_manager(game.manager);
	load_level(asset->path);
}

void destroy_level(Asset asset)
{

}

