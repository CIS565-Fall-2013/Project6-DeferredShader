#version 330
#extension GL_EXT_gpu_shader4: enable

////////////////////////////
//       ENUMERATIONS
////////////////////////////

#define	NO_CHANGE  				0
#define	HASTEX_OVERLAY 			1
#define	TEXCOORDS_AS_DIFFUSE 	2
#define	BUMP_AS_DIFFUSE 	 	3
#define	MASK_OVERLAY 	 		4


uniform float u_Far;
uniform vec3 u_Ka;
uniform vec3 u_Kd;
uniform vec3 u_Ks;
uniform float u_specExp;

uniform int u_PassthroughMode;

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
out vec4 out_Diff_Color;
out vec4 out_Spec_Color;

void main(void)
{
    out_Normal = vec4(normalize(fs_Normal),0.0f);
    out_Position = vec4(fs_Position.xyz,1.0f); //Tuck position into 0 1 range
	
	if(u_hasMaskTex > 0)
	{
		//Only discard samples if not in overlay mode
		if(texture(u_MaskTex,vec2(fs_Texcoord)).x < 1)
			discard;
			
	}
	
	
	vec4 diffuseColor = vec4(u_Kd,1.0);
	if(u_hasDiffTex > 0)
	{
		diffuseColor *=vec4(texture(u_DiffTex,vec2(fs_Texcoord)).rgb,1.0);
	}
	
	if(u_hasBumpTex > 0)
	{
		//TODO: Do Bump Mapping Here
		ivec2 size = textureSize2D(u_BumpTex,0);
		
	}	
	
	//Pass through diffuse color
	out_Diff_Color = diffuseColor*vec4(u_Kd,1.0);
	 
	if(u_PassthroughMode == HASTEX_OVERLAY){
		out_Diff_Color += 0.9*vec4(u_hasMaskTex,u_hasDiffTex,u_hasBumpTex,0.0);
	}else if(u_PassthroughMode == TEXCOORDS_AS_DIFFUSE){
		out_Diff_Color = vec4(fs_Texcoord.x, fs_Texcoord.y, 0.0, 1.0);
	}else if(u_PassthroughMode == BUMP_AS_DIFFUSE){
		out_Diff_Color = vec4(texture(u_BumpTex,vec2(fs_Texcoord)).rgb,1.0);
	}else if(u_PassthroughMode == MASK_OVERLAY){
		out_Diff_Color = vec4(texture(u_MaskTex,vec2(fs_Texcoord)).rgb,1.0);
	}//ELSE NO_CHANGE
	
	//Pass through specular color
	out_Spec_Color = vec4(u_Ks, 1.0);
	if(u_hasSpecTex > 0)
	{
		out_Spec_Color *=vec4(texture(u_SpecTex,vec2(fs_Texcoord)).rgb,1.0);
	}
}
