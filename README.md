-------------------------------------------------------------------------------
CIS565: Project 6: Deferred Shader
-------------------------------------------------------------------------------
Fall 2013
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
NOTE:
-------------------------------------------------------------------------------
This project requires an NVIDIA graphics card with CUDA capability! 
Any card with CUDA compute capability 1.1 or higher will work fine for this project.

-------------------------------------------------------------------------------
INTRODUCTION:
-------------------------------------------------------------------------------
This project uses deferred shading in OpenGL using G-Buffers.
The key advantage of deferred shading is that it makes it much easier to manage 
your shaders, especially when you have loads of materials and lights.

-------------------------------------------------------------------------------
FEATURES:
-------------------------------------------------------------------------------

- Use of G Buffers - Depth, Normal, Color, Eye space position, Shininess
- Blinn Phong lighting model
- Post process effects like bloom and toon shading
- Support for efficient rendering of multiple point lights using scissor testing in OpenGL

-------------------------------------------------------------------------------
RESULTS:
-------------------------------------------------------------------------------
*BLINN PHONG* :
![alt tag](https://raw.github.com/vimanyu/Project6-DeferredShader/master/renders/blinnPhong.png)

*TOON SHADING*:
![alt tag](https://raw.github.com/vimanyu/Project6-DeferredShader/master/renders/toonShading.png)

*BLOOM*:
![alt tag](https://raw.github.com/vimanyu/Project6-DeferredShader/master/renders/bloom.png)


-------------------------------------------------------------------------------
VIDEO
-------------------------------------------------------------------------------
The following is a video of the deferred shading in action

[![ScreenShot](https://raw.github.com/vimanyu/Project6-DeferredShader/master/renders/deferredShader_video_screenshot.png)](http://www.youtube.com/watch?v=MMZ0mmsKyqM)

-------------------------------------------------------------------------------
BUILDING AND RUNNING CODE
-------------------------------------------------------------------------------
The code has been tested on Visual Studio 2012/2010 and cuda 5.5 on a laptop with compute capability 1.0 as well as 3.0.

Keyboard bindings for interactivity:

Key|Action
---|---
'1'| View depth
'2'| View eye space normals
'3'| View diffuse Color
'4'| View eye space positions
'5'| View lighting debug mode
'6'| View shininess
'7'| View toon shading
'8'| View bloom effect
'0'| Standard view
'r'| Reset camera
'x'| Toggle OpenGL Scissors

Apart from this, WSADQZ and mouse can be used for movement.

-------------------------------------------------------------------------------
PERFORMANCE ANALYSIS 1: Packed G Buffer
-------------------------------------------------------------------------------
There is another branch on github named "PackedGBuffer" in which I have packed my G-Buffer in a different manner.

The main difference lies in the way normals have been packed.

In the **original G-buffer**, there were separate G-Buffers for normals and shininess

Normals buffer:

Component|Data
---|---
Component 1| normal.x
Component 2| normal.y
Component 3| normal.z

Shininess buffer:

Component|Data
---|---
Component 1| shininess

In the **packed G-Buffer**, the normals G-Buffer has

Component|Data
---|---
Component 1| normal.x
Component 2| normal.y
Component 3| shininess

For this, the third component of the normal was made on the fly using thie formula,

```
normal.z = sqrt(1- normal.x*normal.x - normal.y*normal.y);
```

Model|Lighting|Without packed G-Buffer(fps)|With packed G-Buffer(fps)
---|---|---
cornell_box.obj|Blinn-Phong|64.48|66.87
cornell_box.obj|Bloom|7.29|7.28
sponza.obj|Blinn-Phong|32.54|33.60
sponza.obj|Bloom|6.52|6.54

-------------------------------------------------------------------------------
PERFORMANCE ANALYSIS 2: Effect of Bloom kernel
-------------------------------------------------------------------------------
Model: Cornell_box

Blinn-Phong without bloom: 64.48 fps

**With Bloom**

Kernel Size| FPS
---|---
100 X 100| 7.29
50 X 50| 13.10
25 X 25| 22.42
10 X 10| 35.93
5 X 5| 47.29


---
ACKNOWLEDGEMENTS
---
Referred this paper for toon shading,
http://www.cs.rutgers.edu/~decarlo/671/readings/decaudin_1996.pdf

Referred this page for bloom(glow) effect. Implemented separable convolution technique
http://http.developer.nvidia.com/GPUGems/gpugems_ch21.html

