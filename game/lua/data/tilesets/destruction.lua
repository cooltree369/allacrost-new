local ns = {};
setmetatable(ns, {__index = _G});
destruction = ns;
setfenv(1, ns);

tileset_name = "Destruction"
image = "img/tilesets/destruction.png"

collisions = {}
collisions[0] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[1] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[2] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[3] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[4] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[5] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[6] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[7] = { 3, 0, 1, 2, 5, 15, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[8] = { 12, 15, 13, 10, 15, 15, 15, 15, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[9] = { 15, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[12] = { 5, 10, 5, 10, 5, 10, 5, 10, 5, 10, 0, 0, 0, 0, 0, 0 }
collisions[13] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 0 }
collisions[14] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
collisions[15] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }

animations = {}
-- 2x2 5 frames/1 second: tall fire located on the left side and second from bottom
animations[0] = { 176, 200, 178, 200, 180, 200, 182, 200, 184, 200 }
animations[1] = { 177, 200, 179, 200, 181, 200, 183, 200, 185, 200 }
animations[2] = { 192, 200, 194, 200, 196, 200, 198, 200, 200, 200 }
animations[3] = { 193, 200, 195, 200, 197, 200, 199, 200, 201, 200 }
-- 2x3 5 frames/1 second: large explosion located on bottom left
animations[4] = { 208, 200, 210, 200, 212, 200, 214, 200, 216, 200 }
animations[5] = { 209, 200, 211, 200, 213, 200, 215, 200, 217, 200 }
animations[6] = { 224, 200, 226, 200, 228, 200, 230, 200, 232, 200 }
animations[7] = { 225, 200, 227, 200, 229, 200, 231, 200, 233, 200 }
animations[8] = { 240, 200, 242, 200, 244, 200, 246, 200, 248, 200 }
animations[9] = { 241, 200, 243, 200, 245, 200, 247, 200, 249, 200 }
-- 1x1 5 frames/1 second: small fire located on right near the bottom
animations[10] = { 218, 200, 219, 200, 220, 200, 221, 200, 222, 200 }
-- 1x2 6 frames/1 second: small explosion location on bottom right
animations[11] = { 234, 200, 235, 200, 236, 200, 237, 200, 238, 200, 239, 200 }
animations[12] = { 250, 200, 251, 200, 252, 200, 253, 200, 254, 200, 255, 200 }

