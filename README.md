-------------------------------------------------------------------------------
CIS565: Project 6: Deferred Shader
-------------------------------------------------------------------------------
Fall 2013
-------------------------------------------------------------------------------
Yingting Xiao
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
FEATURES IMPLEMENTED:
-------------------------------------------------------------------------------

I implemented point lights, toon shading, bloom shading and screen space ambient occlusion. Since I think these effects only look good on specific scenes, I only enable one of the effects on one scene. You can switch between the effects by using a command line argument "mode=[number]". 0 is point lights, 1 is toon shading, 2 is bloom shading, 3 is screen space ambient occlusion. Below are some images:

1) Point lights

![alt tag](https://raw.github.com/YingtingXiao/Project6-DeferredShader/master/screenshots/pointlights.PNG)

2) Toon shading and bloom shading

![alt tag](https://raw.github.com/YingtingXiao/Project6-DeferredShader/master/screenshots/toon.PNG)

The sihouette is drawn where the normals of neighbor pixels differ in direction. The rest of the model's colors are posterized.

![alt tag](https://raw.github.com/YingtingXiao/Project6-DeferredShader/master/screenshots/bloom.PNG)

In this example, I have the bloom shader detect the color of the model and apply bloom effect to the red pixels and the pixels around them. For each "glow" pixel, I get the sum of the colors of its neighbors, scale it and add it to the pixel's color.

I thought about using an additional G buffer for differentiating the "glow" pixels from the rest. Then I realized that the color value should be enough. So I did not make an additional G buffer.

In comparison, the same model with diffuse shader:

![alt tag](https://raw.github.com/YingtingXiao/Project6-DeferredShader/master/screenshots/diffuse.PNG)

3) Screen space ambient occlusion

Sampling from regular grid:

![alt tag](https://raw.github.com/YingtingXiao/Project6-DeferredShader/master/screenshots/ssao_blend.PNG)

![alt tag](https://raw.github.com/YingtingXiao/Project6-DeferredShader/master/screenshots/ssao.PNG)

Sampling from poisson disk in screen space:

![alt tag](https://raw.github.com/YingtingXiao/Project6-DeferredShader/master/screenshots/ssao_poisson_blend.PNG)

![alt tag](https://raw.github.com/YingtingXiao/Project6-DeferredShader/master/screenshots/ssao_poisson.PNG)

I used the base code from last year's Project 5 and followed the instructions on that project.

-------------------------------------------------------------------------------
PERFORMANCE EVALUATION
-------------------------------------------------------------------------------

![alt tag](https://raw.github.com/YingtingXiao/Project6-DeferredShader/master/charts/bloom.PNG)

![alt tag](https://raw.github.com/YingtingXiao/Project6-DeferredShader/master/charts/ssao.PNG)

For point lights, the FPS with scissor test is 60. The FPS without scissor test is 5.5.