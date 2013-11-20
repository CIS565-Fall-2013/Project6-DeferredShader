#CIS 565 : Project 6 : Deferred Shader

----

##Overview

We have implemented a deferred shader that utilizes screen space techniques to enable real-time rendering with visually desirable effects.

<div align="center">
<img src="https://raw.github.com/harmoli/Project6-DeferredShader/master/renders/depth_of_field_sponza.JPG" "DOF Sponza">
<img src="https://raw.github.com/harmoli/Project6-DeferredShader/master/renders/toon_shaded_suzanne.JPG" "Toon Shaded Suzanne">
</div>
----

##Features

We have implemneted the following requirements:
* Mutliple point lights
* Bloom (light emissive materials)
* Toon Shading 

and the following extras:
* Depth of Field
* General Gaussian Blur
* Poisson disk screen space ambient occlusion

Some of the following are planned / in-progress:
* optimizing G-Buffer (condensing color formats into 32-bit uints instead of vec3/4s)

----

##Performance Analysis

Toon Shading

Optimization | FPS
---- | ----
No G-Buffer Optimizations | 16.60 
Compressed Normals | 16.98 
Compressed Normals & 8-bit precision RGBA | 17.11 
Compressed Normals & 8-bit precision RGB | 17.27 

==================

Bloom

Optimization | FPS
----- | ----
Compressed Normals | 2.29
Compressed Normals & 8-bit precision RGB | 3.57

------

##Discussion

#### Bloom (Light Emissive Objects)
In implementing light-emissive objects, we have used the "Ke" term in the .mtl file to hold the emissive color of the 
material.  Thus, we added a vec3 to the G-buffer.  In turn, in post-processing, we sample this frame buffer for the 
color, blur it and add the color to the original diffuse coloring.  Using this techinque, we can get images such 
as Suzanne with glowing red eyes even though the original diffuse material has blue eyes.

<div align="center">
<img src="https://raw.github.com/harmoli/Project6-DeferredShader/master/renders/glowing_suzanne.JPG" "Glowing Suzanne">
<img src="https://raw.github.com/harmoli/Project6-DeferredShader/master/renders/evil_suzanne.JPG" "Suzanne with Glowing Red eyes">
</div>

#### Toon Shading
We have first used the Sobel convolution for edge detection. Then we have quantized the color to yield color banding for toon shading.

#### Depth of Field
Here we have implented a reverse Z-buffer depth-of-field.  Here, we scale the strength (the standard deviation of the Gaussian distribution) linearly to the distance away from the focus plane.  

#### G-Buffer Optimizations
We have done a little bit of g-buffer optimization, namely compressing normals and color channels.  We can see from the 
numbers above that this does aid in speeding up the frame rate.  This is sensible as deferred shading has a heavy load from
writing to and from framebuffers.  Here, we are able to achieve a significant speed-up by compressing and "shaving off" 8/32-bits 
from each color/normal.  

#### Poisson-Disk Screen Space Ambient Occlusion
We have implemented screen space ambient occlusion.  This simply uses a generated set for a poisson disk and samples
along the disk, rotating the sample in a random angle.  We do not blur the AO, so there are banding artifacts.

<div align="center">
<img src="https://raw.github.com/harmoli/Project6-DeferredShader/master/renders/no_ssao_sponza.JPG" "No AO">
<img src="https://raw.github.com/harmoli/Project6-DeferredShader/master/renders/ssao_only_poisson_disk_sponza.JPG" "AO">
<img src="https://raw.github.com/harmoli/Project6-DeferredShader/master/renders/ssao_poisson_disk_sponza.JPG" "Composite">
</div>

-----

## Video

https://vimeo.com/79866921

-----

## References

[CIS 565 Fall 2012](https://github.com/CIS565-Fall-2012/Project5-AdvancedGLSL)

[GPU Gems Real-Time Glow](http://http.developer.nvidia.com/GPUGems/gpugems_ch21.html)

[GPU Gems Depth of Field](http://http.developer.nvidia.com/GPUGems/gpugems_ch23.html)


