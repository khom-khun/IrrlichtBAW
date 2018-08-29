
get_filename_component(EXECUTABLE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REGEX REPLACE "[0-9]+\." "" EXECUTABLE_NAME ${EXECUTABLE_NAME})

project(${EXECUTABLE_NAME})

add_executable(${EXECUTABLE_NAME} main.cpp ${EXTRA_SOURCES}) 
# EXTRA_SOURCES is var containing non-common names of sources (if any such sources, then EXTRA_SOURCES must be set before including this cmake code)
add_dependencies(${EXECUTABLE_NAME} Irrlicht)

target_include_directories(${EXECUTABLE_NAME} PUBLIC ../../include)
target_link_libraries(${EXECUTABLE_NAME} Irrlicht)

irr_adjust_flags() # macro defined in root CMakeLists
irr_adjust_definitions() # macro defined in root CMakeLists

set_target_properties(${EXECUTABLE_NAME} PROPERTIES DEBUG_POSTFIX _d)
set_target_properties(${EXECUTABLE_NAME}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "bin"
)
if(MSVC)
	set_target_properties(${EXECUTABLE_NAME}
		PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG "bin"
		RUNTIME_OUTPUT_DIRECTORY_RELEASE "bin"
	)
endif()