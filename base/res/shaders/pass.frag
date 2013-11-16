#version 330

uniform float u_Far;
uniform vec3 u_Color;
uniform float u_shininess;

in vec3 fs_Normal;
in vec4 fs_Position;

in vec4 PrevNDCPos;
in vec4 NDCPos;

out vec4 out_Normal;
//out vec4 out_Position;
out vec4 out_Color;
//out vec4 out_MV;

void main(void)
{
    //out_Position = vec4(fs_Position.xyz,1.0f); //Tuck position into 0 1 range
	//get velocity
	vec4 pos;
	vec4 prevPos;
    vec3 N = normalize(fs_Normal.xyz);
	vec4 mv;
    pos= NDCPos/ NDCPos.w;
	pos= pos * 0.5 + 0.5; //position of this frame at screen space
	
	prevPos = PrevNDCPos / PrevNDCPos.w;
	prevPos = prevPos * 0.5 + 0.5;  //position of last frame at screen space
	mv = ( pos - prevPos)/2.0; //velocity

	//out_Normal.zw = ( pos - prevPos ).xy / 2.0;
	//out_Normal.xy = normalize(N.xy)*sqrt(N.z*0.5+0.5);
	out_Normal = vec4( N, mv.x );
	//out_Position = fs_Position;
	out_Color = vec4(u_Color, mv.y );
}
