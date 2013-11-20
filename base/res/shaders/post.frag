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
#define DISPLAY_TOON 6
#define DISPLAY_BLUR 7
#define DISPLAY_DOF 8
#define DISPLAY_GLOW 9

#define E 2.71828182846
#define PI 3.14159


/////////////////////////////////////
// Uniforms, Attributes, and Outputs
////////////////////////////////////
uniform sampler2D u_Posttex;
uniform sampler2D u_Positiontex;
uniform sampler2D u_Normaltex;
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;
uniform sampler2D u_Colortex;
uniform sampler2D u_Depthtex;
uniform sampler2D u_Glowtex;

uniform float u_Far;
uniform float u_Near;

uniform int u_ScreenWidth;
uniform int u_ScreenHeight;

uniform int u_DisplayType;

uniform mat4 u_Persp;

in vec2 fs_Texcoord;

out vec4 out_Color;
///////////////////////////////////////

uniform float zerothresh = 1.0f;
uniform float falloff = 0.1f;

float texelHeight = 1.0f / float(u_ScreenHeight);
float texelWidth = 1.0f / float(u_ScreenWidth);

/////////////////////////////////////
//				UTILITY FUNCTIONS
/////////////////////////////////////

//Helper function to automicatlly sample and unpack positions
vec3 sampleCol(vec2 texcoords) {
     return texture(u_Posttex,texcoords).xyz;
}

vec3 sampleGlow(vec2 texcoords){
    return texture(u_Glowtex,texcoords).xyz;
}

vec3 retrieveNormal(vec2 n){
    vec3 normal = vec3(n.x, n.y, 0.0); 
	normal.z = sqrt(1.0 - dot(n.xy, n.xy));
    return normal;
}

vec3 sampleNormal(vec2 texcoords){
    return retrieveNormal(texture(u_Normaltex, texcoords).xy);
}

//Get a random normal vector  given a screen-space texture coordinate
//Actually accesses a texture of random vectors
vec3 getRandomNormal(vec2 texcoords) {
    ivec2 sz = textureSize(u_RandomNormaltex,0);
    return texture(u_RandomNormaltex,vec2(texcoords.s* (u_ScreenWidth)/sz.x,
                (texcoords.t)*(u_ScreenHeight)/sz.y)).rgb;
}

//Depth used in the Z buffer is not linearly related to distance from camera
//This restores linear depth
float linearizeDepth(float exp_depth, float near, float far) {
    return	(2 * near) / (far + near -  exp_depth * (far - near)); 
}

//Get a random scalar given a screen-space texture coordinate
//Fetches from a random texture
float getRandomScalar(vec2 texcoords) {
    ivec2 sz = textureSize(u_RandomScalartex,0);
    return texture(u_RandomScalartex,vec2(texcoords.s*u_ScreenWidth/sz.x,
                texcoords.t*u_ScreenHeight/sz.y)).r;
}

vec3 getRandomNoise(vec2 texcoords){
   ivec2 sz = textureSize(u_RandomScalartex,0);
   return texture(u_RandomScalartex,vec2(texcoords.s*u_ScreenWidth/sz.x,
                texcoords.t*u_ScreenHeight/sz.y)).xyz;
}

///////////////////////////////////
// EDGE DETECTION
///////////////////////////////////

float edgeWeight(){
  vec2 UL = vec2(-texelWidth, -texelHeight), UC = vec2(0, -texelHeight), UR = vec2(texelWidth, -texelHeight);
  vec2 L = vec2(-texelWidth, 0), R = vec2(texelWidth, 0);
  vec2 LL = vec2(-texelWidth, texelHeight), LC = vec2(0, texelHeight), LR = vec2(texelWidth, texelHeight);

  vec3 gx = (texture(u_Colortex, fs_Texcoord + UL).rgb - texture(u_Colortex, fs_Texcoord + UR).rgb) +
            (2 * texture(u_Colortex, fs_Texcoord + L).rgb - 2 * texture(u_Colortex, fs_Texcoord + R).rgb) +
			(texture(u_Colortex, fs_Texcoord + LL).rgb - texture(u_Colortex, fs_Texcoord + LR).rgb);
  vec3 gy = (texture(u_Colortex, fs_Texcoord + UL).rgb + 2 * texture(u_Colortex, fs_Texcoord + UC).rgb + texture(u_Colortex, fs_Texcoord + UR).rgb) -
            (texture(u_Colortex, fs_Texcoord + LL).rgb + 2 * texture(u_Colortex, fs_Texcoord + LC).rgb + texture(u_Colortex, fs_Texcoord + LL).rgb);
		  
  return length(gx) + length(gy);
}

///////////////////////////////////
// POSTERIZE
///////////////////////////////////

float posterize(float x, int numColors){
  float range = 1.0f / float(numColors + 1.0f);
  int i = int((x + range / 2.0f) / range);
  return float(i) * range;
}

///////////////////////////////////
// TOON SHADING
///////////////////////////////////

vec4 toonShade(vec3 color, int numColors){
  vec3 p_color = vec3(posterize(color.r, numColors), posterize(color.g, numColors), posterize(color.b, numColors));
  return abs(edgeWeight()) > .6f ? vec4(vec3(0.0f), 1.0f) : vec4(p_color, 1.0f);
}

///////////////////////////////////
// SCREEN SPACE AMBIENT OCCLUSION
///////////////////////////////////

