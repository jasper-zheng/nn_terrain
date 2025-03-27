# Latent Terrain Priors for Neural Audio Autoencoders

*Latent Terrain* is an algorithmic approach to dissecting the latent space of a neural audio autoencoder into a lower-dimensional (e.g., 2D), mountainous, and steep space. The dissected structure is a streamable "terrain", which can be used to access latent vectors given coordinates in the lower-dimensional space.

This is a (work-in-progress) Max external for sound synthesising with latent terrain, as well as creating letent terrains on-the-fly, using interactive machine learning.


Constructing a terrain by defining the trajectories of audio fragments:  
![cppn](./assets/terrain_training_cppn_s.gif)  

It also support the point-by-point steering approach proposed by Vigliensoni and Fiebrink (2023):    
![cppn](./assets/terrain_training_points_s.gif)  


## Progress   

We're currently testing the model on acids-ircam's [RAVE](https://github.com/acids-ircam/nn_tilde).  

- [✔︎] Load and inference scripted mapping model exported bt torchscript in realtime.   
- [✕︎] Display terrain visualisation.  
  - [✔︎] Greyscale (one-channel)  
  - [✕︎] Multi-channel  
- [✔︎] Interactive training of terrain models in Max MSP.   
- [✕︎] Customised configuration of Fourier-CPPNs (Tancik et al., 2020).  
- [✕︎] Documentation, example patches, tutorials...  

## Compile

To compile, you will need: 
 - Min-DevKit, available from the Package Manager inside of Max or [directly from Github](https://github.com/Cycling74/min-devkit).   
 - [LibTorch](https://pytorch.org/cppdocs/installing.html).  



## Acknowledgements

 - This work is supported by UK Research and Innovation [EP/S022694/1].

 - This is built on top of acids-ircam's [nn_tilde](https://github.com/acids-ircam/nn_tilde).  
 - Caillon, A., Esling, P., 2022. Streamable Neural Audio Synthesis With Non-Causal Convolutions. https://doi.org/10.48550/arXiv.2204.07064  
 - Tancik, M., Srinivasan, P.P., Mildenhall, B., Fridovich-Keil, S., Raghavan, N., Singhal, U., Ramamoorthi, R., Barron, J.T., Ng, R., 2020. Fourier Features Let Networks Learn High Frequency Functions in Low Dimensional Domains. NeurIPS.  
 - Vigliensoni, G., Fiebrink, R., 2023. Steering latent audio models through interactive machine learning, in: In Proceedings of the 14th International Conference on Computational Creativity.  

