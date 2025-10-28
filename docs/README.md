# Questvien

Welcome to Questvein, an immersive dungeon crawler where players explore treacherous dungeons, battle fearsome monsters, and uncover legendary treasures.

## Table of Contents

* 1.0 Game Overview
* 2.0 Core Features
* 3.0 Setup
* 4.0 Controls
* 5.0 Contact

## 1.0 Game Overview

Questvein is a single-player dungeon crawler where players navigate procedurally generated dungeons, engage in turn-based combat, and make strateic choices for survival. This game combines classic RPG and Roguelike mechanics with a unique flair for an unforgettable adventure.

## 2.0 Core Features

* Procedural Dungeons: Every playthrough offers unique layouts, ensuring endless replayability.
* Parsed Content: Use the built-in parser to create and add your own monsters and items, tailoring the game world to your imagination.
* Deep Combat System: Engage in turn-based combat with a variety of enemies, each with unique stats and abilities.
* Inventory Management: Collect and manage weapons, armor, potions, and artifacts to suit your own personal playstyle.

## 3.0 Setup

### 3.1 System Requirements

* OS: Linux or maxOS

### 3.2 Installation

1. Clone the Repository:
```
git clone https://github.com/sethclover/Questvein.git
```
2. Move to the New Directory:
```
cd Questvein
```

### 3.3 Running the Game:

1. Build the Game:
```
make
```
2. Switch directories
```
cd bin
```
Note: you must be in the `bin` directory or the object and monster description files will not be detected.

3. Run the Game:
```
./dungeon
```
or if you need help with usage:
```
./dungeon --help
```
## 4.0 Customizing the game

By default, you have an `object_desc.example.txt` and `monster_desc.example.txt` which is what is used to parse the game entities. If you want to modify this information (You should!), then just copy the file, rename it without the `.example`as follows: `object_desc.txt` and `monster_desc.txt`. Now you can add, remove, or modify game entities and that data will be parsed in place of the `.example` data automatically!

## 5.0 Controls

In order to see the full list of controls, run the game, press any key to start the game, and the use the '?' command. Navigate the command list with the up and down arrow on your keyboard.

## 6.0 Contact

Developed by Seth Clover

Email: seth.clover@gmail.com
