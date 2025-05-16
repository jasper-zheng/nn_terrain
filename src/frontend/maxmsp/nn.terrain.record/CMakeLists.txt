# Copyright 2018 The Min-DevKit Authors. All rights reserved.
# Use of this source code is governed by the MIT License found in the License.md file.

cmake_minimum_required(VERSION 3.0)

set(C74_MIN_API_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../min-api)
include(${C74_MIN_API_DIR}/script/min-pretarget.cmake)

if (APPLE)
	set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0")
endif()

#############################################################
# MAX EXTERNAL
#############################################################


include_directories( 
	"${C74_INCLUDES}"
)



set(
SOURCE_FILES
	nn.terrain.record.cpp
    	"../shared/circular_buffer.h"
	"../shared/utils.h"
	"../shared/min_path.h"
	"../shared/min_dictionary.h"
)


add_library( 
	${PROJECT_NAME} 
	MODULE
	${SOURCE_FILES}
    	"../shared/circular_buffer.h"
	"../shared/utils.h"
	"../shared/min_path.h"
	"../shared/min_dictionary.h"
)
if (APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework CoreFoundation")
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-Wl,-F/Library/Frameworks")
endif()

include(${C74_MIN_API_DIR}/script/min-posttarget.cmake)


