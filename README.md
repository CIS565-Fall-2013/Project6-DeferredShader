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
- Post process effects like bloom and toon shading
- Support for efficient rendering of multiple point lights using scissor testing in OpenGL

-------------------------------------------------------------------------------
RESULTS:
-------------------------------------------------------------------------------
*COLOR INTERPOLATION* : Smooth linear interpolation of colors for each triangle based on barycentric co-ordinates
![alt tag](https://raw.github.com/vimanyu/Project4-Rasterizer/master/renders/cow_color_interp.png)

*NORMALS*: World space normals
![alt tag](https://raw.github.com/vimanyu/Project4-Rasterizer/master/renders/cow_normals.png)

*Z-DEPTH*: Z-depth in camera space
![alt tag](https://raw.github.com/vimanyu/Project4-Rasterizer/master/renders/dragon_depth.png)

*LAMBERT DIFFUSE LIGHING*: Simple lambertian shading
![alt tag](https://raw.github.com/vimanyu/Project4-Rasterizer/master/renders/dragon.png)

*BLINN PHONG LIGHTING*: Diffuse lighting with specular highlights
![alt tag](https://raw.github.com/vimanyu/Project4-Rasterizer/master/renders/buddha_spec.png)

-------------------------------------------------------------------------------
VIDEO
-------------------------------------------------------------------------------
The following is a video of the rasterizer tool in action

[![ScreenShot](https://raw.github.com/vimanyu/Project4-Rasterizer/master/doc_images/rasterizer_video_screenshot.png)](http://www.youtube.com/watch?v=s8ehsuIoL_U)

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

Apart from this WSADQZ and mouse can be used for movement.


-------------------------------------------------------------------------------
PERFORMANCE ANALYSIS
-------------------------------------------------------------------------------
There is another branch on github named "PackedGBuffer" in which I have packed my G-Buffer in a different manner.

The main difference lies in the way normals have been packed.

In the **original G-buffer**, there were separate G-Buffers for normals and shininess

Normals buffer:

Component|Data
---|---
Component 1| nomral.x
Component 2| normal.y
Componetn 3| normal.z

Shininess buffer:

Component|Data
---|---
Component 1| shininess

In the **packed G-Buffer**, the normals G-Buffer has

Component|Data
---|---
Component 1| nomral.x
Component 2| normal.y
Componetn 3| shininess

For this, the third component of the normal was made on the fly using thie formula,

```
normal.z = sqrt(1- normal.x*normal.x - normal.y*normal.y);
```


---
ACKNOWLEDGEMENTS
---
The objs for this project were downloaded from various websites online.
