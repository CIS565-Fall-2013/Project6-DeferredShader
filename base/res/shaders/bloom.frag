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
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;

uniform float u_Far;
uniform float u_Near;
uniform int u_DisplayType;

uniform int u_ScreenWidth;
uniform int u_ScreenHeight;

uniform vec4 u_Light;
uniform float u_LightIl;

in vec2 fs_Texcoord;

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
void main() {

    float exp_depth = texture(u_Depthtex, fs_Texcoord).r;
    float lin_depth = linearizeDepth(exp_depth,u_Near,u_Far);

	vec3 light = u_Light.xyz;
    float strength = u_Light.w;
	vec3 color = sampleCol(fs_Texcoord);
	vec3 normal = sampleNrm(fs_Texcoord);

    if (lin_depth > 0.99f) {
		out_Color = vec4(vec3(0.5), 1.0);
	}
    else if ( u_DisplayType == DISPLAY_TOTAL )
    {
		float ambient = u_LightIl;
		float diffuse = max(0.0, dot(normalize(light),normal));
		out_Color = vec4(color*(strength*diffuse + ambient),1.0f);

		vec3 red = vec3(1.0, 0.0, 0.0);
		vec3 accumulated = vec3(0.0);
		//float sigma = 6.0; //for Gaussian blur
		for (int i=-20; i<=20; i++) {
			for (int j=-20; j<=20; j++) {
				vec2 texCoord = fs_Texcoord + i * vec2(1.0/u_ScreenWidth, 0.0)
					+ j * vec2(0.0, 1.0/u_ScreenHeight);
				vec3 color = sampleCol(texCoord);
				if (distance(color, red) < 0.001) {
					//float G = 1.0/(2*3.1415926*sigma*sigma) * exp(-(i*i+j*j)/(2*sigma*sigma)); //Gaussian
					accumulated += (color+vec3(0.3));
				}
			}
		}
		if (length(accumulated) > 0.001) {
			out_Color = vec4(accumulated * 0.001 + out_Color.xyz, 0.0);
		}
    }
	else if ( u_DisplayType == DISPLAY_LIGHTS ) {
		float ambient = u_LightIl;
		float diffuse = max(0.0, dot(normalize(light),normal));
		out_Color = vec4(color*(strength*diffuse + ambient),1.0f);
	}
	else
	{
		out_Color = vec4(vec3(0.0), 1.0);
	}
    return;
}

