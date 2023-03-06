# TODO

* Bundle everything into a single html file.
  * Somehow put the wasm and/or js into the html file
* With Minetest's default_lava.png, green pixels appear. Perhaps the colour
  decorrelation works incorrectly or badly
* Add a HTTP option which enables rounding uv coordinates in the shader and
  disables interpolation. This is needed for Minetest textures.
* Code quality and comments; perhaps somebody is going to read my code
* Support high bit depth and wide gamut images
* Give transparent pixels no weight when calculating the covariance matrix
  for the colour decorellation and skip fully transparent pixels in the
  histogram transformation
