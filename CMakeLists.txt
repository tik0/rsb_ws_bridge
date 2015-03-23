cmake_minimum_required (VERSION 2.6)

# Set the project name to the name of the folder
string (REGEX MATCH "[^/]+$" PROJECT_NAME "${CMAKE_CURRENT_BINARY_DIR}")
message (STATUS "Set PROJECT_NAME to ${PROJECT_NAME}")

project ("${PROJECT_NAME}")


# Read the pkg-config modules
find_package(PkgConfig)

pkg_check_modules(RSB rsb0.11)
pkg_check_modules(RSC rsc0.11)
pkg_check_modules(PROTO protobuf)

# Get modules
find_package(Boost REQUIRED regex date_time program_options system thread)
# find_package(OpenCV REQUIRED)

# Get the compiler specific includes and store them in GXX_INCLUDES
# INCLUDE("$ENV{MUROX_CMAKE_MODULES}/setCompilerIncludes.cmake")

# Set the include path
include_directories(
    ${Boost_INCLUDE_DIRS}
    ${RSB_INCLUDE_DIRS}                                             # RSB
    ${RSC_INCLUDE_DIRS}                                             # RSC
    ${PROTO_INCLUDE_DIRS}                                             # RSC
#     ${OpenCV_INCLUDE_DIRS}                                          # OpenCV
#     ${LIBV4l2_INCLUDE_DIRS}                                         # Video for Linux
#     "../../includes"                                                # Other includes like converters and stuff
    "${CMAKE_CURRENT_BINARY_DIR}"                                   # add the binary tree to the search path 
    "./mongoose"                                                    # Mongoose
#     "$ENV{MUROX_INCLUDE_DIRS}"                                      # Other includes like converters and stuff
#     ${CMAKE_CURRENT_BINARY_DIR}                                     # add the binary tree to the search path for
#     ${GXX_INCLUDES}
)

# add the executable
add_executable ("${PROJECT_NAME}" main.cxx ./mongoose/mongoose.c ./example2.pb.cc ./number.pb.cc ./SimpleConverter.cxx)
#project_html.c)

# Set the compiler
# SET (CMAKE_C_COMPILER $ENV{CPP})
# SET (CMAKE_CXX_COMPILER $ENV{CXX})

# Link the executable to the library. 
# SET (BLAH "$ENV{SDKTARGETSYSROOT}/usr/lib")
# link_directories (
#     "$ENV{SDKTARGETSYSROOT}/usr/lib"
# )
target_link_libraries (
    "${PROJECT_NAME}"
    ${RSB_LIBRARIES}
    ${RSC_LIBRARIES}    
    ${Boost_LIBRARIES}
    ${PROTO_LIBRARIES}
#     ${OpenCV_LIBRARIES}
)

# Additional compiler flags

# SET (
#     CMAKE_CXX_FLAGS
#         "-std=c++0x -Wno-deprecated-declarations -I.. -O2 -pthread -g -pipe -fpermissive -Wall -Wno-unused-variable"
# )

# Additional compiler flags
ADD_DEFINITIONS (-Wall -Wno-deprecated-declarations -O2 -Wno-write-strings -std=c++11)
