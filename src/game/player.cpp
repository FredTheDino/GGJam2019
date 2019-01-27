#define PLAYER_ACCELERATION 10.0f
#define PLAYER_DEACCELERATION 40.0f
#define PLAYER_MAX_SPEED 5
#define PLAYER_JUMP_SPEED 7
#define PLAYER_SHOT_DELAY 0.3f
#define MAX_KAYOTEE_TIME 1.0f
#define ANIMATION_TIME 0.5f

struct Player {
	// Body
	BodyID body_id;
	// Shooting
	f32 shot_time;
	ShotKind weapon;
	// Jumping
	bool jumped;
	bool grounded;
	bool bounced;
	f32 kayotee_time;
	f32 animation_timer; 
	// Direction
	s32 face_direction;
};

bool player_callback(Body *self, Body *other, Overlap overlap)
{
	Player *player = (Player *) self->self;

	if (other->type == PICKUP_TYPE)
		return false;

	if (dot(-overlap.normal, V2(0, 1)) > 0.8)
	{
		if (player->kayotee_time != 0.0f)
		{
			shake_camera(0.05f);
		}
		player->jumped = false;
		player->grounded = true;
		player->bounced = false;
	}
	if (other->type == JELLO_TYPE) 
	{
		player->bounced = true;
	}
	return false;
}

Player *create_player()
{
	Player *player = push_struct(Player);
	player->body_id = create_body(0xff, 1, 0);
	player->body_id->self = player;
	player->body_id->overlap = player_callback;
	player->body_id->scale = V2(1,1);
	player->body_id->type = PLAYER_TYPE;
	player->shot_time = 0;
	player->weapon = JELLO;
	player->face_direction = 1;
	player->animation_timer = 0;
	return player;
}

void player_update(Player *player, f32 delta) 
{
	player->animation_timer += delta;

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
		player->face_direction = sign_no_zero(vel.x);
	}
	else 
	{
		if (vel.x < 0)
		{
			vel.x = minimum(0.0f, vel.x + PLAYER_DEACCELERATION * delta);
			player->face_direction = -1;
		}
		else if (vel.x > 0)
		{
			vel.x = maximum(0.0f, vel.x - PLAYER_DEACCELERATION * delta);
			player->face_direction = 1;
		}
	}

	if (player->grounded && acc_direction)
	{
		movement_particles(player);
	}

	vel.x = clamp((f32) -PLAYER_MAX_SPEED, (f32) PLAYER_MAX_SPEED, vel.x);

	//
	// Jumping 
	//
	player->kayotee_time += delta;
	if (player->grounded)
	{
		player->kayotee_time = 0;
	}

	if (player->kayotee_time < MAX_KAYOTEE_TIME && pressed("jump")) 
	{
		shake_camera(0.01f);
		vel.y = PLAYER_JUMP_SPEED;
		player->jumped = true;
		player->kayotee_time = MAX_KAYOTEE_TIME;
        play_sound(audio_hop, 0.3, 1);
		jump_particles(player);
	}
	player->grounded = false;

	if (player->bounced)
		bounce_particles(player);


	body->velocity = vel;

	//
	// Shooting
	//
	player->shot_time += delta;

	game.camera->position = body->position;

}


void player_shoot(Player *player, List<Shot*> *shots, List<Jello*> *jellos) 
{
	if (player->shot_time > PLAYER_SHOT_DELAY)
	{
		shake_camera(0.2f);
		shots->append(create_shot(player, player->weapon, player->face_direction, jellos));
		player->shot_time = 0;
	}
}

void player_draw(Player *player) 
{
	int sprite_index = 65;
	if (not player->grounded)
	{
		if (player->body_id->velocity.y > 0)
		{
			sprite_index = 67;
		}
		else
		{
			sprite_index = 66;
		}
	}
	else if (player->body_id->velocity.x == 0)
	{
		sprite_index = 65;
	}
	else if ((s32)(player->animation_timer / ANIMATION_TIME) % 2 == 0)
	{
		sprite_index = 64;
	}

	if (player->face_direction == 1) {
		draw_sprite(sprite_index, player->body_id->position, hadamard(player->body_id->scale, V2(-1,1)));
	}

	else {
		draw_sprite(sprite_index, player->body_id->position);
	}
}

void player_change_weapon(Player *player, ShotKind weapon) {
	player->weapon = weapon;
}
