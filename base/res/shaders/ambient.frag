#version 330

////////////////////////////
//       ENUMERATIONS
////////////////////////////

#define	DISPLAY_DEPTH 0
#define	DISPLAY_NORMAL 1
#define	DISPLAY_POSITION 2
#define	DISPLAY_COLOR 3
#define	DISPLAY_TOTAL 4
#define	DISPLAY_LIGHTS 5


/////////////////////////////////////
// Uniforms, Attributes, and Outputs
////////////////////////////////////
uniform mat4 u_Persp;

uniform sampler2D u_Depthtex;
uniform sampler2D u_Normaltex;
uniform sampler2D u_Positiontex;
uniform sampler2D u_Colortex;
uniform sampler2D u_Lightmaptex;
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;

uniform float u_Far;
uniform float u_Near;
uniform int u_OcclusionType;
uniform int u_DisplayType;

uniform int u_ScreenWidth;
uniform int u_ScreenHeight;

uniform vec4 u_Light;
uniform float u_LightIl;

in vec2 fs_Texcoord;

out vec4 out_Color;
out vec4 out_postlight;
///////////////////////////////////////




uniform float zerothresh = 1.0f;
uniform float falloff = 0.1f;


/////////////////////////////////////
//				UTILITY FUNCTIONS
/////////////////////////////////////

//Depth used in the Z buffer is not linearly related to distance from camera
//This restores linear depth
float linearizeDepth(float exp_depth, float near, float far) {
    return	(2 * near) / (far + near -  exp_depth * (far - near)); 
}

//Helper function to automatically sample and unpack normals
vec3 sampleNrm(vec2 texcoords) {
    return texture(u_Normaltex,texcoords).xyz;
}

//Helper function to automicatlly sample and unpack positions
vec3 samplePos(vec2 texcoords) {
    return texture(u_Positiontex,texcoords).xyz;
}

//Helper function to automicatlly sample and unpack positions
vec3 sampleCol(vec2 texcoords) {
    return texture(u_Colortex,texcoords).xyz;
}

//Get a random normal vector  given a screen-space texture coordinate
//Actually accesses a texture of random vectors
vec3 getRandomNormal(vec2 texcoords) {
    ivec2 sz = textureSize(u_RandomNormaltex,0);
    return texture(u_RandomNormaltex,vec2(texcoords.s* (u_ScreenWidth)/sz.x,
                (texcoords.t)*(u_ScreenHeight)/sz.y)).rgb;
}


//Get a random scalar given a screen-space texture coordinate
//Fetches from a random texture
float getRandomScalar(vec2 texcoords) {
    ivec2 sz = textureSize(u_RandomScalartex,0);
    return texture(u_RandomScalartex,vec2(texcoords.s*u_ScreenWidth/sz.x,
                texcoords.t*u_ScreenHeight/sz.y)).r;
}

///////////////////////////////////
// MAIN
//////////////////////////////////
const float occlusion_strength = 1.5f;

vec3 colorProduct(vec3 c1, vec3 c2)
{
	return vec3(c1.x*c2.x,c1.y*c2.y,c1.z*c2.z);
}



void main() {

    float exp_depth = texture(u_Depthtex, fs_Texcoord).r;
    float lin_depth = linearizeDepth(exp_depth,u_Near,u_Far);
    vec3 normal = sampleNrm(fs_Texcoord);
    vec3 position = samplePos(fs_Texcoord);
    vec3 color = sampleCol(fs_Texcoord);
	vec3 lightcolor=texture(u_Lightmaptex,fs_Texcoord).xyz;

	vec2 tempTex;
	tempTex=fs_Texcoord+vec2(1.0f/float(u_ScreenWidth),0.0f); float depth1=texture(u_Depthtex,tempTex).r; float angle1=dot(sampleNrm(tempTex),normal); vec3 lightcolor1=texture(u_Lightmaptex,tempTex).xyz;
	tempTex=fs_Texcoord-vec2(1.0f/float(u_ScreenWidth),0.0f); float depth2=texture(u_Depthtex,tempTex).r; float angle2=dot(sampleNrm(tempTex),normal); vec3 lightcolor2=texture(u_Lightmaptex,tempTex).xyz;
	tempTex=fs_Texcoord+vec2(0.0f,1.0f/float(u_ScreenHeight)); float depth3=texture(u_Depthtex,tempTex).r; float angle3=dot(sampleNrm(tempTex),normal);vec3 lightcolor3=texture(u_Lightmaptex,tempTex).xyz;
	tempTex=fs_Texcoord-vec2(0.0f,1.0f/float(u_ScreenHeight)); float depth4=texture(u_Depthtex,tempTex).r; float angle4=dot(sampleNrm(tempTex),normal);vec3 lightcolor4=texture(u_Lightmaptex,tempTex).xyz;

	depth1=linearizeDepth(depth1,u_Near,u_Far);
	depth2=linearizeDepth(depth2,u_Near,u_Far);
	depth3=linearizeDepth(depth3,u_Near,u_Far);
	depth4=linearizeDepth(depth4,u_Near,u_Far);

	
	//color+=lightmap;
	//color=colorProduct(color,vec3(196.0f/255.0f,165.0f/255.0f,21.0f/255.0f));
    vec3 light = u_Light.xyz;
    float strength = u_Light.w;
    if (lin_depth > 0.99f) {
        out_Color = vec4(vec3(0.0), 1.0);
		//out_Color = vec4(vec3(1.0), 1.0);
    } else {
        float ambient = u_LightIl;
		//strength=1.0f;
		//ambient=0.3f;
        float diffuse = max(0.0, dot(normalize(light),normal));
		//float diffuse2 = max(0.0, dot(normalize(-light),normal));
		//float diffuse=(diffuse1*0.9f+diffuse2*0.1f);
	
		if(diffuse<0.05f) diffuse=0.0f;
		else if(diffuse<0.25f) diffuse=0.25f;
		else if(diffuse<0.5f) diffuse=0.5f;
		else if(diffuse<0.75f) diffuse=0.75f;
		else if(diffuse<1.0f) diffuse=1.0f;




        out_Color = vec4(color*(strength*diffuse + ambient),1.0f);
		if(length(lightcolor)>0.01f || length(lightcolor1)>0.01f || length(lightcolor2)>0.01f || length(lightcolor3)>0.01f || length(lightcolor4)>0.01f ) return;


		float checkedge=-dot(normalize(position),normal);
		//if(checkedge<0.1f) out_Color=vec4(0,0,0,1);

		


		if(depth1>1.05f*lin_depth || depth1<0.95f*lin_depth || angle1<0.7f||
		depth2>1.05f*lin_depth || depth2<0.95f*lin_depth ||angle2<0.7f||
		depth3>1.05f*lin_depth || depth3<0.95f*lin_depth ||angle3<0.7f||
		depth4>1.05f*lin_depth || depth4<0.95f*lin_depth ||angle4<0.7f)
			out_Color=vec4(0,0,0,1)
		;
    }	
    return;
}

