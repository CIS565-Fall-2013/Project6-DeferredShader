------------------------------------------------------------------------------------
Project 6: Deferred Shader
====================================================================================
Ricky Arietta Fall 2013
-------------------------------------------------------------------------------

![Image](https://raw.github.com/rarietta/Project6-DeferredShader/master/readme_imgs/header.png)

------------------------------------------------------------------------------------
INTRODUCTION:
====================================================================================

This project implements a deferred shader using OpenGL/GLSL. Scene attributes
are rendered out independently to various G-Buffers (i.e. Z-Depth, Normals,
Camera Space Positions, Color, Ambient Occlusion Maps) and then utilized for
post-process lighting and rendering calculations. In this project, I explore
how to use these G-Buffers for rendering multiple point light sources, as well
as calculating screen space ambient occlusion and rendering the scene using
a toon shader (or "cel shader"). 

------------------------------------------------------------------------------------
PART 1: Required Features
====================================================================================

-------------------------------------------------------------------------------
Multiple Point Lighting
-------------------------------------------------------------------------------

By pulling data from the normal, color, and position buffers, rendering point
light sources becomes a highly parallel and possible in the fragment shader. For
each light, we define an area of influence based on its strength and perform
a scissor test, thereby rendering a screen quad covering only that area of
influence. Within each light render, we compute the distance and direction from
the light to the fragment (from the position buffer) and calculate a diffuse
coefficient (using the normal and color data) and color the fragment appropriately.  

In the images below, I have set up a grid of point lights in the scene, evenly
spaced within the bounds of the Cornell box. Their influence can be seen in the
final render.

![Image](https://raw.github.com/rarietta/Project6-DeferredShader/master/readme_imgs/box_notoon1.png)

-------------------------------------------------------------------------------
Toon Shading
-------------------------------------------------------------------------------

To implement toon shading, I simply analyzed the normals of each fragment
in relation to its cardinal neighbors. If the angle between the normals was
great enough, I assumed this to be an edge and rendered it as a black line.
Otherwise, I discretized the luminance of the fragment to one of five
predefined bucket values. The results are below.  

![Image](https://raw.github.com/rarietta/Project6-DeferredShader/master/readme_imgs/toon_side_by_side.png)

-------------------------------------------------------------------------------
Additional G-Buffer
-------------------------------------------------------------------------------

![Image](https://raw.github.com/rarietta/Project6-DeferredShader/master/readme_imgs/...png)

------------------------------------------------------------------------------------
PART 2: Additional Features
====================================================================================

-------------------------------------------------------------------------------
Screen Space Ambient Occlusion
-------------------------------------------------------------------------------

![Image](https://raw.github.com/rarietta/Project6-DeferredShader/master/readme_imgs/AOpass.png)

To compute screen space ambient occlusion, I implemented a method described by 
NVidia (here: http://www.nvidia.com/object/siggraph-2008-HBAO.html) called
Horizon Based Ambient Occlusion. This method finds the horizon angle from each
fragment to its neighbors along a set of sampling axes (in my implementation
I simply used the 4 axes in X, Y, -X, and -Y and sampled 6 fragments in each
direction). These sampled angles are used to compute the occlusion value of any one
fragment and multiplied with the ambient light in a scene. You can see the
difference between the two sets of images below: the ones on the left were
rendered with no occlusion, while the ones on the right include HBAO.

![Image](https://raw.github.com/rarietta/Project6-DeferredShader/master/readme_imgs/occlusion_demo.png)

-------------------------------------------------------------------------------
Who Knows What This Will Be
-------------------------------------------------------------------------------

![Image](https://raw.github.com/rarietta/Project6-DeferredShader/master/readme_imgs/...png)

------------------------------------------------------------------------------------
PART 3: Performance Analysis
====================================================================================



------------------------------------------------------------------------------------
ACKNOWLEDGMENTS:
====================================================================================

This project was built on a basic framework provided by Patrick Cozzi and Liam
Boone for CIS 565 at The University of Pennsylvania, Fall 2013.