#define ENEMY_ACCELERATION 10.0f
#define ENEMY_DEACCELERATION 40.0f
#define ENEMY_MAX_SPEED 2
#define ENEMY_START_HP 3

#define ENEMY_SPRITE 0x80

struct Enemy {
    BodyID body_id;
    u32 hp;
    s32 facing;
	bool crying;
	f32 animation_timer;
};

// Fixed your buggy ass code, you lazy son of a gun. <3
void destroy_enemy(Enemy *enemy);
bool enemy_callback(Body *self, Body *other, Overlap overlap)
{
    Enemy *enemy = (Enemy *) self->self;
    if (other->type == SHOT_TYPE) {
	Shot *shot = (Shot *) other->self;
    if (shot->shot_kind == ONION) {
        enemy->crying = true;
    }
	destroy_shot(shot);
	enemy->hp--;
	if (enemy->hp <= 0) {
            destroy_enemy(enemy);
	}
	return 1;
    } else if (other->inverse_mass == 0) {
	s32 dir = overlap.normal.x;
	if (dir)
		enemy->facing = -dir;
	return 0;
    }
    return 0;
}


Enemy *create_enemy(List<Enemy*> *enemies, Vec2 position)
{
    Enemy *enemy = push_struct(Enemy);
    enemy->facing = 1;
    enemy->body_id = create_body(0x2, 1, 0);
    enemy->body_id->self = enemy;
    enemy->body_id->position = position;
    enemy->body_id->overlap = enemy_callback;
    //enemy->body_id->velocity.x = ENEMY_MAX_SPEED;
    enemy->hp = ENEMY_START_HP;
	enemy->facing = 1;
	enemy->animation_timer = 0;
	enemies->append(enemy);

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
		sprite_index += 4;
	}

    draw_sprite(sprite_index, enemy->body_id->position, hadamard(enemy->body_id->scale, V2(enemy->facing,1)));
}

void destroy_enemy(Enemy *enemy)
{
    destroy_body(enemy->body_id);
    pop_memory(enemy);
}

void enemy_update(Enemy *enemy, f32 delta)
{
    BodyID body_id = enemy->body_id;
    Vec2 *vel = &body_id->velocity;
    vel->y -= GRAVITY * delta;
    vel->x = ENEMY_MAX_SPEED * enemy->facing;
    enemy->animation_timer += delta;
}

void enemies_draw (List<Enemy*> *enemies) 
{
	for (u32 i = 0; i < enemies->length; i++) 
	{
		Enemy *enemy = (*enemies)[i]; 
		enemy_draw(enemy);
	}
}

void update_enemies (List<Enemy*> *enemies, f32 delta) 
{
	for (s32 i = enemies->length-1; i >= 0; i--) 
	{
		Enemy *enemy = (*enemies)[i]; 
		enemy_update(enemy, delta);
	}
}

