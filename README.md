-------------------------------------------------------------------------------
Deferred Shader
-------------------------------------------------------------------------------
Fall 2013
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
REQUIREMENTS:
-------------------------------------------------------------------------------

In this project, you are given code for:
* Loading .obj files
* Rendering to a minimal G buffer:
  * Depth
  * Normal
  * Color
  * Eye space position
* Rendering simple ambient and directional lighting to texture
* Example post process shader to add a vignette

You are required to implement:
* Either of the following effects
  * Bloom (feel free to use [GPU Gems](http://http.developer.nvidia.com/GPUGems/gpugems_ch21.html) as a rough guide)
  * "Toon" Shading (with basic silhouetting)
* Point light sources
* An additional G buffer slot and some effect showing it off

**NOTE**: Implementing separable convolution will require another link in your pipeline and will count as an extra feature if you do performance analysis with a standard one-pass 2D convolution. The overhead of rendering and reading from a texture _may_ offset the extra computations for smaller 2D kernels.

You must implement two of the following extras:

* The effect you did not choose above
* Screen space ambient occlusion
* Compare performance to a normal forward renderer with
  * No optimizations
  * Coarse sort geometry front-to-back for early-z
  * Z-prepass for early-z
* Optimize g-buffer format, e.g., pack things together, quantize, reconstruct z from normal x and y (because it is normalized), etc.
  * Must be accompanied with a performance analysis to count
* Additional lighting and pre/post processing effects! (email first please, if they are good you may add multiple).


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

---
ACKNOWLEDGEMENTS
---
* This project makes use of [tinyobjloader](http://syoyo.github.io/tinyobjloader/) and [SOIL](http://lonesock.net/soil.html)
* References for edge detection algorithm used in toon shader:
	* [Wikipedia Canny edge detector page](http://en.wikipedia.org/wiki/Canny_edge_detector)
	* [Wikipedia Sobel operator page](http://en.wikipedia.org/wiki/Sobel_operator)
	* [Songho's page for 2D convolution](http://www.songho.ca/dsp/convolution/convolution.html)
* References for blooming effect:
    * [GPU Gem chapter](http://http.developer.nvidia.com/GPUGems/gpugems_ch21.html) 
	* [Devmaster Tutorial](http://devmaster.net/posts/3100/shader-effects-glow-and-bloom)	
	* [Non-maximal suppression](http://http.developer.nvidia.com/GPUGems2/gpugems2_chapter40.html)
* References for SSAO implementation:
	* [Devmaster Tutorial](http://devmaster.net/posts/3095/shader-effects-screen-space-ambient-occlusion)
	* [Game Dev Tutorial](http://www.gamedev.net/page/resources/_/technical/graphics-programming-and-theory/a-simple-and-practical-approach-to-ssao-r2753)
	* [Base code for SSAO from CIS565 Fall 2012](https://github.com/CIS565-Fall-2012/Project5-AdvancedGLSL)



