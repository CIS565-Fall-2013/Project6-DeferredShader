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

#define ARG_PASS 0
#define ARG_BLOOM_SELECT 1
#define ARG_BLOOM_2D 2
#define ARG_GLOW_SELECT 3
#define ARG_GLOW_2D 4
#define ARG_BLUR_X 5
#define ARG_BLUR_Y 6
#define ARG_VIGNETTE 7
#define ARG_SSAO_SCALE 8
#define ARG_BLEND 9

/////////////////////////////////////
// Uniforms, Attributes, and Outputs
////////////////////////////////////

#define PI 3.14159265

#define NUM_SAMPLES 4
#define NUM_SPIRAL_TURNS 4

uniform sampler2D u_Posttex;
uniform sampler2D u_Posttex2;
uniform sampler2D u_Positiontex;
uniform sampler2D u_Normaltex;
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;

uniform int u_ScreenWidth;
uniform int u_ScreenHeight;

uniform int u_Mode;
uniform int u_kernelWidthX;
uniform int u_kernelWidthY;

in vec2 fs_Texcoord;

out vec4 out_Color;
///////////////////////////////////////

#define BLOOM_THRESHOLD 0.8

vec2 texel = vec2(1.0/float(u_ScreenWidth),1.0/float(u_ScreenHeight));
float oneOverSqrtTwoPi = 0.3989422804;

uniform float zerothresh = 1.0f;
uniform float falloff = 0.1f;


/////////////////////////////////////
//				UTILITY FUNCTIONS
/////////////////////////////////////

// Normal Distribution weighting
float gaussianWeight(float dx, float radius)
{
	//float width = (2.0 * radius + 1.0);
	//return 1.0/(width);

	if (radius < 0.0001) return 1.0;

	float sigma = radius/2.0;
	return (oneOverSqrtTwoPi / sigma) * exp(- (dx*dx)/(2.0 * sigma * sigma) );
}

//Helper function to automicatlly sample and unpack positions
vec4 sampleCol(vec2 texcoords) {
    return texture(u_Posttex,texcoords).xyzw;
}

vec4 samplePos(vec2 texcoords) {
    return texture(u_Positiontex,texcoords);
}

