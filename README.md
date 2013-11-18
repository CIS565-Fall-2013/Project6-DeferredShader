-------------------------------------------------------------------------------
Deferred Shader
-------------------------------------------------------------------------------
Fall 2013
-------------------------------------------------------------------------------
![fullscene](/renders/FullHallAllTextures.PNG "Finished Rendering")

Youtube Video of Rendering:
<dl>
<a href="http://www.youtube.com/watch?feature=player_embedded&v=kvQ3dNG4Mdg
" target="_blank"><img src="http://img.youtube.com/vi/kvQ3dNG4Mdg/0.jpg" 
alt="Youtube Video of Rendering Process" width="480" height="360" border="10" /></a>
</dl>

NOTE:
-------------------------------------------------------------------------------
This project requires any graphics card with support for a modern OpenGL 
pipeline. Any AMD, NVIDIA, or Intel card from the past few years should work 
fine, and every machine in the SIG Lab and Moore 100 is capable of running 
this project.

-------------------------------------------------------------------------------
INTRODUCTION:
-------------------------------------------------------------------------------
This project implements a three stage deffered shader pipeline. 
Stage 1 renders geometry and samples model textures.
Stage 2 performs point and ambient lighting calculations
Stage 3 does any post processing effects

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
CODE TOUR/CONTROLS
-------------------------------------------------------------------------------

Stage 1 samples model textures renders the scene geometry to the G-Buffer
* pass.vert
* pass.frag

Stage 2 renders the lighting passes and accumulates to the P-Buffer
* shade.vert
* ambient.frag
* point.frag
* diagnostic.frag

Stage 3 renders the post processing
* post.vert
* post.frag

Keyboard controls
[keyboard](https://github.com/cboots/Deferred-Shading/blob/master/base/src/main.cpp#L1178):
This is a good reference for the key mappings in the program. 
WASDQZ - Movement
X - Toggle scissor test
R - Reload shaders
1 - View depth
2 - View eye space normals
3 - View Diffuse color
4 - View eye space positions
5 - View lighting debug mode
6 - View Specular Mapping
7 - View Only Bloomed Geometry
0 - Standard view

x - Toggle Scissor Test
r - Reload Shaders
p - Print camera position to console
j - Toggle timing measurements to console (averaged since last reset)
SPACE - Reset timing averages

Shift-L - Toggle Bloom
Shift-T - Toggle Toon Shading
Shift-D - Toggle Diffuse Mapping
Shift-S - Toggle Specular Mapping
Shift-B - Toggle Bump Mapping
Shift-M - Toggle Transparency Masking

c - Reset  diffuse color to default
t - Change diffuse color to texture coordinate visualization
h - Overlay diffuse color with visualization of available textures
b - Change diffuse color to bump map visualization if available
m - Change diffuse color to white if mask texture is available

-------------------------------------------------------------------------------
Features:
-------------------------------------------------------------------------------

* Renders .obj files with support for .mtl files with
  * Diffuse Textures
  * Specular Textures
  * Height Maps (Bump Mapping)
  * Texture Masking
 
* Bloom (Not seperable, very inefficient)
![NoBloom](/renders/LampNoBloom.PNG "Without Bloom")
![Bloom](/renders/LampWithBloom.PNG "With Bloom")

* "Toon" Shading (with basic silhouetting and color quantization)
![Toon](/renders/ToonShadingNoColor.PNG "Toon Shading B/W")
![Toon](/renders/FullHallToon.PNG "Toon Shading")

* Point light sources with specular

![Point Specular](/renders/PointLightSpeculars.PNG "Point Light Speculars")
-------------------------------------------------------------------------------
SCREENSHOTS
-------------------------------------------------------------------------------

No special features enabled. Just Point lighting:
![Baseline](/renders/FullHallBaseline.PNG "Baseline")

Diffuse Texture:
![Diffuse Map](/renders/FullHallDiffuseOnly.PNG "Diffuse Texture")

Specular Texture:
![Specular Map](/renders/FullHallSpecularOnly.PNG "Specular Texture")

Bump Mapping:
![Bump Map](/renders/FullHallBumpOnly.PNG "Bump Texture")
![Bump Map](/renders/LionCloseNoBump.PNG "Without Bump")
![Bump Map](/renders/LionClose.PNG "With Bump")

Masking:
![Mask](/renders/FullHallMaskOnly.PNG "Mask Texture")
![No Mask](/renders/PlantsNoMask.PNG "No Mask")
![With Mask](/renders/PlantsWithMask.PNG "With Mask")


All Textures:
![All Textures Map](/renders/FullHallAllTextures.PNG "Specular Texture")
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
