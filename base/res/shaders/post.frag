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
#define	DISPLAY_TOON 7
#define	DISPLAY_PART 8
#define	DISPLAY_FISH 9

/////////////////////////////////////
// Uniforms, Attributes, and Outputs
////////////////////////////////////
uniform sampler2D u_Posttex;
uniform sampler2D u_Depthtex;
uniform float u_Far;
uniform float u_Near;
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;

uniform int u_ScreenWidth;
uniform int u_ScreenHeight;

in vec2 fs_Texcoord;
uniform int u_DisplayType;

out vec4 out_Color;
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

//Helper function to automicatlly sample and unpack positions
vec3 sampleCol(vec2 texcoords) {
    return texture(u_Posttex,texcoords).xyz;
}

//Helper function to automicatlly sample and unpack depths
vec3 sampleDep(vec2 texcoords) {
    return texture(u_Depthtex,texcoords).xyz;
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
void main() {
 float exp_depth = texture(u_Depthtex, fs_Texcoord).r;
 float lin_depth = linearizeDepth(exp_depth,u_Near,u_Far);

    // Silhotte

    vec3 color = sampleCol(fs_Texcoord);
    float gray = dot(color, vec3(0.2125, 0.7154, 0.0721));
    float vin = min(2*distance(vec2(0.5), fs_Texcoord), 1.0);
    out_Color = vec4(mix(pow(color,vec3(1.0/1.8)),vec3(gray),vin), 1.0);

	// Toon Shading 

	if(u_DisplayType == DISPLAY_TOON)
	{
	vec2 texCord = fs_Texcoord;
	const float ygrad[9] = {1,0,-1,2,0,-2,1,0,-1};
	const float xgrad[9] = {-1,-2,-1,0,0,0,1,2,1};
	int k = 0;
	float x = texCord.x - 1.0/u_ScreenWidth; 
	float y = texCord.y - 1.0/u_ScreenHeight ;
	float gradX = 0.0;
	float gradY = 0.0;

	for(int i = 0 ; i < 3 ; i++)
	{
		for(int j = 0 ; j < 3 ; j++)
		{
		    exp_depth = texture(u_Depthtex, vec2(x,y)).r;
			lin_depth = linearizeDepth(exp_depth,0.1,100);
			gradX += lin_depth * xgrad[k];
			gradY += lin_depth * ygrad[k];
			k++;
			y= y + 1.0/u_ScreenHeight;
		}
		y = texCord.y - 1.0/u_ScreenHeight;
		x = x + 1.0/u_ScreenWidth;
	}

	float edge = sqrt(gradX * gradX + gradY * gradY);

	if(edge > 0.05)
	{
		out_Color = vec4(0.0,0.0,0.0,1.0f);
	}
	else
	{
		if(color.x > 0 && color.x <= 0.2)
			color.x = 0.2;
		else if(color.x > 0.2 && color.x <= 0.4)
			color.x = 0.4;
		else if(color.x > 0.4 && color.x <= 0.6)
			color.x = 0.6;
		else if(color.x > 0.6 && color.x <= 0.8)
			color.x = 0.8;
		else if(color.x > 0.8 && color.x <= 1.0)
			color.x = 1.0;

		if(color.y > 0 && color.y <= 0.2)
			color.y = 0.2;
		else if(color.y > 0.2 && color.y <= 0.4)
			color.y = 0.4;
		else if(color.y > 0.4 && color.y <= 0.6)
			color.y = 0.6;
		else if(color.y > 0.6 && color.y <= 0.8)
			color.y = 0.8;
		else if(color.y > 0.8 && color.y <= 1.0)
			color.y = 1.0;

		if(color.z > 0 && color.z <= 0.2)
			color.z = 0.2;
		else if(color.z > 0.2 && color.z <= 0.4)
			color.z = 0.4;
		else if(color.z > 0.4 && color.z <= 0.6)
			color.z = 0.6;
		else if(color.z > 0.6 && color.z <= 0.8)
			color.z = 0.8;
		else if(color.z > 0.8 && color.z <= 1.0)
			color.z = 1.0;
		

	
		out_Color = vec4(color,1.0f);
	}
	}

	// PIXEL ART EFFECT -- MOSAIC EFFECT

	else if (u_DisplayType == DISPLAY_PART)
	{
	vec2 cood = fs_Texcoord;
	cood.x = (floor(cood.x * u_ScreenWidth / 10.0) / u_ScreenWidth) * 10.0;
	cood.y = (floor(cood.y * u_ScreenHeight / 10.0) / u_ScreenHeight) * 10.0 ;
	out_Color = vec4(sampleCol(cood),1.0f);
	}
	//Fish eye effect 
	else if (u_DisplayType == DISPLAY_FISH)
	{
	vec2 nc = fs_Texcoord - 0.5;
	float z = sqrt(1.0 - nc.x * nc.x - nc.y * nc.y);
	float a = 1.0 / (z * tan(15.0* 0.5 ));
	out_Color = vec4(sampleCol(nc*a + 0.5),1.0f);
	}
	//EMBOSS Post processing effect

	vec2 cood = fs_Texcoord;
	vec3 xcol = sampleCol(vec2(cood.x,cood.y));
	vec3 ycol = sampleCol(vec2(cood.x + 100/ u_ScreenWidth, cood.y + 100 / u_ScreenHeight));
	float r = min(abs(xcol.x - ycol.x) + 0.5, 1.0);
	float g = min(abs(xcol.y - ycol.y) + 0.5, 1.0);
	float b = min(abs(xcol.z - ycol.z) + 0.5, 1.0);
	//out_Color = vec4(vec3(r,g,b),1.0f);

	

    return;
}

