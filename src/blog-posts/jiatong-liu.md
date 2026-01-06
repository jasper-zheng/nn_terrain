---
title: "Latent space artists: A conversation with Jiatong Liu"
description: In this series of interviews, we invited artists to share the design thinking and experience behind the making process of their demo projects. 
layout: libdoc_page.liquid
permalink: "{{ libdocConfig.blogSlug }}/conversation-with-jiatong-liu/index.html"
tags:
    - post
    - demos
date: 2025-12-15
ogImageUrl: "../assets/jt.jpg"
author: Jiatong Liu
tocEnabled: false
---

Interview with Jiatong Liu, edited by Jasper Shuoyang Zheng.

{% include "artist-interview.md" %}


{% iconCard 'Artist Bio', '**Jiatong Liu** is a Creative Technologist studying at the Creative Coding Institute, University of the Arts London. Their practice and research revolve around Machine Learning, Composition and Film - combining narrative design, spatial design and technology. [https://jiatongliu.com/](https://jiatongliu.com/)', 'user' %} 

### nn/mémoire: An audio-visual installation

nn/mémoire is an audio-visual virtual environment that aims to use neural audio synthesis to reimagine the sound of cultural heritage in Hutongs, a type of traditional northern Chinese courtyard house in Beijing. The piece is a virtual gallery that allows audiences to wander through and hear the real-time generated soundscape that captures the sound of Hutongs. The soundscape is composed of archival recordings from the Sound Art Museum Beijing, aiming to preserve the vanishing living culture that is now vanishing due to urban changes. It invites the audience to discover, remember, and connect to their memory of sound through the embodied listening in the environment.

{{ 'https://www.youtube-nocookie.com/embed/EZ7BbCh0b6Q?si=4ZIU8baGOICIaOtG?controls=1' | embed }}

<br><br>


##### Could you tell us a bit about yourself and your artistic practice in MaxMSP / interactive sound art / neural audio synthesis?
**Jiatong:** I’m a composer for film and theater with a background in creative music production. I also work on interactive sound projects that involve coding. In terms of Max/MSP, I’d say I’m an intermediate user—I’m comfortable with most objects and can prototype systems with a good level of independence.

**Jiatong:** I didn’t have any prior experience with neural audio synthesis or AI music tools like RAVE or autoencoders. I hadn’t even heard of those terms before this project.


##### Could you tell us about the work you did in the nn.terrain project?

**Jiatong:** I created an interactive audio-visual installation that uses nn.terrain as a sound design tool inside Unreal Engine. The idea was to build a virtual sound environment where the audience explores a 3D game-like space in a first-person view. As they move through the environment, their movements are mapped to a neural network. This means that walking around changes the sounds they hear, which are generated from different parts of the latent space in the neural network.

**Jiatong:** The theme of the piece is “nostalgic dreamcore,” and it uses traditional cultural sounds from Beijing hutongs. I worked with a dataset from the Sound Art Museum in Beijing, which archives disappearing hutong soundscapes. The goal was to recreate the feeling of these spaces in an immersive, interactive environment.


##### Is working with neural audio synthesis different from working with conventional sound synthesis algorithms?

**Jiatong:** Yes, it’s very different. Traditional music production tools—like reverb, compression, or EQ—mainly shape the sound that already exists. Neural audio synthesis, on the other hand, changes the structure and order of the sound itself. It feels closer to granular synthesis than to conventional techniques, but overall, it’s a completely new way of working compared to standard audio production.

##### How did you approach composing with neural audio synthesis? What did you learn about neural audio synthesis through the making/crafting process of your project?

**Jiatong:** Working with the 2D latent terrain map helped me understand how an autoencoder works internally—the encoder, decoder, and latent space architecture. To me, working with neural audio synthesis is a very different workflow than traditional materials, I had to find a balance between creating something new and experimental while keeping it somewhat familiar and recognizable for a broader audience. 

**Jiatong:** I need to learn to deal with the unpredictability of neural audio synthesis, through trial and error, I started to see patterns in how certain sounds are encoded and what unexpected results might emerge. I like being able to predict what would happen in the terrain, but also not so certain on all of them, is really interesting.

##### Could you highlight some features of your work that make it valuable, unique, or special? 
**Jiatong:** One key feature is how the sound is mapped into a 3D Unreal Engine environment. I placed sounds in specific regions of the virtual space, which adds a spatial dimension to the experience. Another unique aspect is the use of Hutongs soundscapes from my hometown, which makes the work personal and culturally meaningful. 

##### What would you like to see implemented in the tool in the future? 
**Jiatong:** I’d love to see more flexible ways to draw and edit trajectories—something like a “Bezier curve” mode where you can sketch paths instead of clicking points manually. It would also be great to have options to delete or move latent vectors, similar to using an eraser or dragging objects in Photoshop. These features would make the composition process more intuitive, especially for live performance.
