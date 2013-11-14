#version 330


uniform float u_Far;
uniform vec3 u_Color;

///====Textures=======
uniform sampler2D u_DiffTex;
///


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
	vec4 textureColor = vec4(texture(u_DiffTex,vec2(fs_Texcoord)).rgb,1.0);
    out_Color = textureColor*vec4(u_Color,1.0);
	//out_Color = textureColor;
	//out_Color = vec4(fs_Texcoord.x, fs_Texcoord.y, 0.0, 1.0);
}
