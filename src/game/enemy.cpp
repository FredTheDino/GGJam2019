#define ENEMY_ACCELERATION 10.0f
#define ENEMY_DEACCELERATION 40.0f
#define ENEMY_MAX_SPEED 2
#define ENEMY_START_HP 3

#define ENEMY_SPRITE 0x80

struct Enemy {
    BodyID body_id;
    u32 hp;
    s32 facing;
};

// Fixed your buggy ass code, you lazy son of a gun. <3
void destroy_enemy(Enemy *enemy);
bool enemy_callback(Body *self, Body *other, Overlap overlap)
{
    Enemy *enemy = (Enemy *) self->self;
    if (other->type == SHOT_TYPE) {
	Shot *shot = (Shot *) other->self;
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
	printf("enemy now facing: %d\n", enemy->facing);
	return 0;
    }
    return 0;
}


Enemy *create_enemy(Vec2 position)
{
    Enemy *enemy = push_struct(Enemy);
    enemy->facing = 1;
    enemy->body_id = create_body(0x2, 1, 0);
    enemy->body_id->self = enemy;
    enemy->body_id->position = position;
    enemy->body_id->overlap = enemy_callback;
    //enemy->body_id->velocity.x = ENEMY_MAX_SPEED;
    enemy->hp = ENEMY_START_HP;

    return enemy;
}

void destroy_enemy(Enemy *enemy)
{
    destroy_body(enemy->body_id);
    pop_memory(enemy);
}

void enemy_draw(Enemy *enemy)
{
    BodyID body_id = enemy->body_id;
    draw_sprite(ENEMY_SPRITE, body_id->position, V2(enemy->facing, 1));
}

void enemy_update(Enemy *enemy, f32 delta)
{
    BodyID body_id = enemy->body_id;
    Vec2 *vel = &body_id->velocity;
    vel->y -= GRAVITY * delta;
    vel->x = ENEMY_MAX_SPEED * enemy->facing;
}
