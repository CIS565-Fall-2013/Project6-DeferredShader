layout(binding = 1) uniform PerDraw_Light
{
    vec4 uf4Light;
    vec3 uf3LightCol;
    float ufLightIl;
};

layout(binding = 2) uniform PerFrame_Light
{
    vec4 uf4DirecLightDir;
    vec3 uf3DirecLightCol;
    vec3 uf3AmbientContrib;
};