target_sources(shaders
	PRIVATE
		${CMAKE_CURRENT_LIST_DIR}/compile.bat
		${CMAKE_CURRENT_LIST_DIR}/color.vert
		${CMAKE_CURRENT_LIST_DIR}/color_skin.vert
		${CMAKE_CURRENT_LIST_DIR}/color.frag
		${CMAKE_CURRENT_LIST_DIR}/ggx.glsl
		${CMAKE_CURRENT_LIST_DIR}/shadow.vert
		${CMAKE_CURRENT_LIST_DIR}/shadow_skin.vert
		${CMAKE_CURRENT_LIST_DIR}/shadow.frag
		${CMAKE_CURRENT_LIST_DIR}/ui.vert
		${CMAKE_CURRENT_LIST_DIR}/ui.frag
		${CMAKE_CURRENT_LIST_DIR}/collect_g_buffers.vert
		${CMAKE_CURRENT_LIST_DIR}/collect_g_buffers.frag
		${CMAKE_CURRENT_LIST_DIR}/build_g_buffers.vert
		${CMAKE_CURRENT_LIST_DIR}/build_g_buffers.frag
)
                                  
