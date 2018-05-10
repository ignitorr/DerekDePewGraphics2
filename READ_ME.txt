Controls guide:
WASD for movement forwards/sideways
Q/E for up/down
Right click + drag with mouse for rotating camera
Zoom in/out with up/down arrows

Notes for grading:

Point light moves up/down between the barrels on the right.

Spot light moves & rotates, best seen by looking at the plane in the sky from below.

The white cube to the left of the barrels shows the Render To Texture

Cube in front of barrels shows normal mapping.

Cube behind barrels shows multi texturing. 
(stone texture from normal mapping + grass/dirt texture)

The rows of crates are done with 1 instancing call (DrawIndexedInstanced)

Penguin with HFILE above is the one loaded in with Obj2Header.

Penguin without HFILE is my own LoadOBJ function. Same goes for the big spaceship on the right.