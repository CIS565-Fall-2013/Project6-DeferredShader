#version 330

uniform float u_Far;
uniform vec3 u_Color;
uniform vec3 u_Glow;

in vec3 fs_Normal;
in vec4 fs_Position;

out vec2 out_Normal;
out vec4 out_Position;
out vec3 out_Color;
out vec3 out_Glow;

void main(void)
{
	vec3 n = normalize(fs_Normal);
    out_Normal = vec2(n.x, n.y);
    out_Position = vec4(fs_Position.xyz,1.0f); //Tuck position into 0 1 range
    out_Color = vec3(u_Color);
	out_Glow = vec3(u_Glow);
}
