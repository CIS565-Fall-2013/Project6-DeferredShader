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

Scissor Test Specular Bug:
![ScissorNo](/renders/SpecularNoScissorTest.PNG "No Scissor Test")
![Scissor](/renders/SpecularWithScissorTest.PNG "With Scissor Test")

-------------------------------------------------------------------------------
PERFORMANCE EVALUATION
-------------------------------------------------------------------------------
I measured the synchronized time to complete each stage of the pipeline with various features enabled.
Two test scenes were used, a wide shot of the entire hall and a closeup of the lion sculpture (see above).
Bloom was so inefficient that I removed it from some charts for scale

Full Hall Metrics:

![TimingFH](/renders/FullHallTiming.PNG "Full Hall")
![TimingFHB](/renders/FullHallTimingBloom.PNG "Full Hall Including Bloom")

Lion Closeup Metrics:

![TimingLC](/renders/LionCloseupTiming.PNG "Lion Closeup")
![TimingLCB](/renders/LionCloseupTimingBloom.PNG "Lion Closeup Including Bloom")

Clearly the non-seperated convolution involved in the bloom effect has a huge impact on performance.
Most of the other effects had only minimal impact on performance. 
The scissor test gave a huge boost to stage 2 performance but as shown above it results in visual artifacts.
Another takeaway of this data is the second stage (lighting calculation) is the most intensive part of the process.

---
ACKNOWLEDGEMENTS
---
This project makes use of [tinyobjloader](http://syoyo.github.io/tinyobjloader/) and [SOIL](http://lonesock.net/soil.html)
