cmake_minimum_required (VERSION 3.8)

add_definitions (-DUNICODE -DWIN32_LEAN_AND_MEAN -D_CRT_SECURE_NO_WARNINGS)

include_directories ($ENV{VULKAN_SDK}/Include)
include_directories (${CMAKE_SOURCE_DIR}/External/stb)

link_directories ($ENV{VULKAN_SDK}/Lib32)
link_directories (${CMAKE_SOURCE_DIR}/External/SDL/lib32)

set (H vulkan_graphics.h game.h utils.h log.h error.h types.h actor_vert.h actor_frag.h vulkan_compute.h vulkan_interface.h)
set (C vulkan_graphics.c game.c utils.c main.c log.c types.c vulkan_compute.c vulkan_interface.c)

add_executable (DOD WIN32 ${H} ${C})
target_link_libraries (DOD vulkan-1)
