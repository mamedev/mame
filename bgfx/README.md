# **BGFX shaders** #

Contains definitions for BGFX shaders, shader chains and effects.

## Vector CRT renderer

When the BGFX video backend is active, vector screens are rendered by a
dedicated GPU pipeline.  It uses persistent RGBA16F phosphor buffers,
time-based exponential decay, ordered instanced beam quads, quarter-resolution
Gaussian bloom, and an HDR composite pass.  It is enabled automatically for
MAME primitives marked as vector output; no screen chain needs to be selected.

The on-screen slider menu exposes phosphor persistence, beam width and
intensity, halo strength, bloom strength and radius, and exposure.  If the
selected GPU cannot create RGBA16F render targets, MAME logs a warning and
falls back to the normal BGFX line renderer.

## Depixelize chain

The `depixelize` screen chain upscales pixel art with the vectorization
algorithm of Johannes Kopf and Dani Lischinski ("Depixelizing Pixel Art",
SIGGRAPH 2011), after the GPU implementation by Felix Kreuzer.  It builds a
similarity graph of the screen, turns the boundaries between dissimilar pixels
into B-splines, smooths them, and rasterizes the result at the output
resolution so that contours stay sharp at any scale.  Every pass is a
fragment shader; intermediate data lives in `rgba32f` render targets (see the
`format` target option in `chains/hlsl.json`).  The "Smooth Splines" slider
switches the spline smoothing off for a more angular look.  It is meant for
clean, low-resolution graphics; dithered or noisy content gains little.

Shader sources: `src/osd/modules/render/bgfx/shaders/chains/depixelize/`.
The chain, effects and shaders of the depixelize chain are licensed under the
MIT License by Felix Kreuzer.

Licensed under [The BSD 3-Clause License](http://opensource.org/licenses/BSD-3-Clause) by Ryan Holtz and MAME Development Team
