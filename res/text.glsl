// Transform
uniform vec2 position;
uniform vec2 scale;
uniform float rotation;

uniform vec4 color;
uniform float width = 0.50;
uniform float edge  = 0.15;

uniform sampler2D font_atlas;

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
	result = rotate(result, rotation);
	result = result + position;
	return vec2(result.x, -result.y);
}

void main()
{
	vec2 world_position = transform(vec2(in_position.x, in_position.y));
	gl_Position = vec4(world_position, 0.0, 1.0);
	uv = in_uv;
}

#else

in vec2 uv;

out vec4 out_color;
void main()
{
	if (width == 0.0)
	{
		out_color = color;
	}
	else
	{
		float distance = 1.0 - texture(font_atlas, uv).a;
		float alpha = 1.0 - smoothstep(width, width + edge, distance);
		out_color = vec4(color.rgb, color.a * alpha);
	}
}

#endif

