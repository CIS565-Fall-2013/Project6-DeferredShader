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
![Image](https://raw.github.com/rarietta/Project6-DeferredShader/master/readme_imgs/toon_side_by_side2.png)

-------------------------------------------------------------------------------
Additional G-Buffer: Luminance Based Specularity
-------------------------------------------------------------------------------

For my implementation of an additional G-Buffer, I stored the luminance of each
fragment computed as:  

	L = 0.2126*color.r + 0.7152*color.g + 0.0722*color.b
	
Here is a visualization of the luminance on the cornell box scene:

![Image](https://raw.github.com/rarietta/Project6-DeferredShader/master/readme_imgs/luminance.png)

Using this luminance buffer, I computed specular lighting for the scene in
addition to the diffuse lighting calculations. I used the luminance value
for each fragment as both the specular exponent and the specular coefficient.
You can see the difference in lighting below, and notice that the specularity
is not uniform across the scene. The specular lighting on the red surface
is the least, since this surface had the lowest luminance value, while the
specularity on the white surfaces is highest, since this surface had the
highest luminance.

![Image](https://raw.github.com/rarietta/Project6-DeferredShader/master/readme_imgs/diffuse_vs_specular.png)

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
Light Bloom
-------------------------------------------------------------------------------

To achieve this effect, I once again utilized the additional Luminance G-Buffer,
this time using it as a glow for the scene (i.e. the amount of glow from each
surface fragment was dictated by its luminance value). By blurring the product
of this simple luminance value and the original color values over a Gaussian 
distribution in screen-space X and Y, I was able to achieve a bloom or glow 
value each fragment. (The algorithm usuall calls for a Gaussian blur in all
directions, but I found comparable effects by simply blurring once in the X
direction and once in the Y direction and summing the components. This ran much
faster than a standard square kernel, which would cause a timeout on the
graphics card even at small sizes. I developed this as a simplification hack
of 2-D convolution).

You can see the results below. Here is the original image without and bloom:

![Image](https://raw.github.com/rarietta/Project6-DeferredShader/master/readme_imgs/bloom0.png)

And here is the bloom image with glow values equal to luminance. Notice how
the white surfaces glow more than the green, and more so than the red:

![Image](https://raw.github.com/rarietta/Project6-DeferredShader/master/readme_imgs/bloom1.png)

To further illustrate how the bloom utilized the luminance buffer, I inverted
the luminance and calculated bloom again. Notice in this version how the surfaces
with lower luminance glow the most. Though it may seem less natural, it shows
how the shader is implemented:

![Image](https://raw.github.com/rarietta/Project6-DeferredShader/master/readme_imgs/bloom2.png)

------------------------------------------------------------------------------------
PART 3: Performance Analysis
====================================================================================

In my analysis of this project, I compared runtime FPS in Bloom shading for a traditional
square 2D kernel versus my modified 2-Axis 1D kernel. Here is what I mean by the
kernels:

![Image](https://raw.github.com/rarietta/Project6-DeferredShader/master/readme_imgs/kernel_shapes.png)

Here is the difference in runtime between mine and the original 2D kernel I
originally attempted to implement:

![Image](https://raw.github.com/rarietta/Project6-DeferredShader/master/readme_imgs/kernel_fps_chart.png)

![Image](https://raw.github.com/rarietta/Project6-DeferredShader/master/readme_imgs/kernel_fps_graph.png)

Notice how the program does not even run for a 2D kernel of size greater than
9x9. Unfortunately, the bloom effects are not even really visible until the kernel
width is greater than 20, as judged by the modified 1D kernel implementation.

------------------------------------------------------------------------------------
ACKNOWLEDGMENTS:
====================================================================================

As noted above, Horizon Based Ambient Occlusion was adapted from an NVidia
presentation found here: http://www.nvidia.com/object/siggraph-2008-HBAO.html

This project was built on a basic framework provided by Patrick Cozzi and Liam
Boone for CIS 565 at The University of Pennsylvania, Fall 2013.