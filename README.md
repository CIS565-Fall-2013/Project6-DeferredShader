-------------------------------------------------------------------------------
CIS565: Project 6: Deferred Shader
-------------------------------------------------------------------------------
Fall 2013
-------------------------------------------------------------------------------
Due Friday 11/15/2013
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
INTRODUCTION
-------------------------------------------------------------------------------
This project is about deferred shading using glsl. In this deferred shading, the first-path shader is just as a common shader: the input is the meshes. When calculating the output, it do not shade the things like 
ambient light, specular light or sth. Instead, it will output the normal, color, position and emission of the point that the pixel is shading on. 

And on the second-path is the true shader(so it is deferred), where use these information, to calculation the final fragment color of each pixel. It is somewhat similar to ray-tracing, but using rasterization to 
replace the ray intersection with the scene. 

In the project, as required, I implemented both Toon shading and Blooming effect(actually more likely to be a glowing effect). 

-------------------------------------------------------------------------------
HOW TO USE
-------------------------------------------------------------------------------

To turn on and off toon shading, uncomment/comment the #define USETOONSHADE in the ambient.frag

To turn on and off SSAO, uncomment/comment the #define USESSAO in the ambient.frag

To turn on and off pointlight, uncomment/comment the #define USEPOINTLIGHT in the point.frag

The bloom shader is default on. I thus fork the cornell_box into 2 versions: cornell_box_notglow.obj and cornell_box_glow.obj. Changing the input argument will work.

By the way, I used "Ka" in the mtl file as the emission of a material. So if you want something shiny, just change it!


-------------------------------------------------------------------------------
TOON SHADING
-------------------------------------------------------------------------------
The toon shading happen in the shader, instead of post-shading processing. The toon shading is simple: So for each dot(light,normal), I do not use it directly. Instead, it will be clamped to the threshold above it.
So in my program, 
0.00-0.05->0.00

0.05-0.25->0.25

0.25-0.50->0.50

0.50-0.75->0.75

0.75-1.00->1.00

Another tricky part of the toon shading is the edge detection. If using post-shading process, it can be done easily with Sobel matrix(just multiply the Sobel operator with the pixel color.) Instead, I introduced my own
 way to detect edge: compare the depth and the normal between itself and the G-Buffer around it. If an obvious difference is detected, than it will be shaded as the edge(color=0,0,0,1)
 
 And below is the Toon shading result
 ![Toon Shading](https://github.com/heguanyu/Project6-DeferredShader/blob/master/img/toonshade.jpg?raw=true)
 ![Toon Shading2](https://github.com/heguanyu/Project6-DeferredShader/blob/master/img/toonshade2.jpg?raw=true)
-------------------------------------------------------------------------------
BLOOM SHADING
-------------------------------------------------------------------------------
The Bloom shading is not that successful, and more likely to be a glow shading.

I implement the bloom shading in the post-shading process: in the post fragment shader. So to implement this, I add a slot in the G-Buffer: the emission of the objects. I notice that the Ka(ambient) of the material
 is the light that the object's emitting. (If it is a light or glowing object, then it is greater than (0,0,0), otherwise for normal object it is (0,0,0)). So in the pass shader, I put this emission information into
  another texture(G Buffer slot). And I retrieve this information in 2 places:
  
 1. In toon-shader, if a pixel is a light source, then it will take the edge test.
 
 2. In the post shader: I applied the Gaussian Blur function to make this effect. Gaussian Function is easy: 
 
 float sigma=...; //tweak this number will tweak the sharpness of the Gaussian Blur
 
 g=exp(-(dx*dx+dy*dy)/2.0f/sigma/sigma)/(2/PI/sigma/sigma)
 
 One of the problem with Gaussian function is that it have to sample a lot of pixels around it, if the blur radius is large. To settle this problem, I increase the step size of the sampling so that the blurring
  effect can expand to further place, and the sample rate per pixel was 21*21, in my program.

Below is the blooming effect. I set the walls and blocks to be the light sources
![Bloom Shading](https://github.com/heguanyu/Project6-DeferredShader/blob/master/img/bloom.jpg?raw=true)

-------------------------------------------------------------------------------
SSAO
-------------------------------------------------------------------------------

The SSAO, Screen-Space-Ambient-Occlusion, is a kind of algorithm that can shade the ambient shadow. The standard way of doing SSAO is like below:

1. Sample a point in hemisphere around the normal. The radius is flexible

2. Cast this point into the eye-coordination

3. Calculate the screen-position of this point

4. Compare the Z-value with the Z-buffer on this pixel. If it is closer, thats fine. Otherwise it is occluded, record it.

5. Loop the above steps(1-4) for several times(8-32), sum the total occluded amount.

6. Attenuate the color according to this occluded amount.

I hack it a little bit:

1. Sample the pixels around the pixel I'm calculating

2. Get the Z-depth and the position of that point

3. If that Z-depth is further than my z-depth, no occlusion happen.

4. If our distance is too far away, no occlusion happen

5. Otherwise I'm occluded by this point, record it.

6. Loop the above steps for each sample point in a certain radius, sum the total occluded amount

7. Attenuate the color according to this occluded amount.

This process is handled in the ambient.frag, before the diffuse happens.

Below are 2 comparison of the result of crytek-sponza's scene. The first one without SSAO, the second one with SSAO

![SSAO Compare 1](https://github.com/heguanyu/Project6-DeferredShader/blob/master/img/SSAO-compare.jpg?raw=true)

![SSAO Compare 2](https://github.com/heguanyu/Project6-DeferredShader/blob/master/img/SSAO-compare2.jpg?raw=true)

Below are something interesting: when I increase the scope of the SSAO, it turns out sth like a stone carve effect

![SSAO overshoot](https://github.com/heguanyu/Project6-DeferredShader/blob/master/img/large-radius-SSAO.jpg?raw=true)

-------------------------------------------------------------------------------
POINTLIGHT
-------------------------------------------------------------------------------

The point light is straight forward. It can be better seen in the cornell_box_notglow.obj scene.

You should turn off the scissors test to make sure that the point light is working correctly. 

result is as below

![Pointlight](https://github.com/heguanyu/Project6-DeferredShader/blob/master/img/pointlight.jpg?raw=true)

-------------------------------------------------------------------------------
Setting Up the project
-------------------------------------------------------------------------------
It took a while to figure out the problem with the project settings. To make this project work, 

1. Project settings->configuration properties->linker->input->add "..\release\SOIL.lib" in it.

2. Project settings->debugging->add mesh="..\..\..\res\cornell\cube.obj" 

3. In the main.cpp file, for each directory of those shaders file, add ../../   before it so that it can approach the file correctly.

-------------------------------------------------------------------------------
PERFORMANCE EVALUATION
-------------------------------------------------------------------------------
With more sample in blurring and SSAO, it become slower....

---
ACKNOWLEDGEMENTS
---
This project makes use of [tinyobjloader](http://syoyo.github.io/tinyobjloader/) and [SOIL](http://lonesock.net/soil.html)
