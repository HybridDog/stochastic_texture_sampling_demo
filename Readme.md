# TODO

* Stochastic texture sampling, at first without histogram transformation for
  simplicity
* Make it work with emscripten and add the html to load it
* Implement the histogram transformation (see the existing demo for reference)
* Enable drag-and-drop of image files in the browser to load and showcase
  them with and without stochastic texture sampling
* Add a HTTP option which enables rounding uv coordinates in the shader and
  disables interpolation. This is needed for Minetest textures.
