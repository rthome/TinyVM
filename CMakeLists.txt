project(TinyVM CXX)
cmake_minimum_required(VERSION 3.0)

set(SRC_LIST
	main.cpp
	vm.cpp
	instruction.cpp)

set(HDR_LIST
	vm.hpp
	instruction.hpp
	vmtypes.hpp)

add_executable(${PROJECT_NAME} ${SRC_LIST} ${HDR_LIST})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
