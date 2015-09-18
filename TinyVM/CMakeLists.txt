project(TinyVM CXX)
cmake_minimum_required(VERSION 3.0)

set(TVM_VERSION_MAJOR 0)
set(TVM_VERSION_MINOR 0)
set(TVM_VERSION_REV   5)

# Windows-specific options
if(WIN32)
	option(ENABLE_WIN32_FILEMAPPING_MMAP "Enable FileMapping mmap optimisation" ON)
endif()

if(UNIX)
	option(ENABLE_POSIX_FILEMAPPING_MMAP "Enable FileMapping mmap optimisations" ON)
endif()

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
	vm.hpp
	instruction.hpp
	instruction_implementation.hpp
	instruction_support.hpp
	vmtypes.hpp)

include_directories("${PROJECT_BINARY_DIR}")
add_executable(${PROJECT_NAME} ${SRC_LIST} ${HDR_LIST})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
