# Raze and Plunder (RnP) and Kittens 

RnP (and Kittens) is a C++ Starcraft BWAPI Bot, based on OpprimoBot 
https://github.com/jhagelback/OpprimoBot upgraded to BWTA2 and pieces 
refactored to more modern and safer C++. 

## Features

* Razing and Plundering are work in progress.
* Kittens are TODO.

## Building

Build system was changed from VS2013 project to CMake, which in turn generates a 
VS2013 project, but also opens possibilities to reach other build systems
such as MinGW32 on Windows or Linux (compile only, requires Microsoft linker or
a Visual Studio 2013 environment).

### Dependencies

* Visual Studio 2013 Community (get at https://www.visualstudio.com/en-us/news/releasenotes/vs2013-community-vs )
* CMake 3.5 or something (get at https://cmake.org )

### Step by step Windows/VS2013

*   Run the included script `cmake-vs2013.bat`, a new directory `cmake-vs/` 
    will be created which contains the Visual Studio solution.
*   Open the solution in Visual Studio. Do not edit project settings, 
    but instead edit `CMakeLists.txt`

### Step by step GCC/MinGW or Linux 

**NOTE** this method can compile only at the moment, not link.

*   You will need to install MinGW (`mingw32` on Ubuntu or download a Windows version)
*   Linking is not working due to Microsoft libraries being not compatible with MinGW linker.

## Opprimo Readme

OpprimoBot is an AI bot for Starcraft:Broodwar. It uses the BWAPI project to communicate with the Starcraft engine. 
It can play all three races on (almost) all maps, but works best with Terrans.

Opprimobot (and its precursor BTHAI) is developed by Dr. Johan Hagelbäck, Assistant Professor in Artificial Intelligence 
at Linnaeus University, Växjö, Sweden. You can contact the author at johan.hagelback@gmail.com.

Opprimobot is free to use (MIT license) in your own research or education projects. The only requirement is that if you 
publish anything that is based on Opprimobot (or BTHAI), add the following reference:

Johan Hagelbäck. "Potential-Field Based navigation in Starcraft". In Proceedings of 2012 IEEE Conference on 
Computational Intelligence and Games (CIG), 2012
