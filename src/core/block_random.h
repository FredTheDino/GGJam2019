//
// *Really* minimal PCG32 code, stolen from the internet.
// http://www.pcg-random.org/download.html
//

//
// NOTE: This is only 32 bits of random numbers,
// it can be expanded to 64 bits of random.
//

#define MAX_RANDOM_INT (0xFFFFFFFF)

struct RandomState { 
	u64 state;  
	u64 inc; 
};

RandomState seed(u64 i)
{
	RandomState rng;
	rng.state = i * 87654291002987654ULL;
	rng.inc = i * 198765777234562234ULL;
	return rng;
}

u32 random(RandomState *rng)
{
    u64 oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + (rng->inc | 1);
    s32 xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    s32 rot = (oldstate >> 57u);
    return (xorshifted >> rot) ^ (xorshifted << ((-rot) & 31u));
}

f32 random_real(RandomState *rng)
{
	u32 n = random(rng);
	ASSERT(n < MAX_RANDOM_INT);
	f32 r = n / (f32) MAX_RANDOM_INT;
	return r;
}

s32 random_int_in_range(RandomState *rng, s32 min, s32 max)
{
	s32 range = max - min;
	s32 rand_range = random(rng) % range;
	return min + rand_range;
}

f32 random_real_in_range(RandomState *rng, f32 min, f32 max)
{
	f32 range = max - min;
	f32 rand_range = random_real(rng) * range;
	return min + rand_range;
}

Vec2 random_unit_vec2(RandomState *rng)
{
	f32 angle = random_real(rng) * 2 * PI;
	Vec2 result = {(f32) cos(angle), (f32) sin(angle)};
	return result;
}

Vec2 random_vec2(RandomState *rng, Vec2 min=V2(-1.0f, -1.0f), Vec2 max=V2(1.0f, 1.0f))
{
	Vec2 result;
	result.x = random_real_in_range(rng, min.x, max.x);
	result.y = random_real_in_range(rng, min.y, max.y);
	return result;
}

Vec3 random_unit_vec3(RandomState *rng)
{
	f32 alpha = random_real(rng) * 2.0f * PI;
	f32 beta  = random_real(rng) * 2.0f * PI;

	f32 sa = sin(alpha);
	f32 ca = cos(alpha);
	f32 sb = sin(beta);
	f32 cb = cos(beta);
	
	Vec3 result = {
		(f32) (sa * cb),
		(f32) (ca),
		(f32) (sa * sb)
	};
	return result;
}

Vec3 random_vec3(RandomState *rng, Vec3 min=V3(-1.0f, -1.0f, -1.0f), Vec3 max=V3(1.0f, 1.0f, 1.0f))
{
	Vec3 result;
	result.x = random_real_in_range(rng, min.x, max.x);
	result.y = random_real_in_range(rng, min.y, max.y);
	result.z = random_real_in_range(rng, min.z, max.z);
	return result;
}

