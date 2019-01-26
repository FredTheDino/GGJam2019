struct Pickup {
	BodyID body_id;
	ShotKind weapon;
};

bool pickup_on_collision(Body *self, Body *other, Overlap overlap) {
	ASSERT(self->self);
	Pickup *pickup = (Pickup*)self->self;
	if (other->type == PLAYER_TYPE) {
		Player *player = (Player*)other->self;
		player->weapon = pickup->weapon;
	}
	return false;
}

Pickup *create_pickup (List<Pickup*> *pickups, Vec2 position, ShotKind weapon)
{
	BodyID body_id = create_body(0xFF, 0, 0.1f, 0.1f, true);
	body_id->position = position;
	body_id->scale = V2(0.8, 0.8);
	body_id->overlap = pickup_on_collision;
	body_id->type = PICKUP_TYPE;
	Pickup *pickup = push_struct(Pickup);
	pickup->body_id = body_id;
	pickup->weapon = weapon;
	body_id->self = pickup;
	return pickup;
}

void destroy_pickup(Pickup *pickup)
{
	destroy_body(pickup->body_id);
	pop_memory(pickup);
}


void pickups_draw (List<Pickup*> *pickups) 
{
	for (u32 i = 0; i < pickups->length; i++) 
	{
		Texture texture = find_asset(pixel).texture;
		BodyID body_id = (*pickups)[i]->body_id;
		draw_sprite(texture, body_id->position, body_id->scale, 0);
	}
}
