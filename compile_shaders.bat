@echo off
echo Compiling shaders...

%VULKAN_SDK%\Bin\glslc.exe shaders/shader.vert -o shaders/shader.vert.spv
%VULKAN_SDK%\Bin\glslc.exe shaders/shader.frag -o shaders/shader.frag.spv

echo Shader compilation completed. 