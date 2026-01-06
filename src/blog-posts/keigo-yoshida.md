---
title: "Latent space artists: A conversation with Keigo Yoshida"
description: In this series of interviews, we invited artists to share the design thinking and experience behind the making process of their demo projects. 
layout: libdoc_page.liquid
permalink: "{{ libdocConfig.blogSlug }}/conversation-with-keigo-yoshida/index.html"
tags:
    - post
    - demos
date: 2025-12-05
ogImageUrl: "../assets/keigo-screenshot-2.jpg"
author: Keigo Yoshida
tocEnabled: false
---

{% include "artist-interview.md" %}


{% iconCard 'Artist Bio', '**Keigo Yoshida** is a Tokyo-based artist and scientist who integrates neuroscience and computer science to explore novel forms of artistic expression. His notable works include the A/V performances "Propagation" and "Mineral Neurons", both presented at Sónar+D 2025, and "liberated frequencies" (Installation and A/V performance). The latter work was supported by Flying Tokyo 2024, involved collaboration with METI and Rhizomatiks, and was also presented at IRCAM Forum Workshops 2025 Hors-les-Murs. [https://keigoyoshida.jp/](https://keigoyoshida.jp/)', 'user' %} 

### Repressive Latent Terrain: An experimental data sonification patch 

An experimental data sonification patch designed by Keigo Yoshida. It allows the audience to engage in the experiment to explore the latent space through an OpenBCI Electroencephalography (EEG) headset. It aims to create a meditative listening experience, in which the audience is instructed to stay calm and relaxed, while the time-varying EEG data is sonified to a disruptive and arousal soundscape. The artistic theme is the active manipulation of consciousness in a focused meditation, and an adversarial tension between the subject's striving for calm and the algorithm's impulse toward arousal sonic responses. 

The EEG headset tracks two critical brainwave frequencies: the Alpha waves that are associated with deep relaxation and inner calm, and the Gamma waves that are associated with high-level arousal and active processing. These two time-varying frequencies are used as the control input for latent space navigation. In this way, the sonic expression continuously shifts based on the balance between how relaxed and how alert the subject is during meditation. The arrangement of latent trajectories is constantly changing, and therefore the terrain is constantly trained during the interaction.  

{{ 'https://www.youtube-nocookie.com/embed/A0bdwQGtKqQ?si=eRUN7HJjXjyyLjEs?controls=1' | embed }}

<br><br>


##### Could you tell us a bit about yourself and your artistic practice in MaxMSP / interactive sound art / music composition? 

**Keigo:** I’ve been using Max/MSP consistently in sound installations and audiovisual performances in my career. One example is my piece ‘liberated frequencies’, created for the Flying Tokyo 2024 project, which was supported by the Ministry of Economy, Trade and Industry (METI) and Rhizomatiks. For that work, I built a system that processed EEG brainwave data in Max/MSP and sent it to Python to control the latent space of a WaveNet model. This system was demonstrated at the IRCAM Forum Workshops in 2025 Hors-les-Murs. My practice extends to using Max/MSP and various other tools for sonic production in a wide range of projects.
 
##### Before starting the nn.terrain project, did you have any prior hands-on experience with neural audio synthesis (e.g., RAVE, autoencoders), or more broadly AI music (e.g., text-to-audio, machine learning)? 

**Keigo:** My recent practice has focused heavily on integrating advanced neural networks into live performance. At Sónar+D 2025 AI Performance Playground, I presented two audiovisual performances: ‘Mineral Neurons’ used a neural synthesis (nn~) with gen_synth_fx coupled with a physical vibration system to transform acoustics in real-time. ‘Propagation’ used two instances of Netone’s RAVE-based Neutone morpho to convert raw noise signals into complex neural network noise. 

**Keigo:** I also worked on a Tedx KeioU conference, I collaborated with researchers Yuki Kawai and Shinya Fujii to integrate a reservoir computing model, which mimics the brain's learning process. The resulting A/V performance featured a dynamic call-and-response between a human drummer and the output of the reservoir computing model, showcasing a deeper form of machine-human interplay. These projects underscore my extensive practical engagement with machine learning across various creative domains.
   
##### Is working with neural audio synthesis different from working with other materials (e.g., conventional sound synthesis algorithms / samples). 

**Keigo:** The possibilities of neural audio synthesis are unique due to their inherent learning process. For me the processes like interactively adapting the latent space to a terrain are how the system instantaneously traces and accesses past states (training data) in real-time to transform sound. They're similar to a sampler but distinct from it. Furthermore, because the model is able to shift timbre and musical structure across genres, I like to use completely different rhythmic patterns and sonic textures to experiment with the sound space creation. This opens creative possibilities that traditional synthesis or sample-based methods simply can’t offer.
 
##### How did you integrate neural audio synthesis in your making/crafting process? What is your workflow?  

**Keigo:** I found useful about the approach of first mapping the latent space and then tracing the map like a musical score. This shares some commonalities with graphic scores, and the process of translating information into a visual form and treating it like a musical score is an entirely new compositional method to me.

**Keigo:** This ability to read these scores in a multi-directional way rather than unidirectional, is very different from the traditional linearity embedded in score-writing. 
 
