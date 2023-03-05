cmake_minimum_required(VERSION 3.13)
project(prog)

add_compile_options(-std=c++20 -g -pipe -Wall -Wextra -Wnull-dereference
#~ add_compile_options(-std=c++20 -Oz -pipe -Wall -Wextra -Wnull-dereference
	-Wmisleading-indentation -Wno-unused-parameter)
if(EMSCRIPTEN)
	# emcmake cmake -G Ninja -B build_emscripten
	# cmake --build build_emscripten/
	# cd build_emscripten/ && python3 -m http.server 9000
	set(CMAKE_EXECUTABLE_SUFFIX .html)
	add_compile_options(-sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sSDL2_IMAGE_FORMATS=["png","jpg"] -fwasm-exceptions)
	#~ add_link_options(-Oz -fwasm-exceptions --preload-file ../data/help_image.png@data/help_image.png --embed-file ../data/shaders@data/shaders -sMAX_WEBGL_VERSION=2 -sMIN_WEBGL_VERSION=2 --use-preload-plugins -sEXPORTED_RUNTIME_METHODS=['ccall'] -sEXPORTED_FUNCTIONS=['_main','_malloc','_free'])
	add_link_options(-fwasm-exceptions --preload-file ../data/help_image.png@data/help_image.png --embed-file ../data/shaders@data/shaders -sMAX_WEBGL_VERSION=2 -sMIN_WEBGL_VERSION=2 --use-preload-plugins -sEXPORTED_RUNTIME_METHODS=['ccall'] -sEXPORTED_FUNCTIONS=['_main','_malloc','_free'])
	configure_file(src/web/index.html index.html COPYONLY)
endif()

add_executable(Prog
	src/main.cpp
	src/image_file.cpp
	src/renderer.cpp
	src/camera.cpp
	src/shader_program.cpp
	src/texture.cpp
	src/texture_stochastic.cpp
)
target_include_directories(Prog PUBLIC src)

target_link_libraries(Prog PUBLIC glfw SDL2 SDL2_image GL GLEW)

