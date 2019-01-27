struct KillFloor {
	BodyID body_id;
};


bool killfloor_on_collision(Body *self, Body *other, Overlap overlap) {
	if (other->type == PLAYER_TYPE) {
		player_respawn((Player*)other->self);
	}
	return false;
}

KillFloor *create_killfloor(List<KillFloor*> *killfloors, Vec2 position) {
	BodyID body_id = create_body(0xFF, 0, 0.1f, 0.1f, true);
	body_id->position = position + V2(0, -0.2);
	body_id->scale = V2(0.8f, 0.4f);
	body_id->overlap = killfloor_on_collision;
	KillFloor *killfloor = push_struct(KillFloor);
	killfloor->body_id = body_id;
	killfloors->append(killfloor);
	return killfloor;
}

void destroy_killfloor(KillFloor *killfloor)
{
	destroy_body(killfloor->body_id);
	pop_memory(killfloor);
}


void killfloors_draw (List<KillFloor*> *killfloors) 
{
	for (u32 i = 0; i < killfloors->length; i++) 
	{
		KillFloor *killfloor = (*killfloors)[i];
		BodyID body_id = killfloor->body_id;
		u32 sprite = 131;
		draw_sprite(sprite, body_id->position + V2(0, +0.2), V2(1,1), 0);
	}
}
