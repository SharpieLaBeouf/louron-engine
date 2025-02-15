# LouronEngine

##Overview

I started Louron Engine way way back in 2020 with an idea of having a layered system where each scene or context would be a state stacked into a vector, and the back would be the one that updates and renders, etc.

This has evolved ALOT over the past years as I have strived to learn more about realtime renderings and C++. I transitioned from this system to an editor application, where everything was controlled through an application that had entity hierarchies, property panels, serialisation of scene states, etc., but all the data of the game state was held in the internal engine, the editor was simply a means of manipulating this data... Like every 'normal' game engine design.

It is still currently just a MSVC solution that utilises C++20. I will be looking to change this to a CMAKE system, but right now is not a priority to my goals/learning.

This engine utilises OpenGL 4.5 and C++20.

##Features

At this point there are an array of features utilised in this project:
- Forward Plus Tile-Basee Rendering
  - Automatic instancing of meshes
  - Octree spatial partitioning system primarily for frustum culling (StackExchangePost)[https://gamedev.stackexchange.com/questions/211647/octree-query-frustum-search-and-recursive-vector-inserts]
  - Occlusion queries and culling
  - LOD mesh grouping
  - Shadow mapping (cascaded shadow maps, point light maps and spot light maps)
  - Multi threaded geometry renderable candidate sorting
- ENTT entity component system
- Mono C# scripting integration
  - Create scripts, 
  - Set fields in editor
  - Serialise fields on entities
- NVIDIA PhysX library implementation
- ImGui for editor user interface
- Centralised asset manager system
- Project and Scene management
- Prefab serialisation and runtime instantiation
- Transform hierarchy system with parent transforms that influence child transforms
- Standard Shading Pipeline
  - PCF shadowing
  - Gamma correction
  - HDR tone mapping
  - Normal mapping
- Custom materials 
  - opaque, 
  - transparent, and 
  - transparent with writeable depth (these write depth values to FBO during transparent rendering e.g., you want transparent/discarded pixels in some areas, but opaque pixels in others)
- Custom shaders
  - Can create custom shader assets in projects with custom uniform blocks
  - Custom compute shaders with compute buffers that can be written to and read from C#
  - Custom uniform blocks for shaders through C# and material serialisation
- Basic audio playback
- Automatic instancing
- Skyboxes
- Multisampling

##Goals

There is no doubt this can make a game at its current state, but it would most likely be less popular and less performance than Shrek 2 on PS2... Here is a list of goals I would like to achieve:
- Particle systems
- Inbuilt volumetric cloud/fog rendering
- Runtime deployment including binary serialisation
- Better graphics
- Better importing options with customised import metadata
- Better memory management using custom allocators
- Caching optimisations
- Better renderer sorting algorithms for real time rendering
- Runtime assets that can be dynamically created and modified through scripting, e.g. meshes, textures, etc.
- Post processing effects such as bloom, and custom FX designed by users
- Animation system
- Thorough documentation
- CMAKE or MAKE system
- Textured lights and caustics (cookies)
- Ambient Occlusion
- Reflections (probes and screen space)
- Refraction

##Dependencies
- GLFW
- GLAD
- PhysX
- STB Image & STB Resize
- Assimp
- Mono C#
- Spdlog
- Yaml-cpp
- ESFW - file watch
- ImGui
- ImGuizmo
- Miniaudio
- ENTT
- GLM

##Acknowledgements

Many of my systems have been designed with the support of various publicly available resources sought through YouTube, GitHub, and the general web. Honourable mentions go to LearnOpenGL for OpenGL API learnings, bcrusco for their Forward Plus Render design, Cherno for engine architecture design choices, and ChatGPT for teaching me when I hit brick walls!

##License

Louron Engine is released under the MIT License.