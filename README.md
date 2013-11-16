# Deferred Shader #

![Alt text](SSAOC.png "SSAO")
SSAO

![Alt text](ToneC.png "Tone Shading")
Tone Shading

![Alt text](totalC.png "Total")
Total


## Description: ##
In this project, I implemented separable convolution bloom effect, tone shading, point light, specular G buffer to display specular highlight and screen space ambient occlusion.

For the bloom effect, instead of doing one pass convolution, I did the two pass separable convolution which has a great speed up. For two pass, I added one more post fragment shader. I also make use of additional specular G buffer to determine which area to bloom. To define which material is effected by bloom effect, user just need to set Ns value to 101 in the .mtl file. I also using Gaussian blur effect for blurring the object.

For Tone shading, I am using sober filter to get the magnitude of the color. Also done in post fragment shader. 

For specular G buffer, I pass a vec4 value to fragment shader. This vec4 includes specular color and specular intensity. User can specify these values in .mtl file using Ks for specular color and Ns for specular intensity.

I did the SSAO using the poisson sphere which I got from 2012 fall GPU class. I did the grid space and world space SSAO.

Pree number change display mode.

- 1 Display depth
- 2 Display normal
- 3 Display color
- 4 Display position
- 5 Display light
- 6 Display tone shading
- 7 Display bloom effect
- 8 Display SSAO
- 0 Display total



## Performance Evaluation: ##

I did performance evaluation on one pass between two pass 2D convolution bloom effect. The two pass convolution has evident boost on frame rate. The most important reason is that the two pass convolution reduce the compute time from nxn to 2n.


![Alt text](TwopassOnepass.png "Two Pass Vs One Pass")


I also did a evaluation on the number of lights and frame rates. Obviously, the frame rate goes down very quickly when the number of lights increase.


![Alt text](numberLights.png "Number of Lights and Frame Rate")



## Screen Shots ##

![Alt text](before.png "before")
**Before Bloom Effect**

![Alt text](after.png "after")
**After Bloom Effect**

![Alt text](ToneB.png "Tone Shading")
**Tone Shading**

![Alt text](SSAOB.png "SSAO")
**SSAO**

![Alt text](withoutSpecular.png "Without Specular")
**With out Specular**

![Alt text](withSpecular.png "With Specular")
**With Specular**


-------------------------------------------------------------------------------
CIS565: Project 6: Deferred Shader
-------------------------------------------------------------------------------
Fall 2013
-------------------------------------------------------------------------------
Due Friday 11/15/2013
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
NOTE:
-------------------------------------------------------------------------------
This project requires any graphics card with support for a modern OpenGL 
pipeline. Any AMD, NVIDIA, or Intel card from the past few years should work 
fine, and every machine in the SIG Lab and Moore 100 is capable of running 
this project.

-------------------------------------------------------------------------------
INTRODUCTION:
-------------------------------------------------------------------------------
In this project, you will get introduced to the basics of deferred shading. You will write GLSL and OpenGL code to perform various tasks in a deferred lighting pipeline such as creating and writing to a G-Buffer.

-------------------------------------------------------------------------------
CONTENTS:
-------------------------------------------------------------------------------
The Project6 root directory contains the following subdirectories:
	
* base/
  * PROJ_WIN/ contains the vs2010 project files
  * PROJ_NIX/ contains makefile for building (tested on ubuntu 12.04 LTS)
  * res/ contains resources including shader source and obj files
  * src/ contains the c++ code for the project along with SOIL and tiny_obj_loader
* shared32/ contains freeglut, glm, and glew.

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
README
-------------------------------------------------------------------------------
All students must replace or augment the contents of this Readme.md in a clear 
manner with the following:

* A brief description of the project and the specific features you implemented.
* At least one screenshot of your project running.
* A 30 second or longer video of your project running.  To create the video you
  can use http://www.microsoft.com/expression/products/Encoder4_Overview.aspx 
* A performance evaluation (described in detail below).

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
