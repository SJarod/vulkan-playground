set(component engine)

add_library(${component} STATIC "")

target_sources(${component}
    PRIVATE
    uniform.hpp

    vertex.hpp

    transform.hpp
    transform.cpp

    camera.hpp
    camera.cpp
)

target_link_libraries(${component}
    PUBLIC ${Vulkan_LIBRARY}
    PUBLIC glm
)

target_include_directories(${component} PUBLIC "${Vulkan_INCLUDE_DIR}")
target_include_directories(${component} PUBLIC "${glm_SOURCE_DIR}")

target_include_directories(${component} PRIVATE "${CMAKE_CURRENT_LIST_DIR}")
target_include_directories(${component} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/..")