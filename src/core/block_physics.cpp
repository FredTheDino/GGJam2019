#include "block_physics.h"

PhysicsWorld initalize_world()
{
	PhysicsWorld world = {};
	world.shapes = create_list<Shape>(32);
	world.bodies = create_list<Body>(32);
	world.limits = create_list<Limit>(32);
	world.overlaps = create_list<Overlap>(32);
	world.overlaps_prev = create_list<Overlap>(32);
    world.timestep = 1.0f / 120.0f;

	world.uid_counter = 32;
	
	world.sorting_axis = V2(1.0, 0.0); 

	// The sorting axis can be changed if you
	// feel the engine is doing unnessecary
	// collision checks.
	return world;
}

void clear_world(PhysicsWorld *world)
{
	world->shapes.clear();
	world->bodies.clear();
	world->limits.clear();
	world->overlaps.clear();
	world->uid_counter *= 1335;
}

void destroy_world(PhysicsWorld *world)
{
	for (u64 i = 0; i <= world->shapes_highest; i++)
	{
		Shape s = world->shapes[i];
		if ((u32) s.id.pos != i) continue;
		destroy_list(&s.normals);
		destroy_list(&s.points);
	}
	destroy_list(&world->shapes);
	destroy_list(&world->bodies);
	destroy_list(&world->limits);
	destroy_list(&world->overlaps);
}

ShapeID new_shape_id(PhysicsWorld *world)
{
	ShapeID id = {};
	id.uid = (s32) world->uid_counter++;
	if (world->shapes_next_free < 0)
	{
		// Use an old.
		id.pos = -(world->shapes_next_free + 1);
		world->shapes_next_free = world->shapes[id.pos].id.next_free;
	}
	else
	{
		// Add a new.
		id.pos = world->shapes_next_free++;
		if (world->shapes_next_free >= (s32) world->shapes.capacity)
		{
			world->shapes.resize(world->shapes.capacity * 2);
		}
	}

	ASSERT(id.pos >= 0);
	world->shapes_highest = maximum((s32) world->shapes_highest, id.pos);
	return id;
}
ShapeID new_shape_id() 
{ 
	return new_shape_id(game.world); 
}

// I doubt you need to free them. The funcionality will be added if needed.
ShapeID register_shape(PhysicsWorld *world, u32 points_length, Vec2 *points)
{
	// THey need volume.
	ASSERT(points_length != 0);
	Shape shape = {};
	shape.points = create_list<Vec2>(points_length);

	// We need the center to lay inside the shapes
	// for the SAT algorithm to work. So this is a 
	// nessecary evil I'm afraid.
	Vec2 center = V2(0, 0);
	for (u32 i = 0; i < points_length; i++)
		center = center + points[i];
	center /= (real) points_length;
	shape.center = center;

	for (u32 i = 0; i < points_length; i++)
		shape.points[i] = points[i] - center;
	shape.points.length = points_length; // Change the length to match.
	// Allocate worstcase memory. 
	// This is a tad wastefull but it shouldn't be much
	// since most shapes should be fairly simple in their 
	// design, and they are reusable so it should be even
	// less important.
	shape.normals = create_list<Vec2>(points_length);
	for (u32 i = 0; i < points_length; i++)
	{
		Vec2 v = shape.points[i];
		Vec2 u = shape.points[(i + 1) % shape.points.length];

		Vec2 relative = normalize(v - u);
		Vec2 normal = {-relative.y, relative.x};

		// If they share the same axis with something else.
		// Sure it's slow to check now, but it does save time
		// in the long run since it can cut the checks in half
		// on a symetric shape. It's cool.
		for (u32 j = 0; j < shape.normals.length; j++)
		{
			if (absolute(dot(normal, shape.normals[j])) >= 0.98f)
			{
				goto _CONTINUE_OUTER_REG_SHAPE;
			}
		}

		shape.normals.append(normal);
_CONTINUE_OUTER_REG_SHAPE: continue;
	}

	// TODO Remove
	shape.id = new_shape_id();
	world->shapes[shape.id.pos] = shape;
	return shape.id;
}

ShapeID register_shape(PhysicsWorld *world, List<Vec2> points)
{
	return register_shape(world, points.length, points.data);
}

