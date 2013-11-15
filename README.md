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
REQUIREMENTS:
-------------------------------------------------------------------------------

In this project, you are given code for:
* Loading .obj files
* Rendering to a minimal G buffer:
  * Depth
  * Normal
  * Color
  * Eye space position
* Rendering simple ambient and directional lighting to texture
* Example post process shader to add a vignette

You are required to implement:
* Either of the following effects
  * Bloom (feel free to use [GPU Gems](http://http.developer.nvidia.com/GPUGems/gpugems_ch21.html) as a rough guide)
  * "Toon" Shading (with basic silhouetting)
* Point light sources
* An additional G buffer slot and some effect showing it off

**NOTE**: Implementing separable convolution will require another link in your pipeline and will count as an extra feature if you do performance analysis with a standard one-pass 2D convolution. The overhead of rendering and reading from a texture _may_ offset the extra computations for smaller 2D kernels.

You must implement two of the following extras:
* The effect you did not choose above
* Screen space ambient occlusion
* Compare performance to a normal forward renderer with
  * No optimizations
  * Coarse sort geometry front-to-back for early-z
  * Z-prepass for early-z
* Optimize g-buffer format, e.g., pack things together, quantize, reconstruct z from normal x and y (because it is normalized), etc.
  * Must be accompanied with a performance analysis to count
* Additional lighting and pre/post processing effects! (email first please, if they are good you may add multiple).

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
The performance evaluation is where you will investigate how to make your 
program more efficient using the skills you've learned in class. You must have
performed at least one experiment on your code to investigate the positive or
negative effects on performance. 

We encourage you to get creative with your tweaks. Consider places in your code
that could be considered bottlenecks and try to improve them. 

Each student should provide no more than a one page summary of their
optimizations along with tables and or graphs to visually explain any
performance differences.

-------------------------------------------------------------------------------
THIRD PARTY CODE POLICY
-------------------------------------------------------------------------------
* Use of any third-party code must be approved by asking on the Google groups.  
  If it is approved, all students are welcome to use it.  Generally, we approve 
  use of third-party code that is not a core part of the project.  For example, 
  for the ray tracer, we would approve using a third-party library for loading 
  models, but would not approve copying and pasting a CUDA function for doing 
  refraction.
* Third-party code must be credited in README.md.
* Using third-party code without its approval, including using another 
  student's code, is an academic integrity violation, and will result in you 
  receiving an F for the semester.

-------------------------------------------------------------------------------
SELF-GRADING
-------------------------------------------------------------------------------
* On the submission date, email your grade, on a scale of 0 to 100, to Liam, 
  liamboone@gmail.com, with a one paragraph explanation.  Be concise and 
  realistic.  Recall that we reserve 30 points as a sanity check to adjust your 
  grade.  Your actual grade will be (0.7 * your grade) + (0.3 * our grade).  We 
  hope to only use this in extreme cases when your grade does not realistically 
  reflect your work - it is either too high or too low.  In most cases, we plan 
  to give you the exact grade you suggest.
* Projects are not weighted evenly, e.g., Project 0 doesn't count as much as 
  the path tracer.  We will determine the weighting at the end of the semester 
  based on the size of each project.


---
SUBMISSION
---
As with the previous projects, you should fork this project and work inside of
your fork. Upon completion, commit your finished project back to your fork, and
make a pull request to the master repository.  You should include a README.md
file in the root directory detailing the following

* A brief description of the project and specific features you implemented
* At least one screenshot of your project running.
* A link to a video of your project running.
* Instructions for building and running your project if they differ from the
  base code.
* A performance writeup as detailed above.
* A list of all third-party code used.
* This Readme file edited as described above in the README section.

---
ACKNOWLEDGEMENTS
---
This project makes use of [tinyobjloader](http://syoyo.github.io/tinyobjloader/) and [SOIL](http://lonesock.net/soil.html)
