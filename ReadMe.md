# Latent Terrain Priors for Neural Audio Autoencoders

At the final stage of packaging up the repository, will get everything done by the end of May.  
Once ready, a release tag will be added to the repository, with the external objects, example Max patches, instructions to compile.

Constructing a terrain by defining the trajectories of audio fragments:  
![cppn](./assets/terrain_training_cppn_s.gif)   

It also supports the point-by-point steering approach proposed by Vigliensoni and Fiebrink (2023):    
![cppn](./assets/terrain_training_points_s.gif)   

*Latent Terrain* is an algorithmic approach to dissecting the latent space of a neural audio autoencoder (e.g., [RAVE](https://github.com/acids-ircam/RAVE)) into a lower-dimensional (e.g., 2D), mountainous, and steep space. The dissected structure is a streamable "terrain", which can be used to access latent vectors given coordinates in the lower-dimensional space.

This is a (work-in-progress) Max external for sound synthesising with latent terrain, as well as creating letent terrains on-the-fly, using interactive machine learning.  

The external, demo Max patches, documentation and tutorials will be released soon.

## Demos

Demo videos recorded for the IRCAM Forum Workshops 2025, see the [article](https://forum.ircam.fr/article/detail/latent-terrain-dissecting-the-latent-space-of-neural-audio-autoencoder-by-shuoyang-jasper-zheng/).  


https://github.com/user-attachments/assets/39dc648f-7c11-4669-895a-1f46999ddca1

## Compatibility

This external works with [nn_tilde v1.5.6 (torch v2.0.0)](https://github.com/acids-ircam/nn_tilde/releases/tag/v1.5.6). If you have a `nn~` built from another torch version, you might have to build this yourself.


## Progress   

We're currently testing the model on acids-ircam's [nn_tilde](https://github.com/acids-ircam/nn_tilde).  

- [✕︎] Load and inference scripted mapping model exported bt torchscript.   
- [✔︎] Display terrain visualisation.  
  - [✔︎] Greyscale (one-channel)   
  - [✔︎] Multi-channel (yes but no documentation atm)   
- [✔︎] Interactive training of terrain models in Max MSP.   
- [✔︎] Customised configuration of Fourier-CPPNs (Tancik et al., 2020).  
- [✕︎] Documentation, example patches, tutorials...  


## Build Instructions

### Prerequisites

Mac: 
 - Xcode 11 or 12 (you can get from the App Store for free).
 - Download arm64 libtorch [here](https://pytorch.org/get-started/locally/) and unzip it to a known directory.
 - Install a recent version of [CMake](https://cmake.org/download/) (version 3.19 or higher).

Windows:
 - ?

### MacOS

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

Then run the command below to generate an Xcode project, replace `path/to/libtorch` to your path to libtorch:

```
cmake ../src/ -DCMAKE_PREFIX_PATH=/path/to/libtorch -DCMAKE_BUILD_TYPE=Release -G Xcode 
```

An Xcode project in this `build` folder will be created, open it and use the GUI to build.

Add `CoreFoundation.framework` to the "Frameworks and Libraries" section before build. See below:

<img src="assets/macbuild.png" width="800px"></img>


Taken from [min-devkit](https://github.com/Cycling74/min-devkit/tree/main)

> If you are running on a Mac with Apple Silicon, you will likely see an error cannot be loaded due to system security policy when loading your externals in Max. To resolve this, you can ad-hoc codesign your external with codesign `--force --deep -s - myobject.mxo`.


## Acknowledgements

 - This work is supported by UK Research and Innovation [EP/S022694/1].

 - This is built on top of acids-ircam's [nn_tilde](https://github.com/acids-ircam/nn_tilde).  
 - Caillon, A., Esling, P., 2022. Streamable Neural Audio Synthesis With Non-Causal Convolutions. https://doi.org/10.48550/arXiv.2204.07064  
 - Tancik, M., Srinivasan, P.P., Mildenhall, B., Fridovich-Keil, S., Raghavan, N., Singhal, U., Ramamoorthi, R., Barron, J.T., Ng, R., 2020. Fourier Features Let Networks Learn High Frequency Functions in Low Dimensional Domains. NeurIPS.  
 - Vigliensoni, G., Fiebrink, R., 2023. Steering latent audio models through interactive machine learning, in: In Proceedings of the 14th International Conference on Computational Creativity.  

