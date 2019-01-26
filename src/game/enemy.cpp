#define ENEMY_ACCELERATION 10.0f
#define ENEMY_DEACCELERATION 40.0f
#define ENEMY_MAX_SPEED 5
#define ENEMY_START_HP 3

struct Enemy {
    BodyId body_id;
    u32 hp;
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

    return enemy;
}

void destroy_enemy(Enemy *enemy)
{
    destroy_body(enemy->body_id);
    pop_memory(enemy);
}
