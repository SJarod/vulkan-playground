# **Vulkan-RHI-Sandbox**

Simple RHI.

## Summary
- [Getting started](#getting-started)
    - [Installation](#installation)
- [Branches](#branches)
- [Third-parties](#third-parties)

# Getting started

## Installation
This is a CMake-based project so make sure to install CMake ([portable version](https://github.com/Kitware/CMake/releases/download/v3.26.0-rc5/cmake-3.26.0-rc5-windows-x86_64.zip) : do not forget to add bin directory to PATH).

Download the [latest Vulkan SDK](https://sdk.lunarg.com/sdk/download/latest/windows/vulkan-sdk.exe) from [LunarG's website](https://vulkan.lunarg.com/sdk/home#), it is used to make the Vulkan validations layers available.

# Branches

## master
- loading the vulkan functions from the dll manually

## architecture/volk
- loading the vulkan functions with volk as a submodule
- building the validation layers from a submodule

## vulkan-sdk
- vulkan functions and layers from Vulkan SDK
- glslc is taken from the SDK

# Third-parties
- GLFW
    - https://www.glfw.org/
- CMake
    - https://cmake.org/
- Vulkan SDK
    - https://vulkan.lunarg.com/
- glm
    - https://github.com/g-truc/glm