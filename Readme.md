# Stochastic Texture Sampling Demonstration

This application is based on the official demonstration of
["Procedural Stochastic Textures by Tiling and
Blending"](https://eheitzresearch.wordpress.com/738-2/) by Thomas Deliot and
Eric Heitz, which explains modifications to ["High-Performance By-Example
Noise using a Histogram-Preserving Blending
Operator"](https://eheitzresearch.wordpress.com/722-2/) for practical
implementations.

Using emscripten, the application can be compiled to a self-contained HTML file,
which works after downloading offline in a web browser.


## Colour Transformation

The decorrelation and gaussianization of colours is implemented as follows:
* Load an input texture, which is usually in the sRGB colour space
* Convert the colours of each pixel to OKLab
* Calculate a decorrelation matrix with Principal Component Analysis from all
  these colour values and apply this matrix on the colours.
  Additionally, create a matrix which inverts this decorrelation and the last
  matrix multiplication step of the OKLab conversion.
* Perform a histogram transformation for each component of the decorrelated
  colour vector. Additionally, create lookup tables for the inverse histogram
  transformation.

Experiments with [SuperTux' snow tiles](https://github.com/SuperTux/supertux/blob/baf72d708b982789c0be8ca912d3b59a76e17c0a/data/images/tiles/snow/convex.png)
and [Minetest's `default_lava.png`](https://github.com/minetest/minetest_game/blob/aeb27c4db6959d20e525f5754b88d107b168e957/mods/default/textures/default_lava.png)
have shown that the colour decorrelation with linear RGB values leads to colour
problems in the final image, such as violet colours in the blue SuperTux
texture and green colours in the orange-red `default_lava.png`.
With sRGB, which is what the demonstration of Thomas Deliot et al. uses, these
problems are less visible, and a perceptual colour space appears to give the
best results of the three for these example textures.


## Missing Features

It is possible to extend this demonstration with more not-yet-implemented
features, for example:
* Arguments in the URL (HTTP parameters) and command-line.
  To showcase an example texture with stochastic texture sampling, it can be
  helpful to initialise the state of the application to a desired one.
  * Option for interpolation
  * Initial camera positions
  * Triangle grid scaling
  * URL to a texture
* Transparency support.
  With a texture with either fully transparent or opaque pixels, the colours
  currently look wrong (too dark).
* Support high bit depth and wide gamut images
  * Investigate if SDL3 works better for this purpose
  * Use the monitor-specific colour profile (if possible at all)
  * Load images into linear rgb float arrays (perhaps SDL2 image supports this)
* Support mipmapping.
  The original demonstration uses 2D lookup tables to accout for colour problems
  when mipmaps are used. This is not yet implemented here for simplicity.
* Texture drag and drop without a browser.
  Currently, the texture can only be changed the application is compiled with
  emscripten.
  With SDL2 drag and drop, it should also be possible to do this without a
  browser.

Pull requests are welcome.
