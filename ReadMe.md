# Coordinates-to-Latents Generator for Neural Audio Autoencoders

> At the final stage of packaging up the repository, will get everything done by the end of May.  
Once ready, a release tag will be added to the repository, with the external objects, example Max patches, instructions to compile.

<img src="assets/overview.gif" width="600px"></img>

*Latent terrain* is a coordinates-to-latents mapping model for neural audio autoencoders (such as [RAVE](https://github.com/acids-ircam/RAVE)), can be used to build a mountainous and steep surface map for the autoencoder's latent space. A terrain produces continuous latent vectors in real-time, taking coordinates in the control space as inputs.  

This repository is a set of Max externals to build, visualise, and program latent terrain:

#### nn.terrain~  
 - Load, build, train, and save a terrain.  
 - Perform the coordinates-to-latents mapping. 

#### nn.terrain.encode  
 - Encode audio buffers into latent trajectories using a pre-trained audio autoencoder, to be used as training data for nn.terrain~.

#### nn.terrain.record  
 - Record latent trajectories to be used as training data for nn.terrain~.  

#### nn.terrain.gui  
 - Edit spatial trajectories to be used as training data for nn.terrain~. 
 - Visualise the terrain.
 - Create and program trajectory playbacks.

## Demos

The projection from a latent space to a latent terrain is done by pairing latent trajectories and spatial trajectories on a 2D plane (or any low-dimensional space). This pairing can be trained very quickly (~15s) using supervised machine learning, by providing examples of inputs (spatial trajectories) and their corresponding outputs (latent trajectories). 

https://github.com/user-attachments/assets/17a306d2-791a-4322-9ec6-aa788713cbac

Sound synthesising with latent terrain is similar to wave terrain synthesis, operating in the latent space of an audio autoencoder. An audio fragment can be synthesised by pathing through the terrain surface.  

https://github.com/user-attachments/assets/2da6380f-272a-40ff-a595-07f7d4008e3b

<!-- Constructing a terrain by defining the trajectories of audio fragments:  
![cppn](./assets/terrain_training_cppn_s.gif)   

It also supports the point-by-point steering approach proposed by Vigliensoni and Fiebrink (2023):    
![cppn](./assets/terrain_training_points_s.gif)    -->

<!-- *Latent Terrain* is an algorithmic approach to dissecting the latent space of a neural audio autoencoder (e.g., [RAVE](https://github.com/acids-ircam/RAVE)) into a lower-dimensional (e.g., 2D), mountainous, and steep space. The dissected structure is a streamable "terrain", which can be used to access latent vectors given coordinates in the lower-dimensional space.

This is a (work-in-progress) Max external for sound synthesising with latent terrain, as well as creating letent terrains on-the-fly, using interactive machine learning.  

The external, demo Max patches, documentation and tutorials will be released soon. -->



A presentation at the IRCAM Forum Workshops 2025 can be found in [this article](https://forum.ircam.fr/article/detail/latent-terrain-dissecting-the-latent-space-of-neural-audio-autoencoder-by-shuoyang-jasper-zheng/).  


<!-- 
https://github.com/user-attachments/assets/39dc648f-7c11-4669-895a-1f46999ddca1 -->

## Installation

### Compatibility

This external works with [nn_tilde v1.5.6 (torch v2.0.0/2.0.1)](https://github.com/acids-ircam/nn_tilde/releases/tag/v1.5.6). If you have a `nn~` built from another torch version, you might have to build this yourself. See the [Build Instructions](BuildInstructions.md) documentation.

[todo]

## Usage
Here we briefly walk through the features/functionalities, while detailed usage can be found in the `.maxhelp` help file for each object.  

### Loading a saved terrain


### Building a customised terrain

#### Terrain parameters

#### Training examples preparation  

#### Training  

#### Saving (Checkpoints)

### Visualising a terrain 

### Programming trajectory playback  

### Stylus Mode  

Set the `UI Tasks` attribute of nn.terrain.gui to the stylus mode to use it as a trackpad. If you have a tablet/stylus, it should also output the pen pressure.


https://github.com/user-attachments/assets/2dd7edea-583d-410b-8b09-7aa1eec09bfa



## TODOs   

We're currently testing the model on acids-ircam's [nn_tilde](https://github.com/acids-ircam/nn_tilde).  

- [✕︎] Load and inference scripted mapping model exported bt torchscript.   
- [✔︎] Display terrain visualisation.  
  - [✔︎] Greyscale (one-channel)   
  - [✔︎] Multi-channel (yes but no documentation atm)   
- [✔︎] Interactive training of terrain models in Max MSP.   
- [✔︎] Customised configuration of Fourier-CPPNs (Tancik et al., 2020).  
- [✕︎] Documentation, example patches, tutorials...  
- [✕︎] PureData

## Build Instructions  

See the [Build Instructions](BuildInstructions.md) documentation.

## Acknowledgements

 - Shuoyang Zheng, the author of this work, is supported by UK Research and Innovation [EP/S022694/1].

 - This is built on top of acids-ircam's [nn_tilde](https://github.com/acids-ircam/nn_tilde), with a lot of reused code including the cmakelists templates, `backend.cpp`, `circular_buffer.h`, and the model performing loop in `nn.terrain_tilde.cpp`.  
 - Caillon, A., Esling, P., 2022. Streamable Neural Audio Synthesis With Non-Causal Convolutions. https://doi.org/10.48550/arXiv.2204.07064  
 - Tancik, M., Srinivasan, P.P., Mildenhall, B., Fridovich-Keil, S., Raghavan, N., Singhal, U., Ramamoorthi, R., Barron, J.T., Ng, R., 2020. Fourier Features Let Networks Learn High Frequency Functions in Low Dimensional Domains. NeurIPS.  
 - Vigliensoni, G., Fiebrink, R., 2023. Steering latent audio models through interactive machine learning, in: In Proceedings of the 14th International Conference on Computational Creativity.  

