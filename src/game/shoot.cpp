enum ShotKind {
    JELLO,
    CARROT,
    ONION
};

struct Shot {
    BodyID body_id;
    ShotKind shot_kind;
};

Shot *create_shot(ShotKind shot_kind, f32 xvel)
{
    BodyID body_id = create_body(0xFF, 1);
    body_id->velocity.x = xvel;
    Shot *shot = push_struct(Shot);
    shot->body_id = body_id;
    shot->shot_kind = shot_kind;
    return shot;
}

void destroy_shot(Shot *shot)
{
    destroy_body(shot->body_id);
    pop_memory(shot);
}

bool shot_on_collision(Body *self, Body *other, Overlap overlap)
{
    Shot *shot = (Shot *) self->self;
    switch (shot->shot_kind) {
    case JELLO:
        break;

    case CARROT:
        break;

    case ONION:
        break;
    }
    return true;
}
