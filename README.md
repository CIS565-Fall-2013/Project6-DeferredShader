-------------------------------------------------------------------------------
<center>Project 6: Deferred Shader
-------------------------------------------------------------------------------
<center>Fall 2013
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
<center>INTRODUCTION:
-------------------------------------------------------------------------------
In this project, I was introduced to the basics of deferred shading. I wrote GLSL and OpenGL code to perform various tasks in a deferred lighting pipeline such as creating and writing to a G-Buffer.

The features I implemented include toon shading, bloom effect, screen space ambient occlusion, and point lights.

-------------------------------------------------------------------------------
<center>RENDERS:
-------------------------------------------------------------------------------

Here are some screenshots:

<center>![diffuse](https://raw.github.com/josephto/Project6-DeferredShader/master/Renders/diffuse.png "screenshots")

diffuse with point light sources 

<center>![bloom](https://raw.github.com/josephto/Project6-DeferredShader/master/Renders/bloom.png "screenshots")

bloom effect

<center>![toon](https://raw.github.com/josephto/Project6-DeferredShader/master/Renders/toon.png "screenshots")

toon effect

<center>![ssao](https://raw.github.com/josephto/Project6-DeferredShader/master/Renders/sponza_ssao.png "screenshots")

screen space ambient occlusion

<center>![ssao](https://raw.github.com/josephto/Project6-DeferredShader/master/Renders/ssao.png "screenshots")

screen space ambient occlusion

<center>![depth](https://raw.github.com/josephto/Project6-DeferredShader/master/Renders/depth.png "screenshots")

depth mode

<center>![normals](https://raw.github.com/josephto/Project6-DeferredShader/master/Renders/normals.png "screenshots")

normals mode

<center>![color](https://raw.github.com/josephto/Project6-DeferredShader/master/Renders/color.png "screenshots")

color mode

<center>![position](https://raw.github.com/josephto/Project6-DeferredShader/master/Renders/position.png "screenshots")

screen space position mode

Here's a video: http://www.youtube.com/watch?v=LJ39Sb76KdU

-------------------------------------------------------------------------------
<center>PERFORMANCE REPORT:
-------------------------------------------------------------------------------

A performance analysis on how the number of point lights affects the frame rate.

Number of Lights | Frames per Second 
------------------|------------------------
1 | ~60 fps
8 | ~60 fps
27 | ~60 fps
64 | ~60 fps
125 | ~60 fps
216 | ~34 fps
343 | ~23 fps
512 | ~18 fps
729 | ~15 fps
1000 | ~13 fps
1331 | ~12 fps
1728 | ~12 fps
2197 | ~12 fps
2744 | ~12 fps

Because there's the frame rate is capped to the 60 fps refresh rate of my computer's display, it's not surprising that the first 5 entires are all around 60 fps. However, what seems to be interesting is that once the number of points lights reaches a certain number, the frame rate doesn't drop off anymore.








Despite the fact that my backface culling was a naive implementation, it still succeeded in speeding up my code. With the ability to ignore faces that weren't facing the camera, my rasterizer was able to show a rather decent speed up in the amount of time it took to compute and rasterize each frame.
