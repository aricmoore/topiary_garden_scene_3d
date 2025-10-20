# 3D Topiary Garden Scene - The Victorian Garden at Brigdemere Show Gardens in Nantwich, Cheshire

This project is a 3D-rendered garden scene created using C++, OpenGL, and GLSL shaders. It features a variety of topiary hedges, tapered cylinder trees with sphere tops, a torus-shaped hedge, and a flat plane as the ground. The scene supports perspective and orthographic camera views, texture mapping, and lighting for a more realistic appearance.

## Features

- **Topiary Trees**: Custom tapered cylinder meshes with sphere tips.
- **Hedges**: Rectangular and X-shaped hedges using scaled box meshes.
- **Torus Hedge**: Ring-shaped hedge surrounding the main bushes.
- **Camera**: Supports both perspective and orthographic projections.
- **Lighting**: Coloured light sources for realistic shading and softer shadows.
- **Texture Mapping**: Leaves and other textures applied to meshes.

## Installation and Running

1. Clone the repository:

   ```bash
   git clone https://github.com/aricmoore/topiary_garden_scene_3d.git

## Module 8-3 Journal

### 1) How do I approach designing software?
Working through this project helped me refine my approach to software design by emphasising planning and modularity. I learnt how to break down a complex scene into manageable components such as planes, boxes, toruses, spheres, and tapered cylinders, each object type with its own unique properties and behaviours. My design process involved studying perspective and orthogonal vantage points from reference images and area maps, identifying key elements of the scene to depict visually, and deciding how each element could be represented using OpenGL primitives. The general outline of breaking down problems, planning transformations, and considering object interactions can be applied to future projects across all types of software development, but especially in graphics and simulation-based fields.

### 2) How do I approach developing programs?
While not exclusive to this particular programme, a few key strategies that I employed include incremental development, individually testing each object, and refining code iteratively. I began building the simpler elements such as cones and boxes, then layered more complex objects like the torus and tree-shaped cone, tailoring transformations and textures as I went along. As requirements grew over the term, iteration was key in development: I was able to maintain modularity and scalability through continuous testing of the placement, alignment, and interaction of objects. With each milestone, I was able to refine my workflow as I moved from static placements to dynamic transformations.

## 3) How can computer science help me in reaching my goals?
This final project was a masterclass in solving complex problems, building out software, and creating interactive visual experiences; working on computational graphics has taught be how to model real-world objects, manage transformations, and apply algorithms practically and effectively. The skills derived from this project are applicable in future education for advanced courses in fascinating computer science fields like graphics, simulations, and game development. From a high level, computer science provides the theory and technical foundation to turn ideas into functional, impactful software.