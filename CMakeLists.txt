cmake_minimum_required (VERSION 2.8)
project (pbr)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++11)

set (SRC_DIR ${PROJECT_SOURCE_DIR}/src)	
file(GLOB PBR_SOURCEFILES
    ${SRC_DIR}/*.h
    ${SRC_DIR}/*.cpp
	${SRC_DIR}/*.c
)

# Libary includes
include_directories(pbr ${PROJECT_SOURCE_DIR}/include)

# Library linking
find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
set (EXTRA_LIBS ${EXTRA_LIBS} ${OPENGL_gl_LIBRARY})						# OpenGL

if (WIN32)
	set (LIB_DIR ${PROJECT_SOURCE_DIR}/lib/win32)
	set (EXTRA_LIBS ${EXTRA_LIBS} ${LIB_DIR}/SDL2.lib)					# SDL
	
	set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} /w4 /wd4121 /wd4201)			# Disable certain warnings
	add_executable (${PROJECT_NAME} WIN32 ${PBR_SOURCEFILES})
elseif(UNIX)
	set (LIB_DIR ${PROJECT_SOURCE_DIR}/lib/unix)
	set (EXTRA_LIBS ${EXTRA_LIBS} ${LIB_DIR}/libSDL2.so)				# SDL
	set (EXTRA_LIBS ${EXTRA_LIBS} -lGL -lX11 -lpthread -lXrandr -lXi)	# Required for OpenGL on Linux
	set (EXTRA_LIBS ${EXTRA_LIBS} -ldl)									# On non-Windows platforms GLAD requires libdl
	add_executable (${PROJECT_NAME} ${PBR_SOURCEFILES})
endif()

# MESSAGE(STATUS "opengl include_dirs are " ${OPENGL_gl_LIBRARY})
#add_executable (${PROJECT_NAME} WIN32 ${PBR_SOURCEFILES})
target_link_libraries(${PROJECT_NAME} ${EXTRA_LIBS})
