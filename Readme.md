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

Instead of a gaussian distribution, this application uses the truncated
gaussian distribution and soft-clipping contrast operator
explained in [On Histogram-preserving Blending for Randomized Texture
Tiling](https://jcgt.org/published/0008/04/02/).
With a simple gaussian distribution, the lookup table works badly: the first and
last entry do not correspond to the highest and lowest value of the sorted
channel values, which results in visible clipping artifacts.


## Missing Features

It is possible to extend this demonstration with more not-yet-implemented
features, for example:
* Low discrepancy noise for the patch offsets.
  If we replace the pcg3d PRNG in the shader by something with
  low discrepancy, e.g. blue noise, there would be less low frequency in the
  patch offsets.
  Hypothetically, this means there are less clumps with visible tiling, so we
  could make the grid coarser for the same tiling visibility.
  A coarser grid means better preservation of patterns within the texture.
  Pressing a key to toggle between low discrepancy and usual randomness could be
  interesting for the user.
* Automatically set a good grid scaling using a heuristic.
  In my experience so far, if the texture has many low frequency parts, a coarse
  grid scaling looks better whereas if the texure consists of only high
  frequency parts, the grid scaling can be configured to be finer without
  altering the look of the texture a lot.
  It should be possible to do a Wavelet or Fourier transformation on the
  gaussianized texture to determine the frequencies and use the low frequency
  amplitudes for a grid scaling value heuristic.
  Determining the frequencies after the histogram transformations (i.e. on the
  gaussianized texture) should give better results than using the input texture
  since the heuristic should be independent of the global contrast, color, etc.
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
* "exponentiated blending" from
  "On Histogram-preserving Blending for Randomized Texture Tiling"

Pull requests are welcome.


# TODO

* The default "press h" texture histogram transformation reversal does not give
  the original texture; with nearest neighbour interpolation, there are some
  wrong pixels.
  This can be tested with adjusted weights in the shader:
  ```
  w1 = 1.0;
  w2 = 0.0;
  w3 = 0.0;
  ```
* With a space texture which is black except for a few stars or a green meadow
  texture with a single small violet flower, there are visible artifacts.
