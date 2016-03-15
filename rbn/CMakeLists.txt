cmake_minimum_required(VERSION 2.8)
project(rbn)

find_package(CUDA REQUIRED)
include_directories(/share/packages/boost-1.53.0/include)
add_definitions(-DENABLE_PARALLEL)

set(SOURCE_FILES
        rbn3.cpp
        boolean_functions.cpp
        handler.cpp
        hsystem.cpp
        hparams.cpp
        my_random.cpp
        network.cpp
        node.cpp
   )

set(CUDA_NVCC_FLAGS -arch=compute_13)

set(CUDA_SOURCE_FILES
        gpu/detail/device/boolean_functions.cu
        gpu/detail/device/knuth_attractor_finder.cu
        gpu/detail/device/liu_bassler_attractor_finder.cu
        gpu/detail/device/equal_to_ref_array.cu
        gpu/rbn.cu
        gpu/detail/device/_rbn.cu
   )

cuda_add_library(gpu ${CUDA_SOURCE_FILES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
add_executable(rbn ${SOURCE_FILES})

target_link_libraries(rbn gpu)
