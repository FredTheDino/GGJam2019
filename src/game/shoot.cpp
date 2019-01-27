bool shot_on_collision(Body *self, Body *other, Overlap overlap)
{
	ASSERT(self->self);
    Shot *shot = (Shot *) self->self;

	if (other->type == PICKUP_TYPE || other->type == PLAYER_TYPE)
		return false;

	if (shot->is_destroyed || other->type == JELLO_TYPE)
		return true;

    switch (shot->shot_kind)
	{
		case JELLO:
			shot->jello_list->append(create_jello(shot));
			break;

		case CARROT:
			break;

		case ONION:
			break;
    }
	shot->is_destroyed = true;
    return true;
}

Shot *create_shot(Player *player, ShotKind shot_kind, s32 direction, List<Jello*> *jellos)
{
    BodyID body_id = create_body(0xFF, 1);
    body_id->velocity.x = direction * SHOT_SPEED;
	body_id->position = player->body_id->position + V2(direction, 0);
	body_id->scale = V2(0.5f, 0.5f);
	body_id->overlap = shot_on_collision;
	body_id->type = SHOT_TYPE;
    Shot *shot = push_struct(Shot);
    shot->body_id = body_id;
    shot->shot_kind = shot_kind;
	shot->alive_time = 0;
	shot->is_destroyed = false;
	shot->jello_list = jellos;
	body_id->self = shot;
    play_sound_perturbed(audio_pew, 1, 1, 0.01, 0.075);
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
		BodyID body_id = (*shots)[i]->body_id;
		int texture_id;
		switch ((*shots)[i]->shot_kind) {
			case JELLO:
				texture_id = 96;
				break;

			case CARROT:
				texture_id = 100;
				break;

			case ONION:
				texture_id = 98;
				break;
		}

		draw_sprite(texture_id, body_id->position, body_id->scale * 2, -angle(body_id->velocity));
	}
}

void update_shots (List<Shot*> *shots, f32 delta) 
{
	for (s32 i = shots->length-1; i >= 0; i--) 
	{
		Shot *shot = (*shots)[i]; 
		shot->alive_time += delta;

		if (shot->shot_kind == JELLO) 
		{
			shot->body_id->velocity.y -= GRAVITY * delta;
		}

		if (shot->alive_time > SHOT_ALIVE_TIME || shot->is_destroyed) 
		{
			destroy_shot(shot);
			shots->remove(i);
		}
	}
}

