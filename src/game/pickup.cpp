struct Pickup {
	BodyID body_id;
	ShotKind weapon;
    bool can_play_sound;
};

bool pickup_on_collision(Body *self, Body *other, Overlap overlap) {
	ASSERT(self->self);
	Pickup *pickup = (Pickup*)self->self;
	if (other->type == PLAYER_TYPE) {
		Player *player = (Player*)other->self;
		player->weapon = pickup->weapon;
        if(pickup->can_play_sound){
            pickup->can_play_sound = false;
            play_sound_perturbed(audio_pickup, 1, 1, 0.25, 0.25);
        }
	}
	return false;
}

bool pickup_on_uncollide(Body *self, Body *other, Overlap overlap){
    Pickup *me = (Pickup *)self->self;
    me->can_play_sound = true;
    return false;
}

Pickup *create_pickup (List<Pickup*> *pickups, Vec2 position, ShotKind weapon)
{
	BodyID body_id = create_body(0xFF, 0, 0.1f, 0.1f, true);
	body_id->position = position;
	body_id->scale = V2(0.8, 0.8);
	body_id->overlap = pickup_on_collision;
	body_id->overlapnt = pickup_on_uncollide;
    body_id->type = PICKUP_TYPE;
	Pickup *pickup = push_struct(Pickup);
	pickup->body_id = body_id;
	pickup->weapon = weapon;
    pickup->can_play_sound = true;
	body_id->self = pickup;
	pickups->append(pickup);
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
		Pickup *pickup = (*pickups)[i];
		BodyID body_id = pickup->body_id;
		print("%f, %f\n", body_id->position.x, body_id->position.y);
		u32 sprite = 97 + (pickup->weapon * 2);
		draw_sprite(sprite, body_id->position, body_id->scale, 0);
	}
}
