#ifndef __BLOCK_MATH__
#define __BLOCK_MATH__

// What do we need?
// Can we get rid of math.h?
//  sin/cos 
//  sqrt
//  tan?
//  pow?
//

#include <math.h>
#define PI 3.1419f

#ifndef NAN
#error "NAN does not seem to be supported by this compiler."
#endif

// Custom Math functions, since I don't to trust the 
// stdlib ones since they may vary between compilers. 
// So I did the work instead and added in the little 
// things I need. We'll see 
template <typename T>
T minimum(T a, T b)
{
	return (a < b) ? a : b;
}

template <typename T>
T maximum(T a, T b)
{
	return (a > b) ? a : b;
}

template <typename T>
T lerp(T a, f32 l, T b)
{
	return a * l + b * (1.0 - l);
}

template <typename T>
T clamp(T min, T max, T v)
{
	return maximum(min, minimum(max, v));
}

template <typename T>
s32 sign_no_zero(T n)
{
	if (n < 0)
		return -1;
	return 1;
}

template <typename T>
s32 sign(T n)
{
	if (n < 0)
		return -1;
	if (n > 0)
		return 1;
	return 0;
}

template <typename T>
T in_range(T low, T high, T value)
{
	return low < value && value < high;
}

template <typename T>
T absolute(T n)
{
	if (n < 0)
		return -n;
	return n;
}


// This is used for all vectors and matricies and 
// quaternions in the engine. So you can set the 
// presicion yourself.
typedef f32 real;
#include "block_vector.h"
#include "block_quaternion.h"
#include "block_matrix.h"
#include "block_transform.h"

struct Rect
{
	union
	{
		struct
		{
			Vec2 min;
			Vec2 dim;
		};
		struct
		{
			real x, y, w, h;
		};
	};
};

Rect R(f32 x, f32 y, f32 w, f32 h)
{
	return { V2(x, y), V2(w, h) };
}

bool point_in(Vec2 point, Vec2 position, Vec2 dimension)
{
	return 
		point.x > position.x - dimension.x &&
		point.x < position.x + dimension.x &&
		point.y > position.y - dimension.y &&
		point.y < position.y + dimension.y;
}

#endif
