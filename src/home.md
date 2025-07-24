---
title: Latent Terrain Synthesis
description: Building new musical instruments that compose and interact with AI audio generators.
layout: libdoc_page.liquid
permalink: index.html
date: git Last Modified
tocEnabled: false
---
{% alert 'MaxMSP externals available [here](https://github.com/jasper-zheng/nn_terrain/releases/tag/v1.5.6), example MaxMSP/Max4Live devices will be available soon.', 'warning', 'News' %}

## Overview   
Latent terrain is a coordinates-to-latents mapping model for neural audio autoencoders (such as [RAVE](https://github.com/acids-ircam/RAVE), [Music2Latent](https://github.com/SonyCSLParis/music2latent)). It can be used to build a surface map for the autoencoder's latent space. It aims to open up the creative possibilities of **latent space navigation**.  

A terrain produces continuous latent vectors in real-time, taking coordinates in the control space as inputs.  

An example **latent space walk** with Music2Latent:

<!-- <video src="../assets/walk.mp4"></video> -->
<video controls="" loop="" playsinline="" aria-labelledby="video-label" src="./assets/terrain-walk.mp4" width="95%"></video>


## Audio autoencoders 

In theory, latent terrain can work with any [audio autoencoder](https://github.com/acids-ircam/creative_ml/blob/main/08_variational_ae_flows.pdf) as long as it offers latent variables. However, only a limited number of them have been implemented for MaxMSP, and we have only tested the following models:  

* [RAVE](https://github.com/acids-ircam/RAVE) <br>Realtime Audio Variational autoEncoder for fast and high-quality neural audio synthesis, by Antoine Caillon and Philippe Esling.   
* [Music2Latent-Scripted](https://github.com/jasper-zheng/music2latent-scripted) <br>Music2Latent is a Consistency Autoencoder to encode and decode audio samples, by Marco Pasini, Stefan Lattner, and George Fazekas. We're using a scripted folk of the original repository.   

We plan to test the following model in the future:  
* [FlowDec](https://github.com/facebookresearch/FlowDec)
* [vschaos2](https://github.com/acids-ircam/vschaos2)

## Get in touch

LibDoc’s website contains a comprehensive documentation about how to install, configure and :

* [LibDoc’s homepage](https://eleventy-libdoc.netlify.app) <br>The website containing LibDocs’s presentation and comprehensive documentation.
    * [Front matter](https://eleventy-libdoc.netlify.app/front-matter/) <br>Documentation of all front matter settings related to a LibDoc page.
    * [Primary navigation](https://eleventy-libdoc.netlify.app/primary-navigation/) <br>Detailed features of LibDoc’s primary navigation.
    * [SEO](https://eleventy-libdoc.netlify.app/configuration/seo/) <br>How LibDoc’s configuration and pages parameters are applied for Search Engine Optimization.
    * [Credits](https://eleventy-libdoc.netlify.app/configuration/credits/) LibDoc could not work without these resources.