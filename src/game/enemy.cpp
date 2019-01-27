#define ENEMY_ACCELERATION 10.0f
#define ENEMY_DEACCELERATION 40.0f
#define ENEMY_MAX_SPEED 5
#define ENEMY_START_HP 3

struct Enemy {
    BodyId body_id;
    u32 hp;
	bool crying;
	s32 face_direction;
	f32 animation_timer;
};

bool enemy_callback(Body *self, Body *other, Overlap overlap)
{
    Enemy *enemy = (Enemy *) self->self;
    return 0;
}

Enemy *create_enemy()
{
    Enemy *enemy = push_struct(Enemy);
    enemy->body_id = create_body(0xff, 1, 0);
    enemy->body_id->self = enemy;
    enemy->body_id->velocity.x = ENEMY_MAX_SPEED;
    enemy->hp = ENEMY_START_HP;
	enemy->face_direction = 1;
	enemy->animation_timer = 0;

    return enemy;
}

void enemy_draw(Enemy *enemy)
{
	int sprite_index = 128;
	if ((s32)(enemy->animation_timer / 0.5f) % 2 == 0)
	{
		sprite_index = 129;
	}

	if (enemy->crying && (s32)(enemy->animation_timer / 0.2f) % 2 == 0)
	{
		sprite_index += 4
	}

	if (enemy->face_direction == 1)
	{
		draw_sprite(sprite_index, enemy->body_id->position, hadamard(enemy->body_id->scale, V2(-1,1)));
	}
	else
	{
		draw_sprite(sprite_index, player->body_id->position);
	}
}

void destroy_enemy(Enemy *enemy)
{
    destroy_body(enemy->body_id);
    pop_memory(enemy);
}