vec2 poissonDisk[16] = vec2[](
    vec2( -0.94201624, -0.39906216 ),
        vec2( 0.94558609, -0.76890725 ),
        vec2( -0.094184101, -0.92938870 ),
        vec2( 0.34495938, 0.29387760 ),
        vec2( -0.91588581, 0.45771432 ),
        vec2( -0.81544232, -0.87912464 ),
        vec2( -0.38277543, 0.27676845 ),
        vec2( 0.97484398, 0.75648379 ),
        vec2( 0.44323325, -0.97511554 ),
        vec2( 0.53742981, -0.47373420 ),
        vec2( -0.26496911, -0.41893023 ),
        vec2( 0.79197514, 0.19090188 ),
        vec2( -0.24188840, 0.99706507 ),
        vec2( -0.81409955, 0.91437590 ),
        vec2( 0.19984126, 0.78641367 ),
        vec2( 0.14383161, -0.14100790)
);

vec3 poissonSphere[16] = vec3[](
        vec3(0.53812504, 0.18565957, -0.43192),
        vec3(0.13790712, 0.24864247, 0.44301823),
        vec3(0.33715037, 0.56794053, -0.005789503),
        vec3(-0.6999805, -0.04511441, -0.0019965635),
        vec3(0.06896307, -0.15983082, -0.85477847),
        vec3(0.056099437, 0.006954967, -0.1843352),
        vec3(-0.014653638, 0.14027752, 0.0762037),
        vec3(0.010019933, -0.1924225, -0.034443386),
        vec3(-0.35775623, -0.5301969, -0.43581226),
        vec3(-0.3169221, 0.106360726, 0.015860917),
        vec3(0.010350345, -0.58698344, 0.0046293875),
        vec3(-0.08972908, -0.49408212, 0.3287904),
        vec3(0.7119986, -0.0154690035, -0.09183723),
        vec3(-0.053382345, 0.059675813, -0.5411899),
        vec3(0.035267662, -0.063188605, 0.54602677),
        vec3(-0.47761092, 0.2847911, -0.0271716));

float gatherOcclusion(vec3 pt_normal, vec3 pt_position, vec3 occluder_normal, vec3 occluder_position){
		float p = min(1.0 - dot(pt_normal, occluder_normal), 1.0);
	    float d = length(occluder_position - pt_position);
		return p * 1.0 / (d * d + .005) * dot(pt_normal, occluder_position - pt_position);
}

float ssao(float sampling_radius, vec2 texcoord){
	float rand = mix(0.0, 2.0 * 3.14159, getRandomScalar(texcoord));
	mat2 rot = mat2(vec2(cos(rand), sin(rand)), vec2(-sin(rand), cos(rand)));

	float occlusion = 0.0;

	for(int i = 0; i < 16; i++){
		vec2 sample = rot * poissonDisk[i];

		// Accumulate occlusion
		vec3 point_pos = texture(u_Positiontex, texcoord).xyz;
		vec3 point_n = texture(u_Normaltex, texcoord).xyz;
		vec3 occ_pos = texture(u_Positiontex, sample).xyz;
		vec3 occ_n = texture(u_Normaltex, sample).xyz;
		occlusion += gatherOcclusion(point_n, point_pos, occ_n, occ_pos);
	}
	// Adjust occlusion to light values
	occlusion = 1.0 - (occlusion / 16.0);
	return occlusion;
}


///////////////////////////////////
// GAUSSIAN BLUR
///////////////////////////////////

vec4 blur(float sigma, float r, bool glow){
  vec3 color = vec3(0.0);
  float g;
  for(float i = -r; i <= r; i += 1.0){
    for(float j = -r; j <= r; j += 1.0){
	  g = 1.0 / (2.0 * PI * sigma * sigma) * pow(E, -((i*i + j*j) / (2.0 * sigma * sigma))); 
	  vec2 texel = fs_Texcoord + vec2(i * texelWidth , j * texelHeight);
	  color += g * (glow ? sampleGlow(texel) : sampleCol(texel));
	}
  }
  return vec4(color, 1.0);
}

///////////////////////////////////
// DEPTH OF FIELD
///////////////////////////////////

vec4 depthOfField(float focusPlane, float aperture){
    float exp_depth = texture(u_Depthtex, fs_Texcoord).r;
    float lin_depth = linearizeDepth(exp_depth,u_Near,u_Far);

	float d = lin_depth - focusPlane;
	return blur((abs(d) + .1) * 10.0, aperture, false);
}

///////////////////////////////////
// MAIN
//////////////////////////////////
const float occlusion_strength = 1.0f;
void main() {

	int numColors = 5;
    vec3 color = sampleCol(fs_Texcoord);
    float gray = dot(color, vec3(0.2125, 0.7154, 0.0721));
    float vin = min(2*distance(vec2(0.5), fs_Texcoord), 1.0);
    if(u_DisplayType == DISPLAY_TOON){
	  out_Color = toonShade(color, numColors);
    }else if(u_DisplayType == DISPLAY_BLUR){
	  out_Color = blur(5.0, 10.0, false);
	}else if(u_DisplayType == DISPLAY_DOF){
	  out_Color = depthOfField(.2, 4.0);
	}else if(u_DisplayType == DISPLAY_GLOW){
	  out_Color = vec4(color + vec3(3.0 * blur(15.0, 25.0, true)), 1.0);
	}else{
	  //out_Color = vec4(clamp(occlusion_strength * ssao(0.8f, fs_Texcoord), 0.0, 1.0) * color, 1.0);
	  out_Color = vec4(color, 1.0);
	}
    return;
}

