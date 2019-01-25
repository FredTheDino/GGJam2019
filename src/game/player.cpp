
struct Player {
	BodyID body_id;
	bool in_air;
	f32 shot_delay;
};

Player create_player()
{
	Player player = {};
	player.body_id = create_body(0xff, 1, 0);
	player.in_air = false;
	player.shot_delay = 0.5f;
}


void player_update(Player *p, f32 delta) 
{

}

void player_shoot(Player *p, s32 direction) 
{

}

void player_draw() 
{

}

void player_jump(Player *p) 
{
	if (!p->in_air) 
	{

	}
}

