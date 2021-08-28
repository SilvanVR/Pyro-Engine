# Pyro-Engine

My first real project, which i started in March 2016 in preparation for my bachelor thesis. 
At that time i had almost no knowledge about C++, Visual Studio and graphics programming. 
Besides i had to work with the vulkan specification directly (and this without proper preexperience) because the 1.0 specification were originally released one month after i started this project in 02/2016. This was quite a challenge and today i'd say that starting with vulkan is a bad idea. It's made for performance but only if you have the proper knowledge about it.

This is mainly the reason why the code is not very clean and changed a lot several times, resulting in a poor overall
structure. But it works and the interface is in my opinion very good but the code behind it, well if you like spaghetti, you will probably like it too :D

~40.000 lines of code
~2000h spent during a timespan of 1 year

Main features:
 - Own Math Library
 - Deferred Rendering with different Light-Volumes and Culling
 - Physically Based Rendering
 - Own GUI Rendering
 - Tons of Post-Processing (Bloom, Light-Shafts, DOA, HDR...)
 - Variance Soft-Shadows
 - Proper Resource Management (Load resources across several scenes only once)
 - Scene Management (Load different scenes, keep several scenes in RAM at the same time -> switch instantly between loaded scenes)
 - Rendering via JSON-Files + Hot-Reloading (change stuff in the .json file, see instantly the result)
 - Node-Addon: Use the renderer in javascript via Node-JS! Render via JSON requests!

Demo Video:
https://www.youtube.com/watch?v=ofnVoZo1bFw&feature=youtu.be

Some Screenshots:
![Alt text](/screens/light_shafts.png?raw=true "")
![Alt text](/screens/pbr_materials.png?raw=true "")
![Alt text](/screens/pbr.png?raw=true "")
![Alt text](/screens/sponza.png?raw=true "")
![Alt text](/screens/Box-Spawn.jpg?raw=true "")
