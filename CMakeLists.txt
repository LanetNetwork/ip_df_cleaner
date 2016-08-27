cmake_minimum_required(VERSION 2.8.11)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-scripts)
include(generic)

include(FindPkgConfig)
if (PKG_CONFIG_FOUND)
	pkg_check_modules(LIBNETFILTER_QUEUE REQUIRED libnetfilter_queue)
	if(NOT CMAKE_BUILD_TYPE MATCHES Debug)
		pkg_check_modules(LIBTCMALLOC_MINIMAL libtcmalloc_minimal)
		if(LIBTCMALLOC_MINIMAL_FOUND EQUAL 1)
			set(GB_LD_EXTRA "${LIBTCMALLOC_MINIMAL_LIBRARIES}")
		endif (LIBTCMALLOC_MINIMAL_FOUND EQUAL 1)
	endif(NOT CMAKE_BUILD_TYPE MATCHES Debug)
endif (PKG_CONFIG_FOUND)

add_executable(ip_df_cleaner
	ip_df_cleaner.c)

target_link_libraries(ip_df_cleaner
	${LIBNETFILTER_QUEUE_LIBRARIES}
	${GB_LD_EXTRA})

install(TARGETS ip_df_cleaner
	RUNTIME DESTINATION bin)

