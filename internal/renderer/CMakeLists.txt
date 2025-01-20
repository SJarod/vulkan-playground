set(component renderer)

add_library(${component} STATIC "")

target_sources(${component}
    PRIVATE
    renderer.hpp
    renderer.cpp

    mesh.hpp
    mesh.cpp
)

target_link_libraries(${component}
    PUBLIC graphics
)


target_include_directories(${component} PRIVATE "${CMAKE_CURRENT_LIST_DIR}")
target_include_directories(${component} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/..")