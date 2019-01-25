
struct Player {
	Vec2 pos;
	Vec2 velocity;
	Body body;
	bool in_air;
	f32 shot_delay;
};

void player_update(Player *p, f32 delta) {
    //body.

}

void player_shoot(Player *p, s32 direction) {

}

void player_draw() {

}

void player_jump(Player *p) {
	if (!p->in_air) {

	}
}

