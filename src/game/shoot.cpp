Shot *create_shot(Player *player, ShotKind shot_kind, s32 direction)
{
    BodyID body_id = create_body(0xFF, 1);
    body_id->velocity.x = direction * SHOT_SPEED;
	body_id->position = player->body_id->position + V2(direction, 0);
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

void shot_draw (Shot *shot) 
{
	Texture texture = find_asset(pixel).texture;
	draw_sprite(texture, shot->body_id->position, V2(0.5f, 0.5f), 0);
}

void update_shots (List<Shot*> *shots, f32 delta) 
{
	for (int i = shots->length-1; i >= 0; i--) 
	{
		(*shots)[i]->alive_time += delta;

		if ((*shots)[i]->alive_time > SHOT_ALIVE_TIME) 
		{
			destroy_shot((*shots)[i]);
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

