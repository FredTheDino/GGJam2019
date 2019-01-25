#ifndef __BLOCK_PHYSICS__
#define __BLOCK_PHYSICS__

typedef u32 Layer;

ShapeID _default_rect;
struct Shape
{
	ShapeID id;
	Vec2 center;
	List<Vec2> normals;
	List<Vec2> points;
};

struct Overlap
{
	BodyID a, b;
	f32 depth;
	Vec2 normal; // Allways points towards a.
	bool is_valid;
};

struct Body;
typedef bool (*PhysicsCallback)(Body *self, Body *other, Overlap overlap);

struct Body
{
	BodyID id;
	EntityID owner;
	ShapeID shape;
	Layer layer;

	Vec2 offset;

	Vec2 scale;
	// This is just for static rotation. So the 
	// bodies can be oriented. No rotational forces
	// are applied.
	f32 rotation; // Maybe rename this to "angle".
	Vec2 position;
	Vec2 velocity;
	Vec2 acceleration;
	Vec2 force;

	f32 inverse_mass;
	f32 damping;
	f32 bounce;
	f32 drag; // TODO: Implement this, it's cool.
	//f32 friction; // TODO: Figure out if this is needed at all.

	bool trigger;
	PhysicsCallback overlap;
  void *self;
};


struct Limit
{
	BodyID owner;
	Layer layer;
	f32 lower, upper;
};

struct PhysicsWorld
{
	u32 shapes_highest;
	s32 shapes_next_free;
	List<Shape> shapes;

	u32 bodies_highest;
	s32 bodies_next_free;
	List<Body> bodies;

	Vec2 *owner;

	List<Limit> limits;
	
	Vec2 sorting_axis;

	u16 uid_counter; // So it won't overflow.

	List<Overlap> overlaps;
	
	f32 timer;
	f32 timestep;

}; // Global state struct, might be a dumb idea.

// 
// World
//

// Set up everything needed for physics simulation.
PhysicsWorld initalize_world();

// Reset the world to it's initalized state
void clear_world(PhysicsWorld *world);

// Clean up after the world.
void destroy_world(PhysicsWorld *world);

// Let the physics engine take a step.
void update_world(PhysicsWorld *world, f32 delta);
void update_world(f32 delta)
{
	update_world(game.world, delta);
}

// 
// Shape
//

// Tell the physics engine about a new shape that can be used.
ShapeID register_shape(PhysicsWorld *world, u32 points_length, Vec2 *points);
ShapeID register_shape(PhysicsWorld *world, List<Vec2> points);
ShapeID register_shape(u32 points_length, Vec2 *points) 
{ 
	return register_shape(game.world, points_length, points); 
}
ShapeID register_shape(List<Vec2> points)
{
	return register_shape(game.world, points);
}

// Make sure the body is centerd to where the shape thinks the center is.
void center(PhysicsWorld *world, BodyID id);
void center(BodyID id)
{
	center(game.world, id);
}

//
// Body
//

// Create a new body that can be simulated.
BodyID create_body(PhysicsWorld *world, u32 layer, f32 mass, 
		f32 bounce=0.1f, f32 damping=0.1f, bool trigger=false);
BodyID create_body(u32 layer, f32 mass, 
		f32 bounce=0.1f, f32 damping=0.1f, bool trigger=false)
{
	return create_body(game.world, layer, mass, bounce, damping, trigger);
}

// Free a bodies resources.
void destroy_body(PhysicsWorld *world, BodyID id);
void destroy_body(BodyID id)
{
	destroy_body(game.world, id);
}

// Find the body pointed to by the handle.
Body *find_body_ptr(PhysicsWorld *world, BodyID id);
Body *find_body_ptr(BodyID id)
{
	return find_body_ptr(game.world, id);
}

// Bind the body and the enitity to use the same position.
#if 0
void bind_body(PhysicsWorld *world, Vec2 *owner, BodyID body_id);
void bind_body(EntityID entity_id, BodyID body_id)
{
	return bind_body(game.world, entity_id, body_id);
}
#endif

// 
// Debug
//

// Draw all the bodies in the engine.
void debug_draw_world(PhysicsWorld *world);
void debug_draw_world()
{
	debug_draw_world(game.world);
}

Body *BodyID::operator-> ()
{
	return find_body_ptr(*this);
}

BodyID::operator bool()
{
	return find_body_ptr(*this) != 0;
}

#endif
