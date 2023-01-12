
# NavMesh Research GPP

This is my Gameplay Programming research topic

I built a system where a user can create walls, updating the navmesh that the AI uses to navigate the world.

I will expand upon the A* pathfinding project, adding a new implementation of the Ear Clipping Algorithm which will update the navmesh if need be.

The user can change the width and height of these walls and is obstructed from placing walls inside eachother or outside of bounds.


-----Implementation-------

Everytime a new wall gets created, the entire navmesh needs to be rebuilt.
All the vertices need to be renumbered otherwise the ear clipping algorithm would not work correctly.
Sadly, there are a few other occasional bugs that I did not have the time to fix.

Perhaps with another algorithm, I wouldn't have needed to do this expensive process.
With more time I could've also polished the project further, removing the last few bugs.

## Acknowledgements

 - My main reference were the given slides in week 06.
 - My second most important guide was this video: https://www.youtube.com/watch?v=QAdfkylpYwc

## Demo

[![Research](https://user-images.githubusercontent.com/77245042/211941971-503551ad-0179-476a-bd94-87deb7251158.gif)](https://user-images.githubusercontent.com/77245042/212044674-55031370-022a-4a9d-954a-6b01d8aca8f6.mp4)
