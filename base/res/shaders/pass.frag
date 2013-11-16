#version 330

uniform float u_Far;
uniform vec3 u_Color;
uniform float u_Glow;
uniform float u_Shininess;

in vec3 fs_Normal;
in vec4 fs_Position;

out vec4 out_Normal;
out vec4 out_Position;
out vec4 out_Color;

void main(void)
{
    out_Normal = vec4(normalize(fs_Normal),0.0f);
	// shininess can fit in 31 bits, glow just really needs 1 bit
    out_Position = vec4(fs_Position.xyz,(u_Shininess * 2 + u_Glow)); //Tuck position into 0 1 range
	// Potentially use alpha channel for something useful like emmission light or not! : for Glow shading
    out_Color = vec4(u_Color, 1.0);
}
