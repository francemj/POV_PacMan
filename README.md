# First Person Pacman - 3GC3 Final Project

By:
- Surinder Gill - 001308896
- Matt Franceschini - 001310437
- Ren-David Dimen - 001222679
- Samuel Jackson - 001213855

#Controls
- Use mouse to change direction of view
- Use W,A,S,D to change position (Forward, Left, Backwards, Right)

#Description
This is a first person recreation of Pacman! The goal of the game is to collect all of the pac dots and power ups without getting hit by any of the ghosts. Collecting pac dots increases your score and power ups give you the ability to see through walls for a short period of time!

#Additional Features
- Lighting: Multiple light sources are used within the game (One for the entire map and another that follows the camera around). Light properties for specific objects are distinct.
- Textures: Added a texture to the entire floor of the map
- Alpha Blending: When collecting a power up the walls become semi-transparent for a short period of time
- Picking: Within the start screen we added the ability to click different menu options. The menu options are also 3D objects
- Non-Geometric Primitives: We used a bitmap to draw the map (creation of the walls and where to put them) as well as for different parts of the menu
- Advanced Camera Control: Since this is a first person game the camera is always being manipulated in where it will look as well as the position of it in relation to all other objects
- Collision Detection: Current position is always being calculated in relation to other objects to see if a collision is detected. Collisions with walls, ghosts, power ups and pac dots are always being accounted for within the game