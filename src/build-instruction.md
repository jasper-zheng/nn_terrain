---
title: Compile from Source
description: Building nn.terrain~ externals from the source code.
layout: libdoc_page.liquid
permalink: compile/index.html
eleventyNavigation:
    key: Compile from Source
date: 2025-05-18

---

If the externals have trouble opening in Max, or doesn't work correctly with nn_tilde, you might want to build the externals yourself:

## Prerequisites

**MacOS:** 
 - Xcode 11 or 12 (you can get from the App Store for free).
 - Download LibTorch [here](https://pytorch.org/get-started/locally/) and unzip it to a known directory. LibTorch's torch version should be the same as nn_tilde.
 - Install a recent version of [CMake](https://cmake.org/download/) (version 3.19 or higher).

**Windows:**
 - Download LibTorch [here](https://pytorch.org/get-started/locally/) and unzip it to a known directory. LibTorch's torch version should be the same as nn_tilde.  
   - If you would like to enable GPU training/inference, you'll need to select the CUDA version of LibTorch, and have the corresponding [CUDA ToolKit](https://developer.nvidia.com/cuda-toolkit).  
 - Install a recent version of [CMake](https://cmake.org/download/) (version 3.19 or higher).  

## Build on MacOS

Recursively clone this repository into **Max's Packages folder**. Terminal command:

```
git clone https://github.com/jasper-zheng/nn_terrain.git --recursive
```

In Terminal, cd into the `nn_terrain` folder you cloned, and make a new folder named `build`. and cd into that folder:

```
cd nn_terrain
mkdir build
cd build
```

Run the command below to generate an Xcode project, replace `path/to/libtorch` to the libtorch folder you've downloaded:

```
cmake ../src/ -G Xcode -DCMAKE_PREFIX_PATH=/path/to/libtorch -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64  
```

An Xcode project will be created in this `build` folder, you can either open the Xcode project and build it from there, or build by running the command below:

```
cmake --build .
```

The `.mxo` files will be created in the `src/externals` folder, move them `~/Documents/Max 9/Packages/nn_terrain/externals/`


Additionally, taken from [min-devkit](https://github.com/Cycling74/min-devkit/tree/main):

If you are running on a Mac with Apple Silicon, you might see an error cannot be loaded due to system security policy when loading your externals in Max. To resolve this, you can ad-hoc codesign your external with codesign `--force --deep`, this can be added in Xcode "Build Settings" page, in the "Signing" section, in "Other Code Signing Flags".

## Build on Windows

Recursively clone this repository into **Max's Packages folder**. Terminal command:

```
git clone https://github.com/jasper-zheng/nn_terrain.git --recurse-submodules
```

In Terminal, cd into the `nn_terrain` folder you cloned, and make a new folder named `build`. and cd into that folder:

```
cd nn_terrain
mkdir build
cd build
```

Then run the command below to generate a project buildsystem, replace `path\to\libtorch` to the libtorch folder you've downloaded, and make sure `Visual Studio 17 2022` is set to your build system generator (run `cmake --help` to get a list of available options).

```
cmake ..\src -A x64 -DCMAKE_PREFIX_PATH="path\to\libtorch" -G "Visual Studio 17 2022"
```

Having generated the projects, now you can build by opening the .sln file in Visual Studio, or build on the command line using:

```
cmake --build . --config Release
```

The externals will be created in the `src/externals` folder, move them `~/Documents/Max 9/Packages/nn_terrain/externals/`
