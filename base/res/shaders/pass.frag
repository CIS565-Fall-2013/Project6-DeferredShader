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
in vec3 fs_Tangent;
in vec3 fs_Binormal;
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
	
	vec4 diffuseColor = vec4(u_Kd,1.0);
	if(u_hasDiffTex > 0)
	{
		diffuseColor *=vec4(texture(u_DiffTex,fs_Texcoord).rgb,1.0);
	}
	
	//Pass through diffuse color
	out_Diff_Color = diffuseColor*vec4(u_Kd,1.0);
	 
	if(u_PassthroughMode == HASTEX_OVERLAY){
		out_Diff_Color += 0.9*vec4(u_hasMaskTex,u_hasSpecTex,u_hasBumpTex,0.0);
	}else if(u_PassthroughMode == TEXCOORDS_AS_DIFFUSE){
		out_Diff_Color = vec4(fs_Texcoord.x, fs_Texcoord.y, 0.0, 1.0);
	}
	
	
	if(u_hasMaskTex > 0)
	{
		//Only discard samples if not in overlay mode
		if(texture(u_MaskTex,fs_Texcoord).x < 1)
			discard;
		if(u_PassthroughMode == MASK_OVERLAY){
			out_Diff_Color = vec4(texture(u_MaskTex,fs_Texcoord).rgb,1.0);
		}
		
	}
	
	
	
	if(u_hasBumpTex > 0)
	{
		//TODO: Do Bump Mapping Here
		ivec2 size = textureSize2D(u_BumpTex,0);
		float scaling = 0.2;//Strength factor
		const vec3 offsetDir = vec3(-1,0,1);
		vec2 offset = 1.0/size;
		
		float s11 = texture(u_BumpTex, fs_Texcoord).x;//Height at center
		float s01 = texture(u_BumpTex, fs_Texcoord+offset*offsetDir.xy).x;//Left
		float s21 = texture(u_BumpTex, fs_Texcoord+offset*offsetDir.zy).x;//Right
		float s10 = texture(u_BumpTex, fs_Texcoord+offset*offsetDir.yx).x;//Down
		float s12 = texture(u_BumpTex, fs_Texcoord+offset*offsetDir.yz).x;//Up
		vec3 va = normalize(vec3(scaling,0.0,s21-s01));
		vec3 vb = normalize(vec3(0.0,scaling,s12-s10));
		vec4 bump = normalize(vec4( cross(va,vb), s11 ));
		if(u_PassthroughMode == BUMP_AS_DIFFUSE){
			out_Diff_Color = bump;
		}
		
		//Modify using 
		mat3 rotMat = mat3(fs_Tangent, fs_Binormal, fs_Normal);
		out_Normal = vec4(rotMat*vec3(bump),0.0);
	}	
	
	
	//Pass through specular color
	out_Spec_Color = vec4(u_Ks, u_specExp);
	if(u_hasSpecTex > 0)
	{
		out_Spec_Color *= vec4(texture(u_SpecTex,fs_Texcoord).rgb,1.0);
	}
}
