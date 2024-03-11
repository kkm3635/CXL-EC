# CMake 프로그램의 최소 버전
cmake_minimum_required(VERSION 3.11)

# 프로젝트 정보
project(
  ModooCode
  VERSION 0.1
  DESCRIPTION "CXL-EC simulator"
  LANGUAGES CXX)

add_subdirectory(lib)

add_executable (main main.cpp)

include_directories(../ezpwd-reed-solomon/c++)

target_link_libraries(main cxlEC)