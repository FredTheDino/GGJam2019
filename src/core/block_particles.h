#ifndef __BLOCK_PARTICLES__
#define __BLOCK_PARTICLES__


struct Particle
{
	f32 time;
	f32 lifetime;

	f32 gravity;

	Vec2 scale;
	f32 rotation;
	Vec2 position;

	f32 angular_velocity;
	Vec2 linear_velocity;

	Vec4 from_color;
	Vec4 to_color;
};

// TODO:
// Potentially we want the distribution
// of the particles to be changeable, that
// would be quite cool and interesting.
//

struct ParticleSystem
{
#if 0
	f32 rate;
	f32 rate_error;
	
	f32 min_lifetime;
	f32 max_lifetime;

	f32 speed;
	f32 speed_error;

	f32 angle;
	f32 angle_error;

	Vec4 colorA;
	Vec4 colorB;
#endif

	Vec2 position;

	AssetID sprite; // This should be animateable.
	List<Particle> particles;
};

ParticleSystem create_particle_system(AssetID sprite)
{
	ParticleSystem system = {};
	system.sprite = sprite;
	system.particles = create_list<Particle>(10);
	return system;
}

void add_particle(ParticleSystem *system, Particle p)
{
	system->particles.append(p);
}

void update_particles(ParticleSystem *system, f32 delta)
{
	const Vec2 gravity = V2(0, -32);
	for (u32 i = 0; i < system->particles.length; i++)
	{
		Particle *p = system->particles + i;
		p->time += delta;
		if (p->time > p->lifetime)
		{
			system->particles.remove(i--);
			continue;
		}

		p->linear_velocity += gravity * p->gravity * delta;
		p->position += p->linear_velocity * delta;

		p->rotation += p->angular_velocity * delta;
	}
}

void draw_particles(ParticleSystem *system)
{
	Texture texture = (*system->sprite).texture;
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	for (u32 i = 0; i < system->particles.length; i++)
	{
		Particle p = system->particles[i];
		Vec4 color = lerp(p.to_color, p.time / p.lifetime, p.from_color);
		ASSERT(p.scale.x != 0 && p.scale.y != 0);
		draw_sprite(texture, p.position, p.scale, p.rotation, color);
	}
}


#endif
