cmake_minimum_required (VERSION 3.9)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project (render_engine)

find_package(Vulkan REQUIRED)

if(Vulkan_FOUND)

	add_subdirectory(submodules/glm)
	add_subdirectory(submodules/freetype)

	add_library(render_engine "")

	target_link_libraries(render_engine PRIVATE Vulkan::Vulkan)

	add_subdirectory(src)
	
	get_target_property (_render_engine_sources render_engine SOURCES)
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src FILES ${_render_engine_sources}) 


	target_sources(render_engine 
		PRIVATE
			include/render/render_engine.h)

	target_include_directories(render_engine PUBLIC include)
	target_include_directories(render_engine PUBLIC submodules/glm)
	target_include_directories(render_engine PUBLIC submodules/tinygltf)
	target_include_directories(render_engine PUBLIC submodules/byes-static-allocator)

	target_include_directories(render_engine PRIVATE src)
	target_include_directories(render_engine PRIVATE submodules)

	if (WIN32)
		target_compile_definitions(render_engine PRIVATE VK_USE_PLATFORM_WIN32_KHR)

		add_custom_target(shaders
			COMMAND cmd /c "${CMAKE_CURRENT_SOURCE_DIR}/shaders/compile.bat"
			COMMENT "compiling shaders..."
		)
		
		add_subdirectory(shaders)

	endif()	


	target_link_libraries(render_engine PRIVATE glm)
	target_link_libraries(render_engine PRIVATE freetype)

	add_executable(render_engine_example "")
	add_dependencies(render_engine_example shaders)
	target_link_libraries(render_engine_example PRIVATE render_engine)


	add_subdirectory(examples)

	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT render_engine_example)	
	  	  
	install(TARGETS render_engine_example DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/install)

else()
	message(STATUS "vulkan not found")
endif()



                                  
