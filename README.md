-------------------------------------------------------------------------------
CIS565: Project 6: Deferred Shader
-------------------------------------------------------------------------------
Yuqin Shao
-------------------------------------------------------------------------------


-------------------------------------------------------------------------------
Featurues:
-------------------------------------------------------------------------------
* Bloom Effects
* Toon Shading
* Point light sources
* Specular map slot added to G buffer
* Separable convolution for bloom effects as well as performance evaluation listed below.

[Note] press 6 to show bloom effects, press 7 show toon shading, press 8 show specular map

-------------------------------------------------------------------------------
Screen Shots
-------------------------------------------------------------------------------

![Alt test] (screenshots/Bloom_separable.JPG "Bloom Effects" )

![Alt test] (screenshots/ToonShadingWithSilhoutte.JPG "toon shading")

![Alt test] (screenshots/specular.JPG "specular")

![Alt test] (screenshots/specularMap.JPG "specular map")


-------------------------------------------------------------------------------
PERFORMANCE EVALUATION
-------------------------------------------------------------------------------
![Alt test] (screenshots/perform1.png "separable convolution evaluation")

![Alt test] (screenshots/perform2.png "point light radius evaluation")

---
SUBMISSION
---
As with the previous projects, you should fork this project and work inside of
your fork. Upon completion, commit your finished project back to your fork, and
make a pull request to the master repository.  You should include a README.md
file in the root directory detailing the following

* A brief description of the project and specific features you implemented
* At least one screenshot of your project running.
* A link to a video of your project running.
* Instructions for building and running your project if they differ from the
  base code.
* A performance writeup as detailed above.
* A list of all third-party code used.
* This Readme file edited as described above in the README section.

---
ACKNOWLEDGEMENTS
---
This project makes use of [tinyobjloader](http://syoyo.github.io/tinyobjloader/) and [SOIL](http://lonesock.net/soil.html)
