#define ENEMY_ACCELERATION 10.0f
#define ENEMY_DEACCELERATION 40.0f
#define ENEMY_MAX_SPEED 5
#define ENEMY_START_HP 3

struct Enemy {
    BodyID body_id;
    u32 hp;
    s32 facing;
};


Enemy *create_enemy()
{
    Enemy *enemy = push_struct(Enemy);
    enemy->body_id = create_body(0xff, 1, 0);
    enemy->body_id->self = enemy;
    enemy->body_id->velocity.x = ENEMY_MAX_SPEED;
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
    Texture texture = find_asset(pixel).texture;
    BodyID body_id = enemy->body_id;
    draw_sprite(texture, body_id->position, body_id->scale, 0);
}

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
    }
    return 0;
}
