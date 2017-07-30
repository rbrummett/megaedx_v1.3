MMX Editor has been edited and improved by several members. Contributions at the end of the file.

How to use
-----------
MMXE works best as a level editor, but you can also add/remove enemy sprites and items, change the checkpoint locations, edit the background, scenes, and layout of a level.

Keyboard Shortcuts
------------------
Change Level (page-up/page-down)
Point (o/p)
Object (q/w)
Tile (a/s)
Palette (z/x)
End Emulation (esc)
Toggle Sprites (t)

Palette Editor
---------------
Shows the current palette for the screen. (Not every palette color is editable).

Tileset Editor
--------------
Edit 1x1 pixels for a tile using several pre-loaded palettes for the level.

Tile Mapping Editor
-------------------
Copy&paste or delete a tile here. The tileset editor is loaded congruently.

Block Structure Editor
----------------------
Proper palette is loaded for the current scene, with corresponding block numbers. A block consists of 4 tiles. They can be switched and swapped here by changing the values.

Scene Structure Editor
------------------------
Scenes consist of larger numbers of blocks. The viewable area in emulation/gameplay is 4 total scenes. Use this to quickly change the graphics for a level. Every scene and graphic from the entire level is viewable here in the right window.

Layout Structure Editor
-----------------------
This window displays even larger blocks of graphics, including those not viewable to the player.

So from small to large we have:
Tile -> Map -> Block -> Scene -> Layout

Event Editor 
--------------
Use this to view collision events throughout the level. This will latch onto important objects such as items (heart tanks, sub tanks, capsules, etc.), enemy sprites, checkpoints, boss doors, bosses, and other important events not viewable to the user. It's important to note that all values in these input boxes are in hexadecimal format, NOT decimal. (Ex: a value that is displayed as 15, is actually 0x15, or 21 in a base-10 numbering system.) There are great free tools and calculators online that will help with hex arithmetic and conversion. Most calculators installed on Windows OS even have programs pre-installed (just switch to scientific mode).

What each input field means:

Prev Event/Next Event buttons?
Scroll through collision events

Type?
Type=0 -> items,boss doors,some enemies
Type=1 -> events such as cut scenes and text screens from in-game characters
Type=2 -> pre-loaded palettes, and enemy sprites, checkpoints
Type=3 -> mainly enemies and capsules

X-pos?
X-coordinate on the screen. Change this value to move object along X-axis.

Y-pos?
Y-coordinate on the screen. Change this value to move object along Y-axis.

EventId/EventSubId?
Unique values for specific objects. Change this value if you would like to load a different enemy, or object. (subid is not as important as id)

LockType/LockId/LockSubId?
Check one of these boxes to only lock onto specific types of objects. (For example, if you only want to view enemy sprites check LockType after locking onto an object of type=3. This will skip over non-enemies. You could also use this to lock onto items for X. Just check LockType when type=0. To only view specific enemies check LockType when type=3 and EventId=15 (to only see Spiky)).

GfxNum?
During a pre-loading event (type=2), shows most objects loaded into the VRAM.

GfxId?
Unique id for an object.

VRAM Slot?
VRAM stands for Video RAM. It's where graphics are temporarily stored in memory so they can be accessed quickly during gameplay. Superfamicom Wiki goes into great detail explaining this. You may need to change this address when switching enemy or object sprites. Using an emulator/debugger (such as Bsnes) will be able to help you understand these values and addresses better. Don't change this value less than 1000 or it will screw up X's sprite.

Palette Offset?
Specific palette (colors) used for item/sprite/layout, etc.

Palette Slot?
Similar to VRAM, except stores color addresses not graphics. A palette consists of up to 16 different colors in BGR value (SNES specific color format).

Property Editor
----------------
Displayed HP/damage values for certain enemies. Disabled as of version 1.1

Known Issues
-------------
Some reported issues with Rom Expansion.
Undo function and tilescreen editor are in progress
Enemy sprites don't load for Rockman&Forte
MMX3 item graphics don't load
I disabled Compress Tiles function in Bubble Crab. Saving caused issues with the font. Similar issue with Blizzard Buffalo in MMX3 (saving damaged level 6).

Contributors
------------
Xeeynamo-
https://github.com/Xeeynamo/MegaEdX/
Original coder. Got the levels to load by hacking debug mode. You were able to make minor changes to the levels here. Initial support included MMX1.
Redguyyyy-
https://github.com/RedGuyyyy/MegaEdX.git
Largest contributor. He added MMX2,X3,and Rockman&Forte functionality. He expanded the collision viewer, added sprite graphics, bosses, allowed internal emulation to run inside, plus many other features.
Pianohombre-
https://github.com/rbrummett/glowing-octo-waffle.git

Thanks to everyone on RHDN,
who may not have directly programmed actual code in the program,
but contributed with address info, assembly help, advice,
articles, and examples.

Megaman is (c) copyrighted by Capcom. No one involved with this program is affiliated with
Capcom America or Capcom Japan. It is a fan-made program to help owners of legally
obtained copies of the game to edit it.