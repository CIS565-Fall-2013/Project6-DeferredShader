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
uniform sampler2D u_Posttex;
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;

uniform sampler2D u_Lightmaptex;

uniform int u_ScreenWidth;
uniform int u_ScreenHeight;

in vec2 fs_Texcoord;

out vec4 out_Color;
///////////////////////////////////////




uniform float zerothresh = 1.0f;
uniform float falloff = 0.1f;


/////////////////////////////////////
//				UTILITY FUNCTIONS
/////////////////////////////////////

//Helper function to automicatlly sample and unpack positions
vec3 sampleCol(vec2 texcoords) {
    return texture(u_Posttex,texcoords).xyz;
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


vec3 getBlurredLightMap(sampler2D lightTex, vec2 v_texCoord, int u_ScreenWidth, int u_ScreenHeight)
{

	float dx=2.0f/float(u_ScreenWidth);
	float dy=2.0f/float(u_ScreenHeight);
	vec2 tmpTexcord=v_texCoord;
	float totalweight=0.0f;
	float weight=0.0f;
	vec3 totalcolor=vec3(0,0,0);


	float blurradius=10.0f;
	float blurdelta=1.0f;
	float sigma=blurradius*1.24089642;
	for(float i=-blurradius;i<=blurradius+0.1f;i+=blurdelta) for(float j=-blurradius;j<=blurradius+0.1f;j+=blurdelta)
	{
		if(i*i+j*j>blurradius*blurradius)continue;
		tmpTexcord=v_texCoord+vec2(i*dx,j*dy);
		float thedist=i*dx*i*dx+j*dy*j*dy;
		vec3 tmpColor=texture(lightTex,tmpTexcord).xyz;
		
		
		weight=exp(-pow(thedist,1.0f)/2.0f/sigma/sigma)/(2*3.1415926*sigma*sigma);
		totalweight+=weight;
		//if(tmpColor.r<0.2f) continue;
		totalcolor+=tmpColor*weight;
	}
	if(totalweight<0.0001f) return vec3(0,0,0);
	return totalcolor;///totalweight;
}


const float occlusion_strength = 1.5f;
void main() {
    
	vec3 color = sampleCol(fs_Texcoord);
	vec3 lightmap=getBlurredLightMap(u_Lightmaptex,fs_Texcoord,u_ScreenWidth, u_ScreenHeight);

	vec2 tempTex;
	tempTex=fs_Texcoord+vec2(1.0f/float(u_ScreenWidth),0.0f); vec3 color1=sampleCol(tempTex); 
	tempTex=fs_Texcoord-vec2(1.0f/float(u_ScreenWidth),0.0f); vec3 color2=sampleCol(tempTex); 
	tempTex=fs_Texcoord+vec2(0.0f,1.0f/float(u_ScreenHeight)); vec3 color3=sampleCol(tempTex);
	tempTex=fs_Texcoord-vec2(0.0f,1.0f/float(u_ScreenHeight)); vec3 color4=sampleCol(tempTex);

    float gray = dot(color, vec3(0.2125, 0.7154, 0.0721));
	gray=1.0f;
    float vin = min(2*distance(vec2(0.5), fs_Texcoord), 1.0);
    out_Color = vec4(mix(pow(color,vec3(1.0/1.8)),vec3(gray),vin), 1.0);
	//out_Color=vec4((color*0.0f+(color1+color2+color3+color4)*0.25f),1.0);
	out_Color=vec4(color*0.5f+lightmap*3.5f,1.0f);
	//out_Color=vec4(color,1.0f);
    return;
}

