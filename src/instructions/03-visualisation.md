---
title: Creating visualisations
description: Given a terrain with 2D inputs, we can visualise it as a map.
layout: libdoc_page.liquid
permalink: instructions/visualisation/index.html
eleventyNavigation:
    key: Creating Visualisations
    parent: Instructions
    order: 3
date: git Last Modified
tocEnabled: false
---

Since the control space is 2D, the latent space can be visualised by sampling the control space across a closed interval (i.e., width and height in this example). Use the `plot_interval` message to do this:  
 - `plot_interval` for 2D plane takes 6 arguments:
 - interval value boundary at the left, top, right, bottom (these usually are the same as the `values_bound` attribute of `nn.terrain.gui`), and the resolution of the x and y axes (these usually are the same as the width and height of `nn.terrain.gui`)
<img src="../../assets/plot.jpg"
        alt="Trajectories">

The greyscale visualisation is like a surface map that looks at a terrain from above, where brighter pixels denote higher latent value.