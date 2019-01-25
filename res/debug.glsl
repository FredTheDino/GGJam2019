// Camera
uniform vec2 camera_position;
uniform float camera_zoom;
uniform float aspect_ratio;
uniform float camera_rotation;


#ifdef VERT
in vec2 in_position;
in vec4 in_color;

out vec4 pass_color;

vec2 rotate(vec2 p, float angle)
{
	float s = sin(angle);
	float c = cos(angle);
	return vec2(p.x * c - p.y * s, p.x * s + p.y * c);
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
	vec2 projected_position = project(vec2(in_position.x, in_position.y));
	gl_Position = vec4(projected_position, 0.0, 1.0);
	pass_color = in_color;
}

#else

in vec4 pass_color;

out vec4 out_color;
void main()
{
	out_color = pass_color;
}

#endif



