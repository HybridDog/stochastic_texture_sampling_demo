cmake_minimum_required(VERSION 3.10)
project(prog)

#~ add_compile_options(-Ofast -march=native -pipe -Wall -Wextra -Wnull-dereference
	#~ -Wmisleading-indentation -Wlogical-op -Wno-unused-parameter)
add_compile_options(-g -pipe -Wall -Wextra -Wnull-dereference
	-Wmisleading-indentation -Wlogical-op -Wno-unused-parameter)

add_executable(Prog
	src/main.cpp
	src/image_file.cpp
	src/renderer.cpp
	src/shader_program.cpp
	src/texture.cpp
)
target_include_directories(Prog PUBLIC src)

target_link_libraries(Prog PUBLIC glfw SDL2 SDL2_image GL GLEW)

