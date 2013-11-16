-------------------------------------------------------------------------------
CIS565: Project 6: Deferred Shader
-------------------------------------------------------------------------------
Fall 2013
-------------------------------------------------------------------------------


-------------------------------------------------------------------------------
INTRODUCTION:
-------------------------------------------------------------------------------
In this project, the basics of deferred shading are implemented. GLSL and OpenGL code is written
to perform various tasks in a deferred lighting pipeline such as creating and writing to a G-Buffer.


The following have been implemented:
* Bloom 
* An additional G buffer slot and some effect showing it off
  To create this effect, a new G-Buffer object called Bloom was added, similar to how other g-buffer objects 
  were added ,the same procedure was followed. A bloom texture was added and it stored 0 or 1 based on the 
  material value. And then a gaussian filter was created which took the average of the neighbouring 5*5 pixels
  and averaged the value according to gaussian distribution. A new fragmnet shader called bloom.frag was created
  and all the bloom fragment operations were written inside it. 
* Point light sources
  To create point lights a number of point light locations were created and the draw light function was called. 
  Inside point.frag a condition was provided to take the point light strength and that there was linear fall-off
  in the intensity of light emitted by the point light.

The following extra features have been implemented:

* "Toon" Shading (with basic silhouetting)
  Toon shading with basic silhoutting was implemented inside post.frag as a post process effect. In order to get the 
  toon colors, the input colors were clamped to a certain range and hence resulted in tool like effect. In order to 
  get the silhouetting to work , each pixel was taken and the neighbouring 3*3 pixels were considered to find the x-gradient 
  and y-gradient , using which the slope of the edge was calculated. A particular threshold was choosen and if less than the 
  threshold, the pixel was colored black ,else it was given the toon color.

* "Mosaic" Shading
  In order to Implement mosaic shading , each pixel was considered and the goal was to assign the same color to the next 
  10*10 pixels, even though each fragment was getting its own color, it had replace that color with the first color
  of the 10*10 kernel. To do this each texture co-ordinate was considered and multiplied with screen width and height.
  then this value was divided by 10 and the floor was taken. The floored value was again divided by screen width and 
  height and multiplied again by 10, so effectively all the co-ordinated of the texture were floored to a certain value
  to put the same color to other pixels.

* "Fish eye effect"
  To create the fish eye effect, the texture co-ordinates were taken and it was projected on to a hemisphere and then 
  the field of view was considered to provide the distortion factor ,which inturn is used to calculate the new texture 
  co-ordinates to be used for that particular pixel .The following link was followed to implement the fish eye effect ,
  http://gamedev.stackexchange.com/questions/27728/how-to-implement-fisheye-effect-with-a-glsl-fragment-shader

* Emboss effect
  Not working in the current scene, but the code is written for the Emboss effect. Emboss is a popular effect in 
  video editing and it compares the color of the neighbouring pixel and difference in coor is added with a grey 
  scale value which inturn provides a depth like feature. In order for this effect to work we need many colors like 
  in a photograph, but since the diffuse colors remain very close to each other , the subtraction between neighbouring 
  pixels makes the image completely gray due to lack of variations in pixels. 

-------------------------------------------------------------------------------
VIDEO
-------------------------------------------------------------------------------
[Deferred Shading Video](http://www.youtube.com/watch?v=KTZC6z9kEW0&feature=youtu.be)


-------------------------------------------------------------------------------
RENDERS
-------------------------------------------------------------------------------

With diffuse lighting 
![alt tag](https://raw.github.com/vivreddy/Project6-DeferredShader/master/base/Renders/pointlights.png)


With Bloom effect applied
![alt tag](https://raw.github.com/vivreddy/Project6-DeferredShader/master/base/Renders/bloom.png)

With toon shading applied
![alt tag](https://raw.github.com/vivreddy/Project6-DeferredShader/master/base/Renders/toon2.png)

With toon shading applied on Sponza scene
![alt tag](https://raw.github.com/vivreddy/Project6-DeferredShader/master/base/Renders/toon.png)

With mozaic shading applied
![alt tag](https://raw.github.com/vivreddy/Project6-DeferredShader/master/base/Renders/mosaic.png)

With fish eye effect applied
![alt tag](https://raw.github.com/vivreddy/Project6-DeferredShader/master/base/Renders/fisheye2.png)

With fish eye effect applied on Sponza scene
![alt tag](https://raw.github.com/vivreddy/Project6-DeferredShader/master/base/Renders/fisheye.png)

-------------------------------------------------------------------------------
PERFORMANCE EVALUATION
-------------------------------------------------------------------------------
For performance evaluation of deferred shading, the frame rates for different effects were noted and
compared. As the computations in the shaders for the effects became complex, the frames rates dropped.

Here are the frame rate comparisons for different effects:

		Effect		       Frame rate
	    Basic Diffuse 	         37.22
	     With Bloom               2.69     
	    Toon Shading             36.45
	   Mosaic Shading            37.11
	   Fish eye effect           37.29
	   
In Bloom shading the as the kernel size for averaging incresed the frame rate decresed drastically because
of more computations and for toon shading , since in toon shading many comparisons in colors was involved
the frame decreased a bit.

