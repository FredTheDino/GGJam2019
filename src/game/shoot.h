#define SHOT_ALIVE_TIME 1
#define SHOT_SPEED 10
#define JELLO_INV_MASS 1.0f/128

enum ShotKind {
    JELLO,
    ONION,
    CARROT
};

struct Jello;

struct Shot {
    BodyID body_id;
    ShotKind shot_kind;
	f32 alive_time;
	bool is_destroyed;
	List<Jello*> *jello_list;
};

struct Player;
Shot *create_shot(Player *player, ShotKind shot_kind, s32 direction, List<Jello*> *jellos);

void destroy_shot(Shot *shot);

void shot_draw (Shot *shot);

void update_shots (List<Shot*> shots);

bool shot_on_collision(Body *self, Body *other, Overlap overlap);


