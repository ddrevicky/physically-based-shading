# Physically Based Shading
This is a physically based shading renderer in OpenGL/GLSL. The physically based part means that the shading model tries to preserve the incoming and outgoing energy in the environment (though it is still a rough approximation).

## Results
![](https://github.com/ddrevicky/physically-based-shading/blob/master/imgs/pbr1.png)
![](https://github.com/ddrevicky/physically-based-shading/blob/master/imgs/pbr2.png)

## Details
* The PBR pipeline uses the metallic/roughness workflow.
* The BRDF uses the Cook-Torrance specular factor and Lambertian diffuse factor. GGX is used as the normal distribution function, GGX-Smith as the geometry function and the Fresnel term is approximated using Schlick.
* The shader uses both point lights and environment lighting. The environment is loaded from an HDR map courtesy of [HDR labs](http://www.hdrlabs.com/sibl/archive.html). The implementation which runs smoothly in real time is based on Epic Games' [ideas](https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf) of split-sum approximation of the evaluated reflectance equation integral.
* If you're interested there's a detailed description of the maths behind the renderer [in this report](https://github.com/ddrevicky/physically-based-shading/blob/master/docs/Physically%20Based%20Shading.pdf).

## Acknowledgements
Joey de Vries (Learn OpenGL)
