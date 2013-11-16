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
#define DISPLAY_TONE 6
#define DISPLAY_BLOOM 7
#define DISPLAY_OCCLUSION 8


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
uniform sampler2D u_Speculartex;

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

//Helper function to automicatlly sample and unpack specular
vec4 sampleSpec(vec2 texcoords) {
    return texture(u_Speculartex,texcoords).xyzw;
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

//Disk of samples for the SS sampling
#define NUM_SS_SAMPLES 16
vec2 poissonDisk[NUM_SS_SAMPLES] = vec2[]( 
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


#define NUM_WS_SAMPLES 16
vec3 poissonSphere[NUM_WS_SAMPLES] = vec3[](
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




#define PI 3.141516

float gatherOcclusion( vec3 pt_normal,
        vec3 pt_position,
        vec3 occluder_normal,
        vec3 occluder_position) {    
   
    if(pt_normal == occluder_normal)
        return 0.0;
    
    float dis = length(pt_position - occluder_position);
    if(dis < falloff)
        return 0.0;

    vec3 dir = normalize(occluder_position - pt_position) * 1.4;
    return max(0.0, dot(pt_normal, dir) * 1.0) * (1.0 / (1.0 + dis)) * 2.3;   
}

const float SPHERE_RADIUS = 0.2f;
float SSAOWorld(float curr_depth, vec3 position, vec3 normal)
{
    float occlusion = 0.0f;

    vec3 random = (getRandomNormal(fs_Texcoord));    
    for(int i = 0; i < NUM_WS_SAMPLES; i++)
    {      
       vec3 dir = (reflect((poissonSphere[i]), random));
        //vec3 dir = normalize(poissonSphere[i]);

        vec3 ranPos = position + sign(dot(dir, normal)) * dir * SPHERE_RADIUS;

        vec4 screenPos = u_Persp * vec4(ranPos, 1.0);
        screenPos = screenPos / screenPos.w;
        screenPos = screenPos / 2.0 + 0.5;

        vec2 newTex = vec2(screenPos.x, screenPos.y);
        vec3 o_normal = sampleNrm(newTex);
        vec3 o_position = samplePos(newTex);
        occlusion += gatherOcclusion(normal, position, o_normal, o_position);
    }

    return occlusion/16.0;
}


const float SS_RADIUS = 0.02f;
float SSAOScreen(float curr_depth, vec3 position, vec3 normal)
{
    float occlusion = 0.0f;

    for(int i = 0; i < NUM_SS_SAMPLES; i++)
    {
        float random = getRandomScalar(vec2(fs_Texcoord.s + float(i) / u_ScreenWidth, fs_Texcoord.t + float(i) / u_ScreenHeight));
        //float random = getRandomScalar(fs_Texcoord);
        float ranAngle = random * 2.0 * PI;

        vec2 dir = normalize(vec2(cos(ranAngle), sin(ranAngle)));
        float r = length(poissonDisk[i]);

        float mx = r * SS_RADIUS;// / u_ScreenWidth;
        float my = r * SS_RADIUS;// / u_ScreenHeight;
   
        vec2 newTex = vec2(fs_Texcoord.s + mx , fs_Texcoord.t + my);
        //vec2 newTex = vec2(fs_Texcoord + poissonDisk[i] * SS_RADIUS);
        vec3 o_normal = sampleNrm(newTex);
        vec3 o_position = samplePos(newTex);
        occlusion += gatherOcclusion(normal, position, o_normal, o_position);
    //}
    }
    return occlusion/16.0;
}


const float REGULAR_SAMPLE_STEP = 0.012f;
float SSAOGird(float curr_depth, vec3 position, vec3 normal)
{
    float occlusion = 0.0f;   

    for(float i = -2; i <= 2.0; i+=1.0)
    {
        for(float j = -2; j <= 2.0; j+=1.0)
        {          
            if(i == 0.0 || j == 0.0)
                continue;
            float mx = i * REGULAR_SAMPLE_STEP;
            float my = j * REGULAR_SAMPLE_STEP;
            vec2 newTex = vec2(fs_Texcoord.s + mx, fs_Texcoord.t + my);
          
            vec3 o_normal = sampleNrm(newTex);
            vec3 o_position = samplePos(newTex);
            occlusion += gatherOcclusion(normal, position, o_normal, o_position);
        }
    }

    return occlusion/16.0;
}


///////////////////////////////////
// MAIN
//////////////////////////////////
float occlusion_strength = 1.5f;
void main() {

    float exp_depth = texture(u_Depthtex, fs_Texcoord).r;
    float lin_depth = linearizeDepth(exp_depth,u_Near,u_Far);

    vec3 normal = sampleNrm(fs_Texcoord);
    vec3 position = samplePos(fs_Texcoord);
    vec3 color = sampleCol(fs_Texcoord);
    vec3 light = u_Light.xyz;
    float lightRadius = u_Light.w;

    float occlusion = 0.0f;

    switch (u_DisplayType) {
        case(DISPLAY_DEPTH):
            out_Color = vec4(vec3(lin_depth),1.0f);            
            break;
        case(DISPLAY_NORMAL):
            out_Color = vec4(abs(normal),1.0f);
            break;
        case(DISPLAY_POSITION):
            out_Color = vec4(abs(position) / u_Far,1.0f);
            break;
        case(DISPLAY_COLOR):
            out_Color = vec4(color, 1.0);
            break;
        case(DISPLAY_OCCLUSION):
            //occlusion = SSAOScreen(lin_depth, position, normal);
            occlusion = SSAOWorld(lin_depth, position, normalize(normal));
            //occlusion = SSAOGird(lin_depth, position, normal);
            occlusion = clamp(occlusion * occlusion_strength, 0.0, 1.0);
            out_Color = vec4(vec3(1.0 - occlusion), 1.0);
            break;
        case(DISPLAY_LIGHTS):            
            break;
        case(DISPLAY_TOTAL):
            break;
    }	

    return;
}

