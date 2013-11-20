CIS 565 : Project 6 : Deferred Shader

----

##Overview

We have implemented a deferred shader that utilizes screen space techniques to enable real-time rendering with visually desirable effects.

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

#### Toon Shading
We have first used the Sobel convolution for edge detection. Then we have quantized the color to yield color banding for toon shading.

#### Depth of Field
Here we have implented a reverse Z-buffer depth-of-field.  Here, we scale the strength (the standard deviation of the Gaussian distribution) linearly to the distance away from the focus plane.  

-----

## Video

-----

## References


