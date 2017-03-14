# Raze and Plunder (RnP) and Kittens 

RnP (and Kittens) is a C++ Starcraft BWAPI Bot, based on OpprimoBot https://github.com/jhagelback/OpprimoBot
upgraded to BWTA2 and pieces refactored to more modern and safer C++. Build system changed from VS2013
project to CMake, which in turn generates a VS2013 project, but opens possibilities to reach other
build systems.

Main dependencies are added as sources to reduce project setup time.

## Features

* Razing and Plundering are implemented.
* Kittens are TODO.

## Building

You will need these things at hand:

* Visual Studio 2013 Community (get at https://www.visualstudio.com/en-us/news/releasenotes/vs2013-community-vs )
* CMake 3.5 or something (get at https://cmake.org )
* Boost libraries 32bit for Visual Studio 2013 (version 120) (get at http://www.boost.org )

Your actions:

* Install dependencies to some nice short directory paths, like C:/VS2013, C:/Boost, etc for your convenience.
* Modify included `CMakeLists.txt` to let it know Boost directory location
* Run the included script `cmake-vs2013.bat`, a new directory `cmake-vs/` will be created which contains
    the Visual Studio solution.

## Opprimo Readme

OpprimoBot is an AI bot for Starcraft:Broodwar. It uses the BWAPI project to communicate with the Starcraft engine. 
It can play all three races on (almost) all maps, but works best with Terrans.

Opprimobot (and its precursor BTHAI) is developed by Dr. Johan Hagelbäck, Assistant Professor in Artificial Intelligence 
at Linnaeus University, Växjö, Sweden. You can contact the author at johan.hagelback@gmail.com.

Opprimobot is free to use (MIT license) in your own research or education projects. The only requirement is that if you 
publish anything that is based on Opprimobot (or BTHAI), add the following reference:

Johan Hagelbäck. "Potential-Field Based navigation in Starcraft". In Proceedings of 2012 IEEE Conference on 
Computational Intelligence and Games (CIG), 2012
