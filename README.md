# LatteUI
***It's all just tables***

## Modules

LatteUI is made of modules, that are as seperate as they can be, some are completely independant.
You are more than welcome to copy a module you need like LatteLayout and use it completely outside of 
other LatteUI stuff.

Certain bits are in C, some are in C++, and some are in Lua. 

| Module | Files | Purpose |
| -------| ---------| -------- |
| LatteLayout | `LatteLayout/LatteLayout/layout.h` `LatteLayout/LatteLayout/layout.c` | UI Layout engine, completely independant of other libraries

## Dependencies
- [nanovg](https://github.com/Progalt/nanovg) (My fork)
- [luajit](https://luajit.org)
- [sol3](https://github.com/ThePhD/sol2)
- [SDL3](https://github.com/libsdl-org/SDL.git)

## Building

Building isn't just a run CMake this time. Building is broken down in few parts. 
1. Build LuaJIT
2. Then you can build with CMake

First make sure you have the submodules. 
```
git submodule update --init --recursive
```
If you haven't got them. 
After this you need to build LuaJIT, I didn't want to try to make my own CMake for this. For details see: https://luajit.org/install.html

But the main gist of it, is run the correct script for your OS and it should build.
> It failed for me on MSVC the first time but I think this was due to permissions so re-ran and it worked fine. 

Just leave everything as is, this allows the CMake script to detect that you have built it, it is looking for certain output libs/so's from this.

You can now run CMake and it should handle the rest for you. 

```
cmake -B build
```

I'll leave it to you to run the platform dependant make stuff from here. 