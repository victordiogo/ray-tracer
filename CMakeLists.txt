cmake_minimum_required(VERSION 3.20)
project("ray-tracer")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(program_executable_name ${CMAKE_PROJECT_NAME})

file(GLOB_RECURSE src_files CONFIGURE_DEPENDS src/*.cpp)

set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
set(compile_options
  "$<${gcc_like_cxx}:-pedantic-errors;-Wall;-Weffc++;-Wextra;-Wsign-conversion;-Wconversion>"
  "$<${msvc_cxx}:/permissive-;/W4;/WX>"
)

include(FetchContent)

FetchContent_Declare(
	glm
	GIT_REPOSITORY https://github.com/g-truc/glm.git
	GIT_TAG 45008b225e28eb700fa0f7d3ff69b7c1db94fadf
)
FetchContent_MakeAvailable(glm)

find_package(OpenMP)

set(external_lib_dir ${CMAKE_BINARY_DIR}/external)

# stb_image.h
file(
	DOWNLOAD
	"https://raw.githubusercontent.com/nothings/stb/master/stb_image.h"
	"${external_lib_dir}/include/stb_image.h"
)

file(
	COPY assets
	DESTINATION ${CMAKE_BINARY_DIR}
)

add_executable(${program_executable_name} ${src_files})
target_include_directories(${program_executable_name} PRIVATE include)
target_include_directories(${program_executable_name} PRIVATE ${external_lib_dir}/include)
target_compile_options(${program_executable_name} PRIVATE ${compile_options})
target_link_libraries(${program_executable_name} PRIVATE glm::glm OpenMP::OpenMP_CXX)
