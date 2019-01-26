// Transform
uniform vec2 position;
uniform vec2 scale;
uniform float rotation;

// Camera
uniform vec2 camera_position;
uniform float camera_zoom;
uniform float aspect_ratio;
uniform float camera_rotation;

uniform sampler2D sprite_texture;

// Misc.
uniform int tilemap;
uniform vec4 tint;
uniform float time;

uniform vec2 min_uv;
uniform vec2 dim_uv;

#ifdef VERT
in vec2 in_position;
in vec2 in_uv;
out vec2 uv;

vec2 rotate(vec2 p, float angle)
{
	float s = sin(angle);
	float c = cos(angle);
	return vec2(p.x * c - p.y * s, p.x * s + p.y * c);
}

vec2 transform(vec2 p)
{
	vec2 result = p;
	result = vec2(result.x * scale.x, result.y * scale.y);
	result = rotate(result, -rotation);
	result = result + position;
	return result;
}

vec2 project(vec2 p)
{
	vec2 result = p;
	result = result - camera_position;
	result = rotate(result, camera_rotation);
	result = vec2(result.x * aspect_ratio * camera_zoom, result.y * camera_zoom);
	return result;
}

void main()
{
	if (tilemap == 1)
	{
		uv = in_uv;
	}
	else
	{
		uv = vec2(in_position.x, in_position.y) * 0.5 + vec2(0.5, 0.5);
		uv = min_uv + vec2(dim_uv.x * uv.x, dim_uv.y * uv.y);
	}
	vec2 world_position = transform(in_position);
	vec2 projected_position = project(vec2(world_position.x, world_position.y));
	gl_Position = vec4(projected_position.x, projected_position.y, 0.0, 1.0);
}

#else

in vec2 uv;

out vec4 out_color;
void main()
{
	if (tilemap == 1)
	{
		out_color = texture(sprite_texture, uv).xyzw * tint;
	}
	else
	{
		out_color = vec4(texture(sprite_texture, uv).xyz, 1.0) * tint;
	}
}

#endif

