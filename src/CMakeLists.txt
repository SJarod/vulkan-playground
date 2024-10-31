set(component src)

add_executable(${component})

target_sources(${component}
    PRIVATE
    main.cpp
)