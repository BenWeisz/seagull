# Wherever "Configure!" is marked, thats where you have to make changes
# Note: You'll have to create a /src and /include for your .c and .h files respectively
#       Don't worry, I've configured those folders below, but you still have to write your
#       .c files below and run cmake .. from your build dir each time you add a new source file.
cmake_minimum_required(VERSION 3.20)

# Configure! your exe name
set(EXE_NAME seagull)
# Configure! this as the absolute path to your vendors folder, DO NOT USE RELATIVE PATHS!
set(VENDORS "/home/bweisz/dev/vendors")
# Configure! this if you want to build for the old intel based apple systems
# set(INTEL_APPLE 1)

# Setting up the project name
project(${EXE_NAME})

###################################
# Configure! the language standard
# Set C standard here (Used in App)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-Wall")

# Set C++ standard here (Used in cimgui lib)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-Wall")

# Configuration for Intel based macs
if(APPLE)
    if(DEFINED INTEL_APPLE)
        set(CMAKE_OSX_ARCHITECTURES "x86_64")
    ENDIF()
ENDIF()

###################################
# Setup cimgui library build

# Sources need by cimgui
set(CIMGUI_SOURCES
	${VENDORS}/cimgui/cimgui.cpp
	${VENDORS}/cimgui/imgui/imgui.cpp
	${VENDORS}/cimgui/imgui/imgui_draw.cpp
	${VENDORS}/cimgui/imgui/imgui_tables.cpp
	${VENDORS}/cimgui/imgui/imgui_demo.cpp
	${VENDORS}/cimgui/imgui/imgui_widgets.cpp
	${VENDORS}/cimgui/imgui/backends/imgui_impl_opengl3.cpp
	${VENDORS}/cimgui/imgui/backends/imgui_impl_glfw.cpp
	${VENDORS}/glad/src/glad.c)

add_library(cimgui STATIC ${CIMGUI_SOURCES})

target_compile_definitions(cimgui PUBLIC 
	"-DIMGUI_IMPL_API=extern \"C\""
	"-DIMGUI_DISABLE_OBSOLETE_FUNCTIONS=1")

# Add include directories cimgui needs
target_include_directories(cimgui PUBLIC
	${VENDORS}/cimgui
	${VENDORS}/cimgui/imgui
	${VENDORS}/cimgui/generator/output
	${VENDORS}/glad/include)

target_link_libraries(cimgui PUBLIC glfw)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/cimgui)
set_target_properties(cimgui PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/cimgui)

# Configure!
# Common sources go here
set(SOURCE_FILES main.c
	src/io.c
	src/database.c
	# src/another_file.c
	# src/and_another.c
	)

# Configure! Set up the platform dependent sources
if(WIN32)
    # list(APPEND SOURCE_FILES src/win32_impl.c)
elseif(APPLE)
    # list(APPEND SOURCE_FILES src/macos_impl.c)
else() # Linux
    # list(APPEND SOURCE_FILES src/linux_impl.c)
endif()

# Add the executable
add_executable(${EXE_NAME} ${SOURCE_FILES})

# Add GLFW subdirectory
add_subdirectory(${VENDORS}/glfw-3.4 glfw-3.4)

# Add the include directory for headers
target_include_directories(${EXE_NAME} PUBLIC
	include
	${VENDORS}/glad/include # GLAD - OpenGL 4.6 Core
	${VENDORS}/cimgui
	${VENDORS}/cimgui/imgui
	${VENDORS}/cimgui/generator/output
	${VENDORS}/sqlite)

# Add sqlite3
target_link_directories(${EXE_NAME} PUBLIC
	${VENDORS}/sqlite)

# Link glfw, cimgui to application
# Note: We do this a bit differently for APPLE beacuse its linker is smarter and will rescan the static libs for objects
if (APPLE)
	target_link_libraries(${EXE_NAME} PUBLIC cimgui sqlite3)
elseif(WIN32)
	target_link_libraries(${EXE_NAME} PUBLIC glfw cimgui sqlite3)
else()
	find_package(X11 REQUIRED)
	target_link_libraries(${EXE_NAME} PUBLIC glfw cimgui sqlite3 X11::X11)
endif()

# Load the GLAD source
target_sources(${EXE_NAME} PUBLIC ${VENDORS}/glad/src/glad.c) 

# Add CIMGUI compile definitions
target_compile_definitions(${EXE_NAME} PUBLIC 
	"-DIMGUI_DISABLE_OBSOLETE_FUNCTIONS=1"
	-DCIMGUI_USE_OPENGL3
	-DCIMGUI_USE_GLFW)

# Add debug mode macro
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(__DEBUG__)
endif()
