cmake_minimum_required (VERSION 3.9)
project (vulkan_visual_facade)

find_package(Vulkan REQUIRED)

add_executable(vulkan_concepts "")


if(Vulkan_FOUND)
	message(STATUS "vulkan found") 
	message(${Vulkan_LIBRARIES})
	target_link_libraries(vulkan_concepts PRIVATE Vulkan::Vulkan)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT vulkan_concepts)	

	add_subdirectory(src)
	add_subdirectory(include/vulkan_concepts)

	target_include_directories(vulkan_concepts PRIVATE include/vulkan_concepts)


	if (WIN32)
		target_compile_definitions(vulkan_concepts PRIVATE VK_USE_PLATFORM_WIN32_KHR)
	endif()	
                                                
	install(TARGETS vulkan_concepts DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/install)
	
else()
	message(STATUS "vulkan not found")
endif()



                                  
