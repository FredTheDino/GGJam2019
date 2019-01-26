#define BOUNCE_SPEED 20

bool jello_on_collision(Body *self, Body *other, Overlap overlap) 
{
	ASSERT(self->self);

	if (other->inverse_mass == 0 || other->inverse_mass == JELLO_INV_MASS)
		return true;

	other->velocity.y = BOUNCE_SPEED;
	
	return false;
}
Jello *create_jello(Shot *shot)
{
	BodyID body_id = create_body(0xFF, 128);
	body_id->position = shot->body_id->position;
	body_id->scale = V2(1, 0.2f);
	body_id->overlap = jello_on_collision;
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
		Texture texture = find_asset(pixel).texture;
		BodyID body_id = (*jellos)[i]->body_id;
		draw_sprite(texture, body_id->position, body_id->scale, 0);
	}
}