Shape find_shape(PhysicsWorld *world, ShapeID id)
{
	ASSERT(id.pos >= 0);
	ASSERT(id.pos <= (s32) world->shapes_highest);
	Shape shape = world->shapes[id.pos];
	ASSERT(id.uid == shape.id.uid) // Assumes it suceeds.
	return shape;
}

BodyID new_body_id(PhysicsWorld *world)
{
	BodyID id = {};
	id.uid = (s32) world->uid_counter++;
	if (world->bodies_next_free < 0)
	{
		// Use an old.
		id.pos = -(world->bodies_next_free + 1);
		world->bodies_next_free = world->bodies[id.pos].id.next_free;
	}
	else
	{
		// Add a new.
		id.pos = world->bodies_next_free++;
		if (world->bodies_next_free >= (s32) world->bodies.capacity)
		{
			world->bodies.resize(world->bodies.capacity * 2);
		}
	}
	ASSERT(id.pos >= 0);
	world->bodies_highest = maximum((s32) world->bodies_highest, id.pos);
	print("new body %d:%d\n", id.pos, id.uid);
	return id;
}
BodyID new_body_id()
{
	return new_body_id(game.world);
}

void center(PhysicsWorld *world, Body *body)
{
	body->offset = find_shape(world, body->shape).center;
	body->offset = hadamard(body->offset, body->scale);
}

void center(PhysicsWorld *world, BodyID id)
{
	center(world, find_body_ptr(world, id));
}

Limit project_shape(Shape shape, Vec2 axis, 
		Vec2 scale=V2(1, 1), Vec2 offset=V2(0, 0))
{
	Limit limit = {};
	f32 delta = dot(offset, axis);
	for (u32 i = 0; i < shape.points.length; i++)
	{
		f32 projection = dot(hadamard(shape.points.data[i], scale), axis);
		limit.upper = maximum(limit.upper, projection + delta);
		limit.lower = minimum(limit.lower, projection + delta);
	}
	return limit;
}

Limit project_body(PhysicsWorld *world, Body body, Vec2 axis)
{
	f32 projected = dot(body.position, axis);
	axis = rotate(axis, -body.rotation);
	Limit limit = project_shape(world->shapes[body.shape.pos], axis, body.scale, 
			body.offset);
	limit.layer = body.layer;
	limit.owner = body.id;
	limit.lower += projected;
	limit.upper += projected;
	return limit;
}

Limit project_body(PhysicsWorld *world, BodyID id, Vec2 axis)
{
	Body *body = find_body_ptr(world, id);
	if (body)
		return project_body(world, *body, axis);
	return {};
}

#if 0
void bind_body(PhysicsWorld *world, Vec2 *pos, BodyID body_id)
{
	find_body_ptr(body_id)->owner = pos;
}
#endif

BodyID create_body(PhysicsWorld *world, u32 layer, 
		f32 mass, f32 bounce, f32 damping, bool trigger)
{
	Body body = {};
	body.scale = V2(1, 1);
	body.id = new_body_id();
	body.damping = damping;
	body.shape = _default_rect;
	body.trigger = trigger;
	body.bounce = bounce;
	body.layer = layer;
	if (mass == 0.0)
		body.inverse_mass = 0.0f;
	else
		body.inverse_mass = 1.0f / mass;

	ASSERT(body.id.pos >= 0);
	world->bodies_highest = maximum(world->bodies_highest, (u32) body.id.pos + 1);

	// TODO: Insert it into at a good place. This is kinda dumb.
	Limit limit = project_body(world, body, world->sorting_axis);
	limit.is_new = true;
	world->limits.append(limit);

	world->bodies[body.id.pos] = body;
	return body.id;
}

inline
bool is_alive(Body e)
{
	return e.id.uid >= 0;
}

inline
bool is_alive(Body *e)
{
	return e->id.uid >= 0;
}

void recalculate_highest(PhysicsWorld *world)
{
	do
	{
		if (is_alive(world->bodies[world->bodies_highest]))
			break;
		world->bodies_highest--;
	} while (world->bodies_highest);
}

void destroy_body(PhysicsWorld *world, BodyID id)
{
	Body *body = find_body_ptr(world, id);
	if (!body) return;

	s32 tmp = world->bodies_next_free;
	world->bodies_next_free = -(id.pos + 1);
	body->id.next_free = tmp;
	body->id.uid = -body->id.uid;
	
	recalculate_highest(world);
}

Body *find_body_ptr(PhysicsWorld *world, BodyID id)
{
	ASSERT(id.pos >= 0);
	// ASSERT(id.pos <= (s32) world->bodies_highest);
	Body *body = world->bodies + id.pos;
	if (id.uid != body->id.uid)
		return {};
	return body;
}

Body find_body(PhysicsWorld *world, BodyID id)
{
	ASSERT(id.pos >= 0);
	// ASSERT(id.pos <= (s32) world->bodies_highest);
	Body body = world->bodies[id.pos];
	if (id.uid != body.id.uid)
		return {};
	return body;
}

void debug_draw_body(PhysicsWorld *world, Body body)
{
	Shape shape = find_shape(world, body.shape);
	for (u32 i = 0; i < shape.points.length; i++)
	{
		u32 v_i = i;
		u32 u_i = (i + 1) % shape.points.length;
		Vec2 v = rotate(
				hadamard(shape.points[v_i], body.scale) + body.offset, 
				body.rotation);
		v += body.position;
		Vec2 u = rotate(
				hadamard(shape.points[u_i], body.scale) + body.offset, 
				body.rotation);
		u += body.position;

		debug_point(v, V4(0.1f, 0.2f, 0.7f, 1.0f));
		debug_line(u, v, V4(0.7f, 0.2f, 0.1f, 1.0f));
	}

	Vec2 middle = body.position + body.offset;
	for (u32 i = 0; i < shape.normals.length; i++)
	{
		Vec2 normal = normalize(rotate(hadamard(shape.normals[i], inverse(body.scale)), body.rotation)) * 0.5;
		debug_line(middle - normal, middle + normal, V4(0.2f, 0.7f, 0.1f, 1.0f));
	}
}

void debug_draw_body(PhysicsWorld *world, BodyID id)
{
	Body body = find_body(world, id);
	if (body.id.uid != id.uid) return;
	debug_draw_body(world, body);
}

void debug_draw_world(PhysicsWorld *world)
{
	Vec2 normal = world->sorting_axis;
	Vec2 tangent = {-normal.y, normal.x};
	for (u32 i = 0; i < world->limits.length; i++)
	{
		Limit l = world->limits[i];
		debug_line(
				normal * l.lower + tangent * (real) i * 0.1f,
				normal * l.upper + tangent * (real) i * 0.1f,
				V4(0.4f, 0.2f, 0.7f, 1.0f));
		debug_draw_body(world, l.owner);
	}
}

void integrate(Body *body, f32 delta)
{
	body->acceleration += body->force * body->inverse_mass;
	body->velocity += body->acceleration * delta;
	body->position += body->velocity * delta;

#if 1
	f32 damping = clamp(0.0f, 1.0f, 1 - body->damping);
	if (damping != 0.0f)
		body->velocity = body->velocity * pow(damping, delta);
#endif
}

Overlap check_bodies(PhysicsWorld *world, Body *body_a, Body *body_b, f32 delta)
{
	// NOTE: If I find that dragging along a surface is jagged,
	// we could try having weighted directions and add a little bit 
	// of weight to the ones along the closing velocity. It might
	// help. I don't know.

	Shape shape_a = find_shape(world, body_a->shape);
	Shape shape_b = find_shape(world, body_b->shape);

	Overlap overlap = {body_a->id, body_b->id, -1.0f};

	Vec2 center = (body_a->position + body_b->position) * 0.5;

	Vec2 relative_position = (body_b->position) - (body_a->position);

	Vec2 scale = inverse(body_a->scale);
	List<Vec2> normals = shape_a.normals;
	for (u32 n = 0; n < 2; n++)
	{
		for (u32 i = 0; i < normals.length; i++)
		{
			Vec2 normal, axis_a, axis_b;
			normal = normals[i];
			normal = normalize(hadamard(normal, scale));

			if (n == 0)
			{
				f32 rotation = body_a->rotation - body_b->rotation;
				axis_a = normal;
				axis_b = rotate(normal, rotation);
				normal = rotate(normal, body_a->rotation);
				if (value("show"))
				{
					debug_line(body_a->position, body_a->position + normal);
				}
			}
			else
			{
				f32 rotation = body_b->rotation - body_a->rotation;
				axis_a = rotate(normal, rotation);
				axis_b = normal;
				normal = rotate(normal, body_b->rotation);
				if (value("show"))
				{
					debug_line(body_b->position, body_b->position + normal);
				}
			}

#if 0
			Vec2 axis_a, axis_b;
			axis_a = normal;
			axis_b = normal;
#endif

			Limit limit_a, limit_b;
			limit_a = project_shape(shape_a, axis_a, body_a->scale, body_a->offset);
			limit_b = project_shape(shape_b, axis_b, body_b->scale, body_b->offset);

			if (value("show"))
			{
				Vec2 a1, a2;
				a1 = body_a->position + normal * limit_a.lower;
				a2 = body_a->position + normal * limit_a.upper;
				debug_line(a1, a2, V4(1, 0, 1, 1));

				Vec2 b1, b2;
				b1 = body_b->position + normal * limit_b.lower;
				b2 = body_b->position + normal * limit_b.upper;
				debug_line(b1, b2, V4(0, 1, 1, 1));
			}
			f32 projected_distance = dot(relative_position, normal);


			f32 depth;
	 		if (projected_distance > 0)
				depth = limit_a.upper - limit_b.lower - projected_distance;
			else
				depth = limit_b.upper - limit_a.lower + projected_distance;

			if (value("show"))
			{
				//debug_line(body_a->position, body_a->position + axis_a);
				//debug_line(body_b->position, body_b->position + axis_b);
				Vec4 color = depth > 0 ? V4(1, 1, 0, 1) : V4(1, 0, 0, 1);
				debug_line(center, center + normal * depth, color);
			}

			if (depth < 0)
				return overlap;

			if (depth < overlap.depth || overlap.depth == -1.0f)
			{
				overlap.depth = depth;
				overlap.normal = normal;
			}
		}

		normals = shape_b.normals;
		scale = inverse(body_b->scale);
	}

	if (value("show"))
		debug_line(center, center + overlap.normal * overlap.depth, V4(0, 1, 0, 1));

	if (dot(overlap.normal, relative_position) < 0)
	{
		// Make sure the normal faces body a
		overlap.normal = -overlap.normal;
	}



	overlap.is_valid = true;
	return overlap;
}

void solve(PhysicsWorld *world, Overlap overlap, f32 delta)
{
	Body *a, *b;
	a = find_body_ptr(world, overlap.a);
	b = find_body_ptr(world, overlap.b);

	if (a->trigger || b->trigger)
		return;

	f32 total_mass = 1.0f / (a->inverse_mass + b->inverse_mass);
	
	// Position
	Vec2 relative_position = (a->position + a->offset) - (b->position + b->offset);
	Vec2 position_direction = normalize(relative_position);
	Vec2 normal = -overlap.normal * (f32) sign(dot(relative_position, overlap.normal));
	//f32 distance = dot(relative_velocity, relative_position);
#if 1
	a->position -= overlap.normal * overlap.depth * total_mass * a->inverse_mass;
	b->position += overlap.normal * overlap.depth * total_mass * b->inverse_mass;
#endif

	// Don't do velocity correct unless you have to.
	Vec2 relative_velocity = a->velocity - b->velocity;
	f32 normal_velocity = dot(normal, relative_velocity);
	if (normal_velocity < 0)
	{
		print("Nope!\n");
		return;
	}

	// Normal
	f32 bounce = maximum(a->bounce, b->bounce);
	f32 normal_force = normal_velocity * total_mass * (1.0f + bounce);
	a->velocity -= normal * normal_force * a->inverse_mass;
	b->velocity += normal * normal_force * b->inverse_mass;

#if 0
	// Friction
	Vec2 tangent = {-overlap.normal.y, overlap.normal.x};
	f32 tangent_velocity = dot(tangent, relative_velocity);
	f32 mu = (a->friction * b->friction);
	f32 friction = normal_force * mu * delta;

	if (absolute(tangent_velocity) < absolute(friction / total_mass))
	{
		a->velocity -= tangent * tangent_velocity * a->inverse_mass;
		b->velocity -= tangent * tangent_velocity * b->inverse_mass;
	}
	else
	{
		a->velocity -= tangent * friction * a->inverse_mass * 
			sign(tangent_velocity);
		b->velocity -= tangent * friction * b->inverse_mass *
			-sign(tangent_velocity);
	}
#endif
}

void update_world(PhysicsWorld *world, f32 delta)
{
	// Fixed timestep
	bool did_update = false;
	world->timer += delta;
	world->overlaps.clear();
	while (world->timer > world->timestep)
	{
		did_update = true;
		f32 delta = world->timestep;
		world->timer -= world->timestep;

		for (u32 i = 0; i < world->limits.length; i++)
		{
			Limit *limit = world->limits + i;
			Body *body = find_body_ptr(world, limit->owner);
			if (!body)
			{
				world->limits.remove(i);
				i--;
				recalculate_highest(world);
				continue;
			}
			//ASSERT(body);

#if 0
			Vec2 *owner = find_entity_ptr(body->owner).owner;
			if (owner)
			{
				body->position = owner;
				//body->rotation = entity->rotation;
			}
#endif

			integrate(body, delta);
			*limit = project_body(world, *body, world->sorting_axis);
		}

		// Sort the list
		for (u32 i = 1; i < world->limits.length; i++)
		{
			for (u32 j = i; 0 < j; j--)
			{
				Limit a = world->limits[j - 0];
				Limit b = world->limits[j - 1];
				if (b.lower < a.lower)
				{
					break;
				}
				world->limits[j - 0] = b;
				world->limits[j - 1] = a;
			}
		}

		// Collision detection and resolution.
		for (u32 i = 0; i < world->limits.length; i++)
		{
			Limit outer = world->limits[i];
			for (u32 j = i + 1; j < world->limits.length; j++)
			{
				Limit inner = world->limits[j];
				if ((outer.layer & inner.layer) == 0)
					break;
				if (outer.upper < inner.lower)
					break;
				if (outer.is_new || inner.is_new)
					break;
				Body *a = find_body_ptr(world, outer.owner);
				Body *b = find_body_ptr(world, inner.owner);

				if (!a) break;
				if (!b) continue;

				if (a->trigger == true && b->trigger == true)
					continue;
				if (a->inverse_mass == 0 && b->inverse_mass == 0)
					continue;

				Overlap overlap = check_bodies(world, a, b, delta);
				if (!overlap.is_valid)
					continue; // Not a collision
				world->overlaps.append(overlap);
				if (a->overlap)
					if (a->overlap(a, b, overlap))
						continue;
				if (b->overlap)
					if (b->overlap(b, a, {b->id, a->id, overlap.depth, -overlap.normal, true})) // No need to get a bool that we know the value of ;)
						continue;
				solve(world, overlap, delta);
			}
		}
        u32 prev_size = world->overlaps_prev.length;
        u32 cur_size = world->overlaps.length;
        bool found;
        for(u32 i = 0; i < prev_size; ++i){
            Overlap overlap = world->overlaps_prev[i];
            if(overlap.a->overlapnt || overlap.b->overlapnt){
                Body *a = find_body_ptr(overlap.a);
                Body *b = find_body_ptr(overlap.b);
                found = false;
                for(u32 j = 0; j < cur_size; ++j){
                    bool regular_exists = a->id == world->overlaps[j].a->id && b->id == world->overlaps[j].b->id;
                    bool reversed_exists = a->id == world->overlaps[j].b->id && b->id == world->overlaps[j].a->id;
                    if(regular_exists || reversed_exists){
                        found = true;
                        break;
                    }
                }
                if(found) continue; // No overlap change
                if(a->overlapnt)
                    a->overlapnt(a, b, overlap);
                if(b->overlapnt)
                    b->overlapnt(b, a, {b->id, a->id, overlap.depth, -overlap.normal, true});
            }
        }

        {
            List<Overlap> otherlaps = world->overlaps;
            world->overlaps = world->overlaps_prev;
            world->overlaps_prev = otherlaps;
        }


		for (u32 i = 0; i < world->bodies_highest; i++)
		{
			Body body = world->bodies[i];
			if (body.id.pos != (s32) i) continue;
#if 0
			Entity *entity = find_entity_ptr(body.owner);
			if (entity)
			{
				entity->position = body.position;
				entity->rotation = body.rotation;
			}
#endif
		}
	}

	if (did_update)
	{
		for (u32 i = 0; i < world->limits.length; i++)
		{
			Limit *limit = world->limits + i;
			Body *body = find_body_ptr(world, limit->owner);
			if (!body) continue;
			body->acceleration = {};
			body->force = {};
		}
	}
}


