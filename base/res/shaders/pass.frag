#version 330

uniform float u_Far;
uniform vec3 u_Color;
uniform vec3 u_SpecularColor;

in vec3 fs_Normal;
in vec4 fs_Position;

out vec4 out_Normal;
out vec4 out_Position;
out vec4 out_Color;
out vec4 out_SpecularColor;

void main(void)
{
	vec3 N_unit = normalize(fs_Normal);
    out_Normal = vec4(N_unit,0.0f);
    out_Position = vec4(fs_Position.xyz,1.0f); //Tuck position into 0 1 range
    out_Color = vec4(u_Color, N_unit.x);
	out_SpecularColor = vec4(u_SpecularColor, N_unit.y);
}
