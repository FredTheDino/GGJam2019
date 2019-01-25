#define PLAYER_ACCELERATION 10.0f
#define PLAYER_DEACCELERATION 40.0f
#define PLAYER_MAX_SPEED 5
#define PLAYER_JUMP_SPEED 7
#define GRAVITY 20

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
	return player;
}

void player_update(Player *player, f32 delta) 
{
	Body *body = find_body_ptr(player->body_id);
	//
	// Movement
	//

	Vec2 vel = body->velocity;

	//
	// Update gravity
	//
	vel.y -= GRAVITY * delta;

	f32 acc_direction = value("right") + value("left");
	if (acc_direction) {
		vel.x += PLAYER_ACCELERATION * acc_direction;
	}
	else 
	{
		if (vel.x < 0)
		{
			vel.x = minimum(0.0f, vel.x + PLAYER_DEACCELERATION * delta);
		}
		else if (vel.x > 0)
		{
			vel.x = maximum(0.0f, vel.x - PLAYER_DEACCELERATION * delta);
		}
	}
	vel.x = clamp((f32) -PLAYER_MAX_SPEED, (f32) PLAYER_MAX_SPEED, vel.x);

	//
	// Jumping 
	//
	if (!(player->in_air) && pressed("jump")) 
	{
		printf("Jumping!");
		vel.y = PLAYER_JUMP_SPEED;
	}

	body->velocity = vel;

	game.camera->position = body->position;
}

void player_shoot(Player *player, s32 direction) 
{

}

void player_draw(Player player) 
{
	Texture texture = find_asset(pixel).texture;
	draw_sprite(texture, player.body_id->position, V2(1, 1), 0);
}

