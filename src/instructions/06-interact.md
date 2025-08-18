---
title: Composing and interacting with a terrain
description: Programming trajectories in a 2D terrain.
layout: libdoc_page.liquid
permalink: instructions/interact/index.html
eleventyNavigation:
    key: Composing and Interacting with Terrains
    parent: Instructions
    order: 6
date: git Last Modified
tocEnabled: false
---

The `play` mode of `nn.terrain.gui` offers position-based trajectory playback, which is similar to the `play~` object in Max - plays back latent trajectories based on an offset within the trajectory.  

It can be used with any signal that generates a positional value in milliseconds (e.g., the `line~` object).



<img src="../../assets/play.gif" width="500" alt="Playback"></img>

**Please see the help file of `nn.terrain.gui` in Max for details:**

<img src="../../assets/gui.png" alt="Playback"></img>