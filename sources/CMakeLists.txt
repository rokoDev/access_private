target_sources(access_private
	PRIVATE
    include/access_private/access_private.hpp
  )

target_include_directories(access_private INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
