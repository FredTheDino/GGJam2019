#define BOUNCE_SPEED 20

bool jello_on_collision(Body *self, Body *other, Overlap overlap) 
{
	ASSERT(self->self);

	if (other->type == PICKUP_TYPE)
		return false;

	if (other->inverse_mass == 0 || other->type == JELLO_TYPE)
		return true;

	if (other->type == PLAYER_TYPE || other->type == SHOT_TYPE) 
	{
		Vec2 pos;
		if (other->type == PLAYER_TYPE)
			pos = ((Player*)other)->body_id->position;
		else
			pos = ((Shot*)other)->body_id->position;

		for (int i = 0; i < 20; i++)
		{
			Particle p = {};
			p.position = pos;
			p.lifetime = 10;
			p.from_color = V4(0, 1, 0, 1);
			p.to_color = V4(0, 1, 0, 1);
			p.linear_velocity = random_vec2(&rnd, V2(-1, 0.5f), V2(1, 1));
			p.scale = V2(0.1f, 0.1f);
			p.gravity = GRAVITY/900.0f;
			add_particle(&particle_system, p);
		}
	}

	Jello *jello = (Jello*) self->self;

	other->velocity.y = BOUNCE_SPEED;
	
    play_sound_perturbed(audio_splat, 1, 1, 0.5, 1);

	return false;
}
Jello *create_jello(Shot *shot)
{
	BodyID body_id = create_body(0xFF, 128);
	body_id->position = shot->body_id->position;
	body_id->scale = V2(1, 0.2f);
	body_id->overlap = jello_on_collision;
	body_id->type = JELLO_TYPE;
	Jello *jello = push_struct(Jello);
	jello->body_id = body_id;
	body_id->self = jello;
	return jello;
}

void destroy_jello(Jello *jello) 
{
	destroy_body(jello->body_id);
	pop_memory(jello);
}

void update_jellos(List<Jello*> *jellos, f32 delta)
{

	for (s32 i = jellos->length-1; i >= 0; i--) 
	{
		Jello *jello = (*jellos)[i]; 
		jello->alive_time += delta;
		jello->body_id->velocity = V2(0, 0);

		if (jello->alive_time > JELLO_ALIVE_TIME) 
		{
			destroy_jello(jello);
			jellos->remove(i);
		}
	}
}

void jellos_draw (List<Jello*> *jellos) 
{
	for (u32 i = 0; i < jellos->length; i++) 
	{
		BodyID body_id = (*jellos)[i]->body_id;
		draw_sprite(32*3+1, body_id->position, V2(1.0f, 0.5f));
	}
}
