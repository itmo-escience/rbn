cmake_minimum_required(VERSION 2.8)
project(rbn_original)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

find_package(Boost 1.46.0 REQUIRED)
find_package(CUDA REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
add_definitions(-DENABLE_GPU_ACCELERATION)

set(SOURCE_FILES
        rbn3.cpp
        boolean_functions.cpp
        handler.cpp
        hsystem.cpp
        hparams.cpp
        my_random.cpp
        network.cpp
        node.cpp
        gpu_acc/find_attractor.cpp
        gpu_acc/dev_network.cpp
    )

set(CUDA_SOURCE_FILES
        gpu_acc/dev_find_attractor.cu
    )

cuda_add_library(gpu_acc ${CUDA_SOURCE_FILES})
add_executable(rbn ${SOURCE_FILES})
target_link_libraries(rbn gpu_acc)
