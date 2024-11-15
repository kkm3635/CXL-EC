# CMake 프로그램의 최소 버전
cmake_minimum_required(VERSION 3.11)

# 프로젝트 정보
project(
  ModooCode
  VERSION 0.1
  DESCRIPTION "CXL-EC simulator"
  LANGUAGES CXX)

# 소스 파일 설정
set(SOURCES
  main.cpp
  cxlEC.cpp
)

# 실행 파일 생성
add_executable (main ${SOURCES})

# include_directories(../ezpwd-reed-solomon/c++)

# 디버그 옵션 추가
target_compile_options(main PRIVATE -g)

# 헤더 파일 디렉토리 추가
target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR})

# 필요시 주소 검증 도구(Address Sanitizer) 활성화
# target_link_options(main PRIVATE -fsanitize=address)
