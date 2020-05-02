workspace "sandbox"

-- Includes ridge from source
-- ridge is one dir up
include ("../")

project "sandbox"
	kind		"ConsoleApp"
	language	"C"
	targetdir	"../bin"
	objdir		"../obj"

	-- Add C and H files
	files { "src/**.c", "src/**.h" }

	-- Add shaders to compilation
	files { "../assets/shaders/**.vert", "../assets/shaders/**.frag" }	

	-- Link ridge and GLFW
	-- Vulkan is linked by ridge
	links { "ridge", "GLFW" }
