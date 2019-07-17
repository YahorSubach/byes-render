cmake_minimum_required (VERSION 3.9)
project (vulkan_visual_facade)

find_package(Vulkan REQUIRED)
if(Vulkan_FOUND)

	add_library(vk_visual_facade "")

	target_link_libraries(vk_visual_facade PRIVATE Vulkan::Vulkan)

	add_subdirectory(src)

	target_sources(vk_visual_facade 
		PRIVATE
			include/vkvf/vkvf.h)

	target_include_directories(vk_visual_facade PUBLIC include/vkvf)
	target_include_directories(vk_visual_facade PRIVATE src)

	if (WIN32)
		target_compile_definitions(vk_visual_facade PRIVATE VK_USE_PLATFORM_WIN32_KHR)
	endif()	
      
	  
	add_executable(vkvf_basics "")
	target_link_libraries(vkvf_basics PRIVATE vk_visual_facade)
	
	add_subdirectory(examples)


	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT vkvf_basics)	
	  
	install(TARGETS vkvf_basics DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/install)

else()
	message(STATUS "vulkan not found")
endif()



                                  
