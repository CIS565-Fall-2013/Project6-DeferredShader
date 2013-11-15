#version 330


uniform float u_Far;
uniform vec3 u_Ka;
uniform vec3 u_Kd;
uniform vec3 u_Ks;
uniform float u_specExp;

//1 if true, 0 if texture is missing.
uniform int u_hasDiffTex;
uniform int u_hasSpecTex;
uniform int u_hasBumpTex;
uniform int u_hasMaskTex;

///====Textures============
uniform sampler2D u_DiffTex;
uniform sampler2D u_SpecTex;
uniform sampler2D u_BumpTex;
uniform sampler2D u_MaskTex;
///========================

in vec3 fs_Normal;
in vec4 fs_Position;
in vec2 fs_Texcoord;

out vec4 out_Normal;
out vec4 out_Position;
out vec4 out_Color;

void main(void)
{
    out_Normal = vec4(normalize(fs_Normal),0.0f);
    out_Position = vec4(fs_Position.xyz,1.0f); //Tuck position into 0 1 range
	vec4 diffuseColor = vec4(texture(u_DiffTex,vec2(fs_Texcoord)).rgb,1.0);
	vec4 bumpColor = vec4(texture(u_BumpTex,vec2(fs_Texcoord)).rgb,1.0);
	
    out_Color = diffuseColor*vec4(u_Kd,1.0);
	out_Color += vec4(u_hasMaskTex,u_hasDiffTex,u_hasBumpTex,0.0);
	//out_Color = vec4(u_Color,1.0);
	//out_Color = vec4(fs_Texcoord.x, fs_Texcoord.y, 0.0, 1.0);
	//out_Color = 0.5*vec4(u_hasMaskTex, u_hasSpecTex, u_hasBumpTex, 1.0);
}
