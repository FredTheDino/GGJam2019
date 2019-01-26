#define SHOT_ALIVE_TIME 1
#define SHOT_SPEED 10

enum ShotKind {
    JELLO,
    CARROT,
    ONION
};

struct Shot {
    BodyID body_id;
    ShotKind shot_kind;
	f32 alive_time;
};

struct Player;
Shot *create_shot(Player *player, ShotKind shot_kind, s32 direction);

void destroy_shot(Shot *shot);

void shot_draw (Shot *shot);

void update_shots (List<Shot*> shots);

bool shot_on_collision(Body *self, Body *other, Overlap overlap);


