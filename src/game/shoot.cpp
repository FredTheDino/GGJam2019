Shot *create_shot(Player *player, ShotKind shot_kind, s32 direction)
{
    BodyID body_id = create_body(0xFF, 1);
    body_id->velocity.x = direction * SHOT_SPEED;
	body_id->position = player->body_id->position + V2(direction, 0);
	body_id->scale = V2(0.5f, 0.5f);
    Shot *shot = push_struct(Shot);
    shot->body_id = body_id;
    shot->shot_kind = shot_kind;
	shot->alive_time = 0;
    return shot;
}

void destroy_shot(Shot *shot)
{
    destroy_body(shot->body_id);
    pop_memory(shot);
}

void shots_draw (List<Shot*> *shots) 
{
	for (u32 i = 0; i < shots->length; i++) 
	{
		Texture texture = find_asset(pixel).texture;
		BodyID body_id = (*shots)[i]->body_id;
		draw_sprite(texture, body_id->position, body_id->scale, 0);
	}
}

void update_shots (List<Shot*> *shots, f32 delta) 
{
	for (s32 i = shots->length-1; i >= 0; i--) 
	{
		Shot *shot = (*shots)[i]; 
		shot->alive_time += delta;

		if (shot->alive_time > SHOT_ALIVE_TIME) 
		{
			destroy_shot(shot);
			shots->remove(i);
		}
	}
}

bool shot_on_collision(Body *self, Body *other, Overlap overlap)
{
    Shot *shot = (Shot *) self->self;
    switch (shot->shot_kind) {
    case JELLO:
        break;

    case CARROT:
        break;

    case ONION:
        break;
    }
    return true;
}

