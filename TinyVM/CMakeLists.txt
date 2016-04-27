project(TinyVM CXX)
cmake_minimum_required(VERSION 2.8.7)

set(TVM_VERSION_MAJOR 0)
set(TVM_VERSION_MINOR 1)
set(TVM_VERSION_REV   0)

set(TVM_CONFIG_TEMPLATE_PATH "${PROJECT_SOURCE_DIR}/config.hpp.in")
set(TVM_CONFIG_HEADER_PATH "${PROJECT_BINARY_DIR}/config.hpp")
configure_file(${TVM_CONFIG_TEMPLATE_PATH} ${TVM_CONFIG_HEADER_PATH})

set(SRC_LIST
    main.cpp
    vm.cpp
    instruction.cpp
    instruction_implementation.cpp
    instruction_support.cpp)

set(HDR_LIST
    ${TVM_CONFIG_TEMPLATE_PATH}
    ${TVM_CONFIG_HEADER_PATH}
    util.hpp
    platform.hpp
    vm.hpp
    instruction.hpp
    instruction_implementation.hpp
    instruction_support.hpp
    vmtypes.hpp)

if (UNIX)
    set(SRC_LIST "${SRC_LIST}" platform_posix.cpp)
else()
    set(SRC_LIST "${SRC_LIST}" platform_generic.cpp)
endif()

include_directories("${PROJECT_BINARY_DIR}")
add_executable(${PROJECT_NAME} ${SRC_LIST} ${HDR_LIST})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
