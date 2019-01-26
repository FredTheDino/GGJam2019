#define PLAYER_ACCELERATION 10.0f
#define PLAYER_DEACCELERATION 40.0f
#define PLAYER_MAX_SPEED 5
#define PLAYER_JUMP_SPEED 7
#define MAX_KAYOTEE_TIME 1.0f
#define GRAVITY 20

struct Player {
	BodyID body_id;
	f32 shot_delay;

	// Jumping
	bool jumped;
	bool grounded;
	f32 kayotee_time;
};

bool player_callback(Body *self, Body *other, Overlap overlap)
{
	Player *player = (Player *) self->self;
	if (dot((self->position - other->position), V2(0, 1)) > 0.8)
	{
		player->jumped = false;
		player->grounded = true;
	}
	return false;
}

Player *create_player()
{
	Player *player = push_struct(Player);
	player->body_id = create_body(0xff, 1, 0);
	player->body_id->self = player;
	player->body_id->overlap = player_callback;
	player->shot_delay = 0.5f;
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
	if (player->grounded)
	{
		player->kayotee_time = 0;
	}

	if (player->kayotee_time < MAX_KAYOTEE_TIME && pressed("jump")) 
	{
		vel.y = PLAYER_JUMP_SPEED;
		player->jumped = true;
		player->kayotee_time = MAX_KAYOTEE_TIME;
	}

	player->kayotee_time += delta;
	player->grounded = false;

	body->velocity = vel;

	game.camera->position = body->position;
}

void player_shoot(Player *player, s32 direction) 
{

}

void player_draw(Player *player) 
{
	Texture texture = find_asset(pixel).texture;
	draw_sprite(texture, player->body_id->position, V2(1, 1), 0);
}

