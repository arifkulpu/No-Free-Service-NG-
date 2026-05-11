cmake_minimum_required(VERSION 3.24)

project(NoFreeService VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CommonLibSSE CONFIG REQUIRED)
find_package(directxtk CONFIG REQUIRED)
find_package(DirectXMath CONFIG REQUIRED)

add_library(${PROJECT_NAME} SHARED
    src/main.cpp
    src/EconomyManager.cpp
    src/RecruitmentHandler.cpp
    src/SerializationManager.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE src)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    CommonLibSSE::CommonLibSSE
    Microsoft::DirectXTK
    Microsoft::DirectXMath
)

set_target_properties(${PROJECT_NAME} PROPERTIES 
    INTERPROCEDURAL_OPTIMIZATION ON
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    PREFIX ""
    SUFFIX ".dll"
)
