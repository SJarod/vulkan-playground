set(component advanced_vulkan_test)

add_executable(${component})

target_sources(${component}
    PRIVATE
    main.cpp
)

target_link_libraries(${component}
    PRIVATE client
)

add_subdirectory(client)