# 
# TESTBASE Device definitions
# 
#     !!! Should be modified according to the device !!!
# 
set(TESTBASE_DEVICE_ID 2820)
set(TESTBASE_REGISTER_VERSION 07)

# Include TESTBASE common CMakeLists.txt content
include(${PROJECT_SOURCE_DIR}/cmake/TESTBASE.CMakeLists.txt.cmake)

# 
# TESTBASE Device API header files and library to install.
# 
#     !!! Should be modified according to the device !!!
# 
install(TARGETS ${TARGET_NAME}
        LIBRARY DESTINATION lib/TESTBASE/${DEVICE_NAME}/v${TESTBASE_REGISTER_VERSION}
        )

install(FILES 
        TB2820_Api.h
    ${MODULE_ABBR_NAME}Version.h DESTINATION include/TESTBASE/${DEVICE_NAME}/v${TESTBASE_REGISTER_VERSION}
    )

# 
# TESTBASE Device API source files for debug.
# 
#     !!! Should be modified according to the device !!!
# 
configure_file(${PROJECT_SOURCE_DIR}/cmake/TESTBASE.Makefile.in ${PROJECT_BINARY_DIR}/TESTBASE/${TARGET_NAME}/Makefile)
file(COPY 
        ${CMAKE_CURRENT_SOURCE_DIR}/TB2820_Api.c
        ${CMAKE_CURRENT_SOURCE_DIR}/TB2820_Api.h
        ${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_ABBR_NAME}Version.h
        ${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_ABBR_NAME}Version.c
        ${PROJECT_SOURCE_DIR}/src/Common/include/Register.h
        ${PROJECT_SOURCE_DIR}/src/Common/include/CommonTypes.h
        ${PROJECT_SOURCE_DIR}/src/Common/include/TESTBASE.h
     DESTINATION ${PROJECT_BINARY_DIR}/TESTBASE/${TARGET_NAME}
)