#define JELLO_ALIVE_TIME 10

struct Jello {
	BodyID body_id;
	f32 alive_time;
};

Jello *create_jello(Shot *shot);

void destroy_jello(Jello *jello);

void update_jellos(List<Jello*> *jellos, f32 delta);

void jellos_draw (List<Jello*> *jellos); 
