cmake_minimum_required(VERSION 2.6)
project(klein)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules)

find_package(OpenGL REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(SDL2 REQUIRED)

add_executable(klein main.cxx)

target_include_directories(klein PUBLIC
 	${PROJECT_SOURCE_DIR}/
	${EIGEN3_INCLUDE_DIR}
	${SDL2_INCLUDE_DIR}
)
target_link_libraries(klein
	${SDL2_LIBRARIES}
	${OPENGL_LIBRARIES}
)
target_compile_definitions(klein PUBLIC
	EIGEN_DONT_ALIGN
)
target_compile_options(klein PRIVATE
	-Wno-deprecated-declarations
)
target_compile_features(klein PUBLIC
	cxx_uniform_initialization
)

install(TARGETS klein RUNTIME DESTINATION bin)
