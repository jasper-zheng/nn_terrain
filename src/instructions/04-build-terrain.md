---
title: Building terrains from scratch
description: Instructions on building and training a customised terrain using supervised machine learning.
layout: libdoc_page.liquid
permalink: instructions/build-terrain/index.html
eleventyNavigation:
    key: Building Terrains from Scratch
    parent: Instructions
    order: 4
date: git Last Modified
tocEnabled: false
---

Building a terrain with 2 continuous control channels, for an autoencoder with 4 latent dimensions.

## What we need

To built a terrain from scratch, we need pairs of **latent trajectories** and **spatial trajectories**.  

* **Latent trajectories** are sequences of latent vectors encoded from audio buffers.
* **Spatial trajectories** are sequences of coordiantes in a control space. For instance:
  * Trajectory of mouse in an XY track pad
  * Trajectory of hand gestures in an XYZ 3D space
  * Timestamps in a timeline playback system

A terrain is a **supervised machine learning** model that learns this coordiantes-to-latents pairs, to produce new latent vectors given any coordiantes in the control space, so that the control space can be rendered as a "map" for the latent space.


<figure>
    <img src="../../assets/latent-coord-pairs.png"
        alt="Trajectories">
    <figcaption>
        Latent trajectories, spatial trajectories, and latent terrain.
    </figcaption>
</figure>

## Synopsis  

This tutorial guides you through: 

* Creating an empty terrain with customised number of inputs, outputs.  
* Gathering **latent trajectories** from audio buffers, using `nn.terrain.encode`.
* Plotting 2D **spatial trajectories** in `nn.terrain.gui` with customised length.
* Training the empty terrain with latent and spatial trajectories.
  * Setting up training dataset
  * Monitoring the training loss
  * Visualising terrain on-the-fly
  * Saving checkpoints