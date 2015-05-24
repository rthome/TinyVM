project(TinyVM CXX)
cmake_minimum_required(VERSION 3.0)

set(SRC_LIST
	main.cpp
	vm.cpp)

set(HDR_LIST
	vm.hpp)

add_executable(${PROJECT_NAME} ${SRC_LIST} ${HDR_LIST})
