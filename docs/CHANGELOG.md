# Changelog

Notable changes to the Dungeon Roguelike Game will be documented here.
The formatting is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/)

## [10.0.0] - 2025-5-8

### Added

- If multiple actions occur after a player's turn, they will
  have the opportunity to press 'v' and view all of the actions
  that have taken place since the last turn for the PC
- Line of sight is now obstructed by walls

### Fixed

- Long names will now be fitted to the screen size, concatenating
  the last word with ellipsis
- Stacks now display properly when the top item changes colors

## [9.5.0] - 2025-5-6

### Added

- Hit bonuses and dodge bonuses are now fully functional, so
  when a character attacks another character, it calculates
  whether the attack lands based on a base success rate of
  75%, and adjusts based on the attacker's hit bonus and the
  defender's dodge bonus
- Monster and object colors will now change in intervals
  while waiting for the PC's action

### Changed

- When a character takes damage, it appears red briefly, and when it
  dodges an attack, it flashes yellow
- When the player takes damage, the status line flashes red, and when
  the player or monster dodges an attack, it flashes yellow on the
  status line

### Fixed

- Patched the monster showing above the player when stomping
  on the boss

## [9.4.0] - 2025-5-5

### Added

- Quips now display when building or cleaning the project
- Flasks ('!') are now incorporated, which can heal (or damage)
  the player by using the 'U' command to use the item in their
  inventory
- Equipment can now have negative attributes, so be careful 
  with what you equip from now on

### Changed

- Killing a monster now is shown in green instead of white
- Dying to a monster is shown in red instead of white

### Fixed

- Stomping a monster will no longer have a chance to crash
- Smart monsters will now stay where the last seen location is
  instead of moving back and forth
- Fixed teleporting not resetting the status message

## [9.3.0] - 2025-5-5

### Added

- Light equipment now extends the view distance of the player, and the 
  special attribute stat of the Light determines how many spaces
  outward it adds (rounding the corners using Euclidean distance)

### Fixed

- Rare occurences of crashes when killing a monster have been patched

## [9.2.1] - 2025-4-28

### Fixed

- Fixed Makefile issue

## [9.2.0] - 2025-4-24

### Added

- Defense will now reduce the damage taken from a monster by a
  percentage calculated from the total defense stat

### Fixed

- Parser error that incorrectly read ability values is patched

## [9.1.0] - 2025-4-24

### Added 

- The new 'L' command can be used to look at a monster, which will
  allow the player to see the name and description of a monster in sight
- Implemented the 'E' command to show the description of an equipment
  item
- Displays the damage taken from a monster for half a second before
  the PC's turn
- Added color to some status commands (damage taken)
- Added color when the player takes damage from a monster
- Added color to the status bar denote health percentage

### Changed

- The player inventory now shows every piece of information that is
  relevant to the item, similar to the equipment menu
- Small tweaks to the appearance of the inventory menu
- Each menu (inventory, equipment, distance map, monster list, etc.)
  can now be exited with both the escape key and the key that is used
  to open the window
- Updated the message text inside the teleport feature

### Fixed

- Swapped the 'T' and 'D' commands to accurately show the tunneling and
  non-tunneling distance maps, respectively, instead of vice versa
- Slain monster names displaying incorrectly has been patched
- Fixed the queueing of inputs while the player does or receives
  damage from a monster
- Fixed a bug where the colors were changing to white when they should
  not have been

## [9.0.0] - 2025-4-23

### Added

- You can now pick up items and add them to your inventory with the
  ',' command and view your inventory with 'i'
- The player can wear items from their inventory with 'w' and take
  them off with 't' and pressing 'e' will show the player's equipment
  menu
- The command 'd' will drop an item back on the floor
- The command 'x' will permanently destroy a specified item
- The new 'I' command will show an object's description from the
  player's inventory
- New status bar showing current player health, speed, and location

### Changed

- Instead of instant death for occupying spaces, damage and health
  has been fully incorporated 
- A monster moving where a monster already is results in the monster
  being moved out of the way and switching places if necessary
- The game now ends when the player kills the boss instead of when every
  monster in the dungeon is dead
- Monster list has a green border now

### Fixed

- The player will always go first at the start of the game to prevent
  losing before their turn ever starts on an unfortunate dungeon layout
- Fixed a bug where teleporting over a monster or item either did not
  show at all or show its color properly 

## [8.2.0] - 2025-4-20

### Added

- New win and death screens featuring text displaying "you win" or
  "you died" with ASCII art font and ASCII art background behind it

## [8.1.0] - 2025-4-17

### Changed

- Moved the --printdist/-d switch that prints the tunneling distances
  into two separate command in the game, 'D' and 'T', that show the
  tunneling and non-tunneling distances maps, respectively, with
  radiating colors for coolness
- the '?' help command now shows up as a suggestion for every turn
  in order to make known when the game starts

### Fixed

- Loading from saved works correctly again instead of failing to
  instantiate the dungeon

## [8.0.0] - 2025-4-16

### Added 

- Monsters are now added to the dungeon from the parsed monster
  file and have attributes according to their descriptions
- Monsters now have color (or multiple) and have names and symbols
  instead of hex code representations of their abilities
- Monsters now have a rarity, which will determine their spawn rate
- The monster list now has more detail, including name, symbol and
  corresponding color, abilities, and location
- Monsters can now be unique, which means they will not respawn
  once they have been slain
- Objects have been added to the dungeon that will display on the
  floor and under a character, and have a symbol that corresponds
  to their type and a color according to the color(s) from the
  parsed object file
- There are 10-12 random objects added to each dungeon floor
- Objects can be artifacts, which means when they are picked up
  they will not spawn again
- A new command 'o' that shows the list of all objects in the
  dungeon along with their name, symbol and corresponding color,
  and location
- A new switch --numobj/-o that takes a positive integer and sets
  the number of items in each dungeon floor is set to that value

### Changed

- The range of numbers of monsters that will spawn in a dungeon
  has been lowered to 7-15 from 7-20 
- The --nummon/-n switch has been replaced with --nummon/-m to
  allow consistency with the new numobj/-o switch

### Fixed

- Corners in the monster list and command list were not displaying
  correctly
- Fixed a bug where every object was being parsed as an artifact (not
  everyone can be special, sadly)

### Removed

- The --typemon/-t switch for spawning a specific monster using the
  hex value of their personality, because it does not work with the
  new parsed monster types

## [7.0.0] - 2025-4-8

### Added

- Parsing for monsters and objects from their respective text files
  in the .rlg327 directory in home (same place as load/save), which
  will print the parsed contents to cout
- Something I forgot to mention for 6.0.0, color has been added to
  the dungeon, denoting fog, player FOV, etc., and improved upon
  since then

## [6.0.1] - 2025-4-2

### Fixed

- Visual bug with teleport that did not properly update the screen
  has been fixed
- Fixed possible crashes when you teleport onto and crush a poor,
  unsuspecting monster

## [6.0.0] - 2025-4-2

### Added

- Fog of war, which is on by default and toggleable with 'f', hides
  what the player can see by showing fog, unless that PC is in a 
  close radius, and shows what they remembered of the dungeon when
  they leave that area
- Teleport, used with 'g' (goto), shows a cursor of the PC's location
  and can be moved with movement keys; pressing 'g' again places the
  player there and pressing 'r' places the PC in a random location,
  even in walls

### Changed 

- Ported everything into C++ from C

## [5.1.0] - 2025-3-31

### Added

- Added a command help function with '?' that will show a list of
  possible commands for the PC, similar to the monster list
- Monsters (not telepathic) can now see the PC if they are both in the same
  connected corridor
- Monsters can now remember the last seen location of the PC and will
  go towards it if they lose sight of them
- Added the ability to quit with 'Q' in automatic movement mode

### Fixed

- Fixed an issue where using stairs to generate a new dungeon could
  cause the PC to spawn in a room with monsters, and possibly die
  immediately (skill issue) 
- Basic performance optimizations

## [5.0.0] - 2025-3-26

### Added

- Dungeon now prints in ncurses with message and status lines above and
  below the dungeon space, respectively
- --help/-h switch has been added to display the available switches and
  their descriptions
- PC can now be controlled with movement keys
- Pressing Q will now quit the game with a goodbye message
- Pressing m will now show a list of living monsters with their relative
  positions and personalities
- Stairs are now fully functional, generating a random dungeon each time
  and spawning you on the stairs going the opposite direction

### Changed

- Optimizations for performance and error handling

### Fixed

- Incorrectly generating the player-to-monster distances during monster's
  turns
- The monster counter was incorrectly displaying how many monsters were
  still left in the dungeon

## [4.0.0] - 2025-3-5

### Added

- The PC now aimlessly wanders the dungeon in search of an exit to the
  neverending nightmare he cannot escape from
- The dungeon now spawns monsters, between 7 and 20 by default 
  unless specified by the --nummon switch, in a random location inside a 
  room that the player is not in, excluding stairs
- Monsters roam the dungeon based on their randomly generated personality
- When a character walks where another character is, the character that
  was already there now dies and prints that they have died and ends the
  game if it was the player
- Switch --montype which takes a char and will make a dungeon with only
  one monster in it, which is that type
- Exhaustive error checking and memory freeing, so now all heap blocks
  are completely freed

### Changed

- Better error checking for switches and load/save functions
- Printing the dungeon now displays the monster's location,
  marked by its hex code equivalent of the binary value of its
  attributes

## [3.1.0] - 2025-2-26

### Added

- Separate header files for each C file, excluding main, in a complete
  refactor of the header file setup, in order to manage dependencies

### Changed

- Updated .gitignore
- Updated Makefile

## [3.0.2] - 2025-2-24

### Removed

- README section from assignment 2

## [3.0.1] - 2025-2-24

### Changed

- README is now properly updated to match specifications for assignment 3

## [3.0.0] - 2025-2-24

### Added

- Distances for tunneling and non tunneling monsters are now calculated
  for generated and loaded dungeons, which are stored in the array of
  Tiles for the dungeon
- Distances for both tunneling and non tunneling monsters are now
  printed after the initial printing of the dungeon
- Fibonacci heap nodes now keep track of their position, for
  implementation of Dijkstra's algorithm
- Proper destruction of a Fibonacci heap along with all of its nodes is
  now possible

### Changed

- Inserting nodes now returns that node for Fibonacci heaps, for easier
  implementation
- Makefile has been updated to include the Fibonacci heap and path
  finding files

### Fixed

- Printing for tunneling distances now displays the character location
  and is not on one line, as intended

## [2.1.0] - 2025-2-21

### Added

- Fibonacci heap implementation (from scratch) supporting insert, getMin,
  extractMin, and decrease key, for use in Dijkstra's algorithm
- Printing for the tunneling and non tunneling distance from the PC for later
  use in displaying Dijkstra's algorithm results
- Stubs for generating distance in dungeonGeneration and printing them in
  main for future implementation purposes
- the Tiles that make up the dungeon now also keep track of their relative
  distance to the PC for tunneling and non tunneling monsters

### Fixed

- Declares struct names now so they are no longer anonymous

## [2.0.2] - 2025-2-12

- More addressing problems

## [2.0.1] - 2025-2-12

### Fixed

- Incorrect addressing for save/load

## [2.0.0] - 2025-2-12

### Added

- Two new switches for saving and loading dungeons in the .rlg327 folder
- Functionality for switches to be used together, including -hb and -ha
  for printing the hardness visuals both before and after fill, --load for
  printing hardness visuals after loading a dungeon
- Global structs containing X and Y coordinates for the PC, upward stairs, and
  downward stairs
- Error checking for malloc function calls

### Changed

- The main function now exists in its own main.c file instead of within
  dungeonGeneration.c
- Main in dungeonGeneration.c is now called fillDungeon, which builds
  rooms, corridors, and stairs, and places the player
- Makefile is updated to include the new saveLoad.c file for saving and
  loading dungeons

### Removed

- Unnecessary passing of external variables into functions

## [1.3.0] - 2025-2-11

### Added

- Player is now placed in a random location in a random room excluding
  positions that are upward or downward stairs

### Changed

- The switch -h for printing the hardness of the dungeon has been
  replaced with -hb and -ha for printing the hardness before the floors
  and corridors are added and printing after the hardness has been added,
  respectively
- Instead of corridors connecting at the center point of a room, it
  connects to a random point in the room that is not on the edge of the
  room
- The print function for hardness acts displays differently, as it now 
  prints floor as empty space, max-hardness rock as '#', and lowest to 
  highest hardness rock using '.', ';', '*', '%', respectively  

### Fixed

- Spelling and grammar in the changelog

## [1.2.0] - 2025-2-10

### Added

- Perlin gradient noise for hardness determination, which
  defines a grid of gradient vectors, computes the dot product,
  and then interpolates their values
- Definitions for border is now defined in the header file
- Switch operation using -h for printing an additional board for
  hardness values that shows a bolder character for a higher
  hardness value and vice versa

### Changed

- The 2d dungeon array is now a Tile struct instead of char array,
  and now contains a type of symbol and a hardness value from 0-255
- buildPerimeter has been replaced with initDungeon and now sets the
  hardness of the board based on the Perlin noise algorithm and then
  sets the type as rock and the edges as max-hardness border rock
- Instead of length for X and width for Y, now fits the convention of
  using width for X and height for Y values
- Makefile updated to include perlin.c

## [1.1.0] - 2025-2-7

### Added

- Header file containing definitions and structs, the global dungeon,
  and methods

### Changed

- Makefile now uses CC and CFLAGS instead of hardcoding
- Makefile was updated to check if the header file has changed during make
- Random room count has been updated from 6-8 to 7-12 for more filled
  space and variability

### Fixed

- Make clean works properly to remove output files

## [1.0.1] - 2025-2-7

### Added

- Git Repo is for the project has been made, link below
- Heading in README for which assignment it refers to

## [1.0.0] - 2025-2-4

### Added

- Created README
- Created Makefile

### Changed

- Logic for building corridors now has an 80% chance of building in the X
  direction and a 20% chance to build in the Y direction to increase
  the number of iterations it takes for the Y's to line up

## [0.2.0] - 2025-2-3

### Added

- Enums for floor, corridor, upward and downward stairs, and rock
- Print statements for failure to allocate memory or build rooms

### Fixed

- Rooms incorrectly checking for overlapping rooms, resulting in large
  rooms completely enveloping smaller rooms, making them practically
  invisible

## [0.1.0] - 2025-2-3

### Added

- Building the perimeter for the dungeon (80x21)
- Generating between 6 and 8 randomly sized and located
  rooms to be placed in the dungeon
- Connecting each room with a random other room with corridors
- Building upward and downward stairs in two different rooms
- Printing the dungeon

https://github.com/sethclover/dungeonGame
