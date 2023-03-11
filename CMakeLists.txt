cmake_minimum_required(VERSION 3.13)
project(prog)

add_compile_options(-Os -std=c++20 -g -pipe -Wall -Wextra -Wnull-dereference
	-Wmisleading-indentation -Wno-unused-parameter -Wfloat-conversion)
if(EMSCRIPTEN)
	# emcmake cmake -G Ninja -B build_emscripten
	# cmake --build build_emscripten/
	set(CMAKE_EXECUTABLE_SUFFIX .html)
	add_compile_options(
		-fwasm-exceptions
		-sUSE_SDL=2
		-sUSE_SDL_IMAGE=2
		-sSDL2_IMAGE_FORMATS=["png","jpg","jxl"]
	)
	add_link_options(
		-Os
		-fwasm-exceptions
		--embed-file ../data@data
		--use-preload-plugins
		--shell-file=../src/web/index.html
		-sMAX_WEBGL_VERSION=2
		-sMIN_WEBGL_VERSION=2
		-sEXPORTED_RUNTIME_METHODS=['ccall']
		-sEXPORTED_FUNCTIONS=['_main','_malloc','_free']
		-sSINGLE_FILE
		-sALLOW_MEMORY_GROWTH
		-sENVIRONMENT='web'
		#~ -sMALLOC=emmalloc
	)
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

