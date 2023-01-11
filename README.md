
# NavMesh Research GPP

This is my Gameplay Programming research topic

I built a system where a user can create walls, updating the navmesh that the AI uses to navigate the world.

I will expand upon the A* pathfinding project, adding a new implementation of the Ear Clipping Algorithm which will update the navmesh if need be.

The user can change the width and height of these walls and is obstructed from placing walls inside eachother or outside of bounds.


-----Implementation-------

Everytime a new wall gets created, the entire navmesh needs to be rebuilt.
All the vertices need to be renumbered otherwise the ear clipping algorithm would not work correctly.

Perhaps with another algorithm, I wouldn't have needed to do this expensive process.
## Acknowledgements

 - My main reference were the given slides in week 06.
 - My second most important guide was this video: https://www.youtube.com/watch?v=QAdfkylpYwc

## Demo
