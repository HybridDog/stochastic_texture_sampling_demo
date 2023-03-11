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
* Convert the colours of each pixel to OKLab (or almost OKLab since the final
  matrix multiplication is excluded)
* Calculate a decorrelation matrix with Principal Component Analysis from all
  these colour values, and apply this matrix on the colours
* Perform a histogram transformation for each component of the decorrelated
  colour vector and lookup tables for the inverse histogram transformation

Experiments with [SuperTux' snow tiles](https://github.com/SuperTux/supertux/blob/baf72d708b982789c0be8ca912d3b59a76e17c0a/data/images/tiles/snow/convex.png)
and [Minetest's `default_lava.png`](https://github.com/minetest/minetest_game/blob/aeb27c4db6959d20e525f5754b88d107b168e957/mods/default/textures/default_lava.png)
have shown that the colour decorrelation with linear RGB values leads to colour
problems in the final image, such as violet colours in the blue SuperTux
texture and green colours in the orange-red `default_lava.png`.
With sRGB, which is what the demonstration of Thomas Deliot et al. uses, these
problems are less visible, and a perceptual colour space appears to give the
best results of the three for these example textures.



# TODO

* Scrolling is too fast
* With the leopard texture and my modified default_gravel.png,
  the colour decorrelation still doesn't look very
  good. Experiment what happens if I use OKLab without PCA.
  Perhaps it works with OKLab if I do not omit OKLab's matrix multiplication.
* Code quality and doc
  * Comment classes and methods in hpp files
  * Explain what this is in this readme
  * Add license info
* Don't crash when dropping unsupported files
* Support different grid scales. If I tile a 16x16 texture from Minetest to
  48x48 in GIMP and use this as input, I get better results.
* Add arguments to main and specify them on command line or as HTTP options
  * Option for interpolation
  * Initial camera positions
  * URL to a texture
* Transparency support
  * Give transparent pixels no weight when calculating the covariance matrix
    for the colour decorellation
  * skip fully transparent pixels in the histogram transformation
  * design the background
* Support high bit depth and wide gamut images
  * Investigate if SDL3 works better for this purpose
  * Use the monitor-specific colour profile (if possible at all)
  * Load images into linear rgb float arrays (perhaps SDL2 image supports this)
