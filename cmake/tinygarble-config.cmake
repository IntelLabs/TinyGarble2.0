find_package(emp-tool)

find_path(TINYGARBLE_DIRS tinygarble/emp-ag2pc.h)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(TINYGARBLE DEFAULT_MSG TINYGARBLE_DIRS)

if(TINYGARBLE_FOUND)
	set(TINYGARBLE_DIRS ${TINYGARBLE_DIRS} ${EMP-TOOL_INCLUDE_DIRS})
	set(TINYGARBLE_LIBRARIES ${EMP-TOOL_LIBRARIES})
endif()
