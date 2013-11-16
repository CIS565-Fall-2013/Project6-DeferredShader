-------------------------------------------------------------------------------
CIS565: Project 6: Deferred Shader
-------------------------------------------------------------------------------
Fall 2013
-------------------------------------------------------------------------------
Qiong Wang
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
INTRODUCTION:
-------------------------------------------------------------------------------

This is an openGL based project for CIS 565 GPU Programming. 
In this project, the basics of deferred shading were introduced and implemented through
writing codes in GLSL and OpenGL.

The deferred lighting pipeline can be the following three stages:

1. Stage 1: renders the scene geometry to the G-Buffer (pass.vert, pass.frag)

2. Stage 2: renders the lighting passes and accumulates to the P-Buffer (shade.vert, ambient.frag, point.frag, diagnostic.frag)

3. Stage 3: renders the post processing (post.vert, post.frag)

Note: this project cannot be running on ultra-book machine with Intel Graphics card, since Intel Graphics card only offer support for version 130 at most currently.

-------------------------------------------------------------------------------
FEATURES IMPLEMENTED
-------------------------------------------------------------------------------

* Bloom effect
* "Toon" Shading (with basic silhouetting)
* Point light sources
* An additional G buffer slot and some effect showing it off
* Screen space ambient occlusion (SSAO)


-------------------------------------------------------------------------------
OPERATION INSTRUCTION
-------------------------------------------------------------------------------
**Keyboard Interaction**

|          Operation        |            Function           |
|:-------------------------:|:-----------------------------:|
|          'x'              |       Toggle scissor test     |
|          'r'              |         Reload shaders        |
|          'wasdqz'         |        Movement and Zoom      |
|             '1'           |           View depth          |
|             '2'           |      View eye space normals   |
|             '3'           |        View Diffuse color     |
|             '4'           |     View eye space positions  |
|             '5'           |     View lighting debug mode  |
|             '6'           |      View result of bloom     |
|             '7'           |  View result of Toon effect   |
|             '8'           |     View result of SSAO       |
|             '0'           |         Standard view         |



-------------------------------------------------------------------------------
SCREENSHOTS OF RESULTS
-------------------------------------------------------------------------------
* Source Point Light Grid
![screenshot](https://raw.github.com/GabriellaQiong/Project6-DeferredShader/master/blueceiling.png)

* Light Display
![screenshot](https://raw.github.com/GabriellaQiong/Project6-DeferredShader/master/light_display.png)

* Bloom with add new slot to G-buffer
![screenshot](https://raw.github.com/GabriellaQiong/Project6-DeferredShader/master/bloom.png)

* Toon with Silhouetting

*Pure Toon*
![screenshot](https://raw.github.com/GabriellaQiong/Project6-DeferredShader/master/cornell_toon.png)

*Toon with Silhouetting*
![screenshot](https://raw.github.com/GabriellaQiong/Project6-DeferredShader/master/toon_silhouetting.png)

* SSAO comparison

*With SSAO*
![screenshot](https://raw.github.com/GabriellaQiong/Project6-DeferredShader/master/SSAO.png)

*Without SSAO*
![screenshot](https://raw.github.com/GabriellaQiong/Project6-DeferredShader/master/no_SSAO.png)

Detailed ones:

*With SSAO*
![screenshot](https://raw.github.com/GabriellaQiong/Project6-DeferredShader/master/SSAO1.png)

*Without SSAO*
![screenshot](https://raw.github.com/GabriellaQiong/Project6-DeferredShader/master/no_SSAO1.png)


-------------------------------------------------------------------------------
PERFORMANCE EVALUATION
-------------------------------------------------------------------------------
Here is the table for the performance evaluation when rendering the cornell_box.obj in scissor mode.

|   Feature          |  approximate fps  |
|:------------------:|:-----------------:|
|  point light grid  |       38.20       |
|      Bloom         |       12.14       |
|   Toon Shading     |       35.86       |
|      SSAO          |       37.25       |

Note: Here each time I toggled to the non-scissor-test mode,light_display.png the FPS dropped dramatically to 1.2 or less. Besides, 
when we zoom in, the FPS decreased and vice versa.

-------------------------------------------------------------------------------
REFERENCES
-------------------------------------------------------------------------------
* Deferred Shading:			http://en.wikipedia.org/wiki/Deferred_shading
* Z-buffering:				http://en.wikipedia.org/wiki/Z-buffering
* Attenuation:			        http://imdoingitwrong.wordpress.com/2011/01/31/light-attenuation/
* Bloom and glow:			http://http.developer.nvidia.com/GPUGems/gpugems_ch21.html
* Toon Shader:			        http://en.wikipedia.org/wiki/Cel_shading
* SSAO:			                https://developer.valvesoftware.com/wiki/Screen_Space_Ambient_Occlusion_(SSAO)

-------------------------------------------------------------------------------
ACKNOWLEDGEMENT
-------------------------------------------------------------------------------
Thanks a lot to Patrick and Liam for the preparation of this project. Thank you :)
