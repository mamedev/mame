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

Licensed under [The BSD 3-Clause License](http://opensource.org/licenses/BSD-3-Clause) by Ryan Holtz and MAME Development Team
