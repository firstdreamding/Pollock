workspace "Pollock"
    architecture "x64"
    startproject "Sandbox"

    configurations
    {
        "Debug",
        "Release"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
include "Pollock/vendor/imgui"
group ""

project "Pollock"
    location "Pollock"
    kind "StaticLib"
    language "C++"
    staticruntime "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir  ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/vendor/glad/src/*.c",
        "%{prj.name}/vendor/yaml-cpp/src/*.cpp",
        "%{prj.name}/vendor/yaml-cpp/src/*.h",
        "%{prj.name}/vendor/yaml-cpp/include/*.h",
    }

    includedirs
    {
        "%{prj.name}/src",
		"%{prj.name}/vendor/glfw/include",
        "%{prj.name}/vendor/glad/include",
        "%{prj.name}/vendor/glm",
        "%{prj.name}/vendor/imgui",
        "%{prj.name}/vendor/yaml-cpp/include",
    }

    links
    {
        "ImGui"
	}

    filter "system:windows"
        cppdialect "C++17"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        optimize "On"

project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"
    staticruntime "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir  ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
    }

    includedirs
    {
        "%{prj.name}/src",
		"Pollock/src",
        "Pollock/vendor/glad/include",
		"Pollock/vendor/glm",
		"Pollock/vendor/imgui",
        "Pollock/vendor/yaml-cpp/include",
    }

    links
    {
        "Pollock",
		"Pollock/vendor/glfw/lib-vc2017/glfw3.lib",
		"opengl32.lib",
    }

    filter "system:windows"
        cppdialect "C++17"
        systemversion "latest"        
        
    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        optimize "On"