cmake_minimum_required(VERSION 3.10)
project(Project1)

set(CMAKE_CXX_STANDARD 17)

set(SOURCES
    main.cpp
    network_client.cpp
    email_ui.cpp
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_widgets.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/backends/imgui_impl_win32.cpp
    vendor/imgui/backends/imgui_impl_dx9.cpp  
)

add_executable(Project1 ${SOURCES})

target_include_directories(Project1 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    vendor/imgui
    vendor/imgui/backends
)

if (WIN32)
    target_link_libraries(Project1 ws2_32 d3d9)
elseif(UNIX)
    find_package(OpenGL REQUIRED)
    target_link_libraries(Project1 OpenGL::GL pthread)
endif()
