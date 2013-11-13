#version 330

uniform float u_Far;
uniform vec3 u_Color;
uniform float u_shininess;

in vec3 fs_Normal;
in vec4 fs_Position;

in vec4 PrevNDCPos;
in vec4 NDCPos;

out vec4 out_Normal;
out vec4 out_Position;
out vec4 out_Color;
out vec4 out_MV;

void main(void)
{
    out_Normal = vec4(normalize(fs_Normal),0.0f);
    out_Position = vec4(fs_Position.xyz,1.0f); //Tuck position into 0 1 range
    out_Color = vec4(u_Color, u_shininess );

	//get velocity
	vec4 pos = NDCPos/ NDCPos.w;
	pos = pos * 0.5 + 0.5;

	vec4 prevPos = PrevNDCPos / PrevNDCPos.w;
	prevPos = prevPos * 0.5 + 0.5;
	out_MV = ( pos - prevPos)/2.0;
}
