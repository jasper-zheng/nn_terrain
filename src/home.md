---
title: Latent Terrain Synthesis
description: Building new musical instruments that compose and interact with AI audio generators.
layout: libdoc_page.liquid
permalink: index.html
date: git Last Modified
tocEnabled: false
---
{% alert 'MaxMSP externals available [here](https://github.com/jasper-zheng/nn_terrain/releases/tag/v1.5.6), example MaxMSP/Max4Live devices will be available soon.', 'warning', 'News' %}

## Welcome   
Latent terrain is a coordinates-to-latents mapping model for neural audio autoencoders (such as [RAVE](https://github.com/acids-ircam/RAVE), [Music2Latent](https://github.com/SonyCSLParis/music2latent)). A terrain is a surface map for the autoencoder's latent space, taking coordinates in a control space as inputs, and producing continuous latent vectors in real-time.

Latent terrain aims to open up the creative possibilities of **latent space navigation**, allowing one to adapt an autoencoder to easier-to-navigate interfaces (such as gestural controllers, stylus and tablets, XY-pads, and more), and build new musical instruments that compose and interact with AI audio generators.


An example **latent space walk** with Music2Latent:

<video controls="" loop="" playsinline="" aria-labelledby="video-label" src="./assets/terrain-walk.mp4" width="95%"></video>

### Example applications

* [Steering a neural audio autoencoder](#) (tutorial coming soon).
* [Building 1D/2D latent granular synthesiser](#) (tutorial coming soon).
* Latent looping device.

## Supported autoencoders 

Latent terrain can work with any [audio autoencoder](https://github.com/acids-ircam/creative_ml/blob/main/08_variational_ae_flows.pdf) as long as it offers latent variables. However, only a limited number of them have been implemented for MaxMSP, and we have only tested the following models:  

* [RAVE](https://github.com/acids-ircam/RAVE) <br>Realtime Audio Variational autoEncoder for fast and high-quality neural audio synthesis, by Antoine Caillon and Philippe Esling.   
* [Music2Latent-Scripted](https://github.com/jasper-zheng/music2latent-scripted) <br>Music2Latent is a Consistency Autoencoder to encode and decode audio samples, by Marco Pasini, Stefan Lattner, and George Fazekas. We're using a scripted fork of the original repository.   

We plan to test the following model in the future:  
* [FlowDec](https://github.com/facebookresearch/FlowDec)
* [vschaos2](https://github.com/acids-ircam/vschaos2)

## Get started 

* [Download and Installation](/installation)
* [Instructions](/instructions)
* [Pre-Trained Terrains (Presets)](/pre-trained)
* [Compiling from Source](/compile)

## Get in touch

Hi, this is Shuoyang (Jasper). `nn.terrain~` is part of my ongoing PhD work on **Discovering Musical Affordances in Neural Audio Synthesis**, supervised by Anna Xambó Sedó and Nick Bryan-Kinns, and part of the work has been (will be) on putting AI audio generators into the hands of composers/musicians.

Therefore, I would love to have you involved in it - if you have any feedback, a features request, a demo / a device / or anything made with nn.terrain, I would love to hear. If you would like to collaborate on anything, please leave a message in this [feedback form](https://forms.office.com/e/EJ4WHfru1A).  


## Acknowledgements

Shuoyang Zheng, the author of this work, is supported by the UKRI Centre for Doctoral Training in Artificial Intelligence and Music [EP/S022694/1].