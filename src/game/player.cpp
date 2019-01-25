
#define PLAYER_ACCELERATION 300
#define PLAYER_MAX_SPEED 5
#define PLAYER_JUMP_SPEED 3
#define GRAVITY 2

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
	Vec2 vel = p->body_id->velocity;
	int input_value = value("right") + value("left");
	if (input_value) {
		vel.x = PLAYER_MAX_SPEED * sign(input_value);
	}
	else {
		vel.x = 0;
	}

	// Update gravity
	vel.y -= GRAVITY * delta;

	p->body_id->velocity = vel;
}

void player_shoot(Player *p, s32 direction) 
{

}

void player_draw() 
{

}

void player_jump(Player *p) 
{
	if (!(p->in_air) && pressed("jump")) 
	{
		p->body_id->velocity.y = PLAYER_JUMP_SPEED;
	}
}

