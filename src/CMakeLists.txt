project(oniongen)

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)

set(CMAKE_CXX_STANDARD 11)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  list (APPEND CMAKE_CXX_FLAGS "--std=gnu++11")
endif ()


# find_package(OpenMP)
# if (OPENMP_FOUND)
#     set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
#     set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
# endif()

find_package(CUDA REQUIRED)
include(FindCUDA)
include_directories(${CUDA_INCLUDE_DIRS})
#set(CUDA_VERBOSE_BUILD ON)

list(APPEND CUDA_NVCC_FLAGS "-gencode arch=compute_30,code=sm_30")
list(APPEND CUDA_NVCC_FLAGS "-gencode arch=compute_35,code=sm_35")
list(APPEND CUDA_NVCC_FLAGS "-gencode arch=compute_50,code=sm_50")
# list(APPEND CUDA_NVCC_FLAGS "-gencode arch=compute_52,code=sm_52")
# list(APPEND CUDA_NVCC_FLAGS "-gencode arch=compute_60,code=compute_60")
#LIST(APPEND CUDA_NVCC_FLAGS "-std=c++11 -Xcompiler -fopenmp")
list(APPEND CUDA_NVCC_FLAGS "-std=c++11 -lineinfo")
set(CUDA_PROPAGATE_HOST_FLAGS OFF)

list(APPEND SOURCES main.cc controls.cc cpu1.cc gpu1.cu helpers.cc)
cuda_add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${CUDA_LIBRARIES})
