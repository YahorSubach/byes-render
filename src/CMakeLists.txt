target_sources(render_engine 
	PRIVATE
		${CMAKE_CURRENT_LIST_DIR}/render_engine.cc
		${CMAKE_CURRENT_LIST_DIR}/stl_util.h
		${CMAKE_CURRENT_LIST_DIR}/platform.h
		${CMAKE_CURRENT_LIST_DIR}/platform.cc
		${CMAKE_CURRENT_LIST_DIR}/surface.h
		${CMAKE_CURRENT_LIST_DIR}/surface.cc
		${CMAKE_CURRENT_LIST_DIR}/render/frame_handler.h
		${CMAKE_CURRENT_LIST_DIR}/render/frame_handler.cc
		${CMAKE_CURRENT_LIST_DIR}/render/validate_object.h
		${CMAKE_CURRENT_LIST_DIR}/render/vk_util.h)
                                  
