# TODO

* Properly add the html file for emscripten
* Stochastic texture sampling, at first without histogram transformation for
  simplicity
* Implement the histogram transformation (see the existing demo for reference)
* Enable drag-and-drop of image files in the browser to load and showcase
  them with and without stochastic texture sampling
* Add a HTTP option which enables rounding uv coordinates in the shader and
  disables interpolation. This is needed for Minetest textures.
* Add an image which shows "press h" and show help text when pressing h, which
  shows info about all key combinations, mouse, drag and drop, etc.
* Fill the whole window instead of using a fixed resolution
* Bundle everything into a single html file
