cmake_minimum_required(VERSION 3.24..3.31)

project(coopmat)

set(sources 
    coopmat.cpp
    coopmat_benchmark.cpp
    coopmat_benchmark_shader.cpp
)

add_executable(coopmat ${sources})


find_package(Vulkan REQUIRED COMPONENTS SPIRV-Tools glslang)
find_package(fmt REQUIRED)
# using hash_combine somewhere
find_package(Boost REQUIRED)

target_link_libraries(coopmat PRIVATE 
    Vulkan::Vulkan 
    # This is broken (Tried on Windows (msys2+ucrt64) and Arch)
    #Vulkan::shaderc_combined
    # This works on msys2+ucrt64 and Arch, but probably not with MSVC?
    # TODO: replace with find_library() or smth. else
    -lshaderc_shared
    fmt::fmt 
    Boost::boost)
target_compile_features(coopmat PRIVATE cxx_std_23)
