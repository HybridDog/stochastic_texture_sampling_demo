# TODO

* Scrolling is too fast
* With Minetest's default_lava.png, green pixels appear. The colour
  decorrelation works incorrectly or badly
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