vec4 sampleNorm(vec2 texcoords) {
    return texture(u_Normaltex,texcoords);
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

vec3 pickUpAngle(int sampleNumber, float spinAngle) 
{
	float radiusSS;
	// radius relative to radiusSS
	float alpha = (float(sampleNumber) + 0.5) * (1.0 / float(NUM_SAMPLES));
	float angle = alpha * (float(NUM_SPIRAL_TURNS) * 6.28) + spinAngle;
  
	radiusSS = alpha;
	return vec3(cos(angle), sin(angle), radiusSS);
}

vec3 getOffsetPosition(vec2 unitOffset, float radiusSS) 
{
	vec2 offsetTexcoord = fs_Texcoord + radiusSS * unitOffset * (1.0 / sqrt(float(u_ScreenWidth * u_ScreenHeight)));
	return samplePos(offsetTexcoord).xyz;
}
 
float sampleAO(vec3 position, vec3 normal, float radius, int index, float angle)
{
	float epsilon = 0.0001;
	float radius2 = radius*radius;
	vec3 ret = pickUpAngle(index, angle);
	vec2 unitOffset = ret.xy;
	float radiusSS =  ret.z  * radius;

	vec3 Q = getOffsetPosition(unitOffset, radiusSS);
	vec3 v = Q - position;

	float bias = 0.001;

	float vv = dot(v,v);
	float vn = dot(v,normal) - bias;

	float invRadius2 = 1.0 / radius2;
	return 4.0 * max(1.0 - vv * invRadius2, 0.0) * max(vn, 0.0);
}
///////////////////////////////////
// MAIN
//////////////////////////////////
const float occlusion_strength = 1.5f;
void main() {
    vec4 color = sampleCol(fs_Texcoord) + vec4(vec3(0.0),1.0);

	float radiusX = float(u_kernelWidthX);
	float radiusY = float(u_kernelWidthY);

	if(u_Mode==ARG_PASS)
	{
		out_Color = color;
	}
	else if(u_Mode == ARG_VIGNETTE)
	{
		float gray = dot(color.xyz, vec3(0.2125, 0.7154, 0.0721));
		float vin = min(2*distance(vec2(0.5), fs_Texcoord), 1.0);
		out_Color = vec4(mix(pow(color.xyz,vec3(1.0/1.8)),vec3(gray),vin), 1.0);
	}
	else if(u_Mode == ARG_BLOOM_2D)
	{
		// We only want to add the bloom, not reduce value of current pixel if it is already below threshold!
		vec3 finalColor = color.xyz;

		// Bloom Effect: simple 2D Lookup kernel
		for(int x = -u_kernelWidthX; x <= u_kernelWidthX ; x++)
		{
			for(int y = -u_kernelWidthY; y <= u_kernelWidthY; y++)
			{
				if(x==0 && y==0) continue;
				// Simple test case of blur
				vec2 currentTexCoord = fs_Texcoord + vec2(texel.s * x, texel.t * y);
				vec4 bColor = sampleCol(currentTexCoord);
				vec3 bloomColor = bColor.xyz;

				if(bloomColor.r > BLOOM_THRESHOLD || bloomColor.g > BLOOM_THRESHOLD || bloomColor.b > BLOOM_THRESHOLD)// || bloomColor.)
					finalColor += gaussianWeight(float(x),radiusX) * gaussianWeight(float(y),radiusY) * bloomColor;
			}
		}
		out_Color = vec4(finalColor,1.0);
	}
	else if(u_Mode == ARG_BLOOM_SELECT)
	{
		vec3 bloomColor = sampleCol(fs_Texcoord).xyz;
		if(bloomColor.r > BLOOM_THRESHOLD || bloomColor.g > BLOOM_THRESHOLD || bloomColor.b > BLOOM_THRESHOLD)// || bloomColor.)
			out_Color = vec4(bloomColor.xyz,1.0);
	}
	else if(u_Mode == ARG_GLOW_2D)
	{
	
		vec3 finalColor = color.xyz;

		// Bloom Effect: simple 2D Lookup kernel
		for(int x = -u_kernelWidthX; x <= u_kernelWidthX ; x++)
		{
			for(int y = -u_kernelWidthY; y <= u_kernelWidthY; y++)
			{
				if(x==0 && y==0) continue;

				// Simple test case of blur
				vec2 currentTexCoord = fs_Texcoord + vec2(texel.s * x, texel.t * y);
				vec4 glowColor = sampleCol(currentTexCoord);
				float glow = samplePos(currentTexCoord).w;
				// glow has been packed.
				glow = glow- 2.0 * floor(glow/2.0);
				if(glow > 0.0)
					finalColor += gaussianWeight(float(x),radiusX) * gaussianWeight(float(y),radiusY) * glowColor.xyz;
			}
		}
		out_Color = vec4(finalColor,1.0);
	}
	else if(u_Mode == ARG_GLOW_SELECT)
	{
		vec4 glowColor = sampleCol(fs_Texcoord);
		float glow = samplePos(fs_Texcoord).w;
		glow = glow- 2.0 * floor(glow/2.0);
		if(glow > 0.0)
			out_Color = vec4(glowColor.xyz,1.0);
	}
	else if(u_Mode == ARG_BLUR_X)
	{
		vec3 finalColor = vec3(0.0);
		for(int x = -u_kernelWidthX; x <= u_kernelWidthX ; x++)
		{
				int y=0;
				vec2 currentTexCoord = fs_Texcoord + vec2(texel.s * x, texel.t * y);
				vec4 blurColor = sampleCol(currentTexCoord);
				finalColor += gaussianWeight(float(x),radiusX) * blurColor.xyz;
		}
		out_Color = vec4(finalColor,1.0);

	}
	else if(u_Mode == ARG_BLUR_Y)
	{
		vec3 finalColor = vec3(0.0);//color.xyz;
		for(int y = -u_kernelWidthY; y <= u_kernelWidthY ; y++)
		{
				int x=0;
				vec2 currentTexCoord = fs_Texcoord + vec2(texel.s * x, texel.t * y);
				vec4 blurColor = sampleCol(currentTexCoord);
				finalColor += gaussianWeight(float(y),radiusY) * blurColor.xyz;
		}
		out_Color = vec4(finalColor,1.0);
	}
	else if(u_Mode == ARG_SSAO_SCALE)
	{
		// http://graphics.cs.williams.edu/papers/SAOHPG12/
		vec3 position = samplePos(fs_Texcoord).xyz;
		vec3 normal = sampleNorm(fs_Texcoord).xyz;

		vec2 randTexCoord = vec2(getRandomScalar(fs_Texcoord),getRandomScalar(fs_Texcoord + fs_Texcoord*fs_Texcoord));
		vec3 sampleNoise = normalize(getRandomNormal(randTexCoord));
		//vec3 randNormal = normalize(getRandomNormal(fs_Texcoord));
		float randomPatternAngle = 2.0 * PI * sampleNoise.x;

		float radius = u_kernelWidthX;
		float occlusion = 0.0;

		for(int i=0; i < NUM_SAMPLES; ++i)
		{
			occlusion += sampleAO(position,normal,radius,i,randomPatternAngle);
		}
		float intensity = 10.0;
		occlusion = 1.0 - occlusion / (4.0 * float(NUM_SAMPLES));
		occlusion = clamp(pow(occlusion, 1.0 + intensity),0.0,1.0);
		out_Color = vec4(vec3(occlusion),1.0);
		out_Color = vec4(occlusion * color.xyz,1.0);
		//out_Color = vec4(vec3(randNormal),1.0);
	}
	else if(u_Mode == ARG_BLEND)
	{
		vec3 col1 = texture(u_Posttex,fs_Texcoord).xyz;
		vec3 col2 = texture(u_Posttex2,fs_Texcoord).xyz;
		out_Color = clamp(vec4(col1 + col2, 1.0),0.0,1.0);
	}
    return;
}

