---
title: Pre-Trained Terrains (Presets)
description: Some already built terrains to get started with.
layout: libdoc_page.liquid
permalink: pre-trained/index.html
eleventyNavigation:
    key: Pre-Trained Terrains (Presets)
    order: 3
date: git Last Modified

---

Make sure to have both:
* an **autoencoder** (a `.ts` file)
* a **terrain** model (a `.pt` file).




### Autoencoder: Percussion - RAVE v1
Pre-trained autoencoder author: Antoine Caillon  
Download from RAVE official repository [percussion.ts](https://acids-ircam.github.io/rave_models_download).

* **Terrain**: [percussion-terrain-demo.pt](https://github.com/jasper-zheng/nn_terrain/raw/refs/heads/main/pre-trained/percussion-terrain-demo.pt)<br>
**Config**: 2 inputs 4 outputs, range: `-16, -4, 16, 4`<br>
<img class="damier" src="https://github.com/jasper-zheng/nn_terrain/blob/main/pre-trained/percussion-terrain-demo.png?raw=true" alt="Trajectories">




### Autoencoder: Music2Latent-Scripted
Original model is the work by Marco Pasini at SonyCSLParis.  
Scripted model can be downloaded from [music2latent.ts](https://github.com/jasper-zheng/music2latent-scripted.git).