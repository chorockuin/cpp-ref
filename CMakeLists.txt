# 요구 CMake 최소 버전
cmake_minimum_required(VERSION 3.0)
 
# 프로젝트 이름 및 버전
project("cpp20")
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 1)
set(PROJECT_VERSION_PATCH 0)

# 빌드 형상(Configuration) 및 주절주절 Makefile 생성 여부
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_VERBOSE_MAKEFILE true)
 
# 빌드 대상 바이너리 파일명 및 소스 파일 목록
file(GLOB CPP20_SRC_FILES
     "*.h"
     "*.cpp"
)
set(CPP20_TARGET_FILE
     "${CMAKE_PROJECT_NAME}-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
)

if(WIN32)
     # 컴파일 옵션, 링크 옵션
     add_compile_options(/std:c++20 /Zi /EHsc)
elseif(UNIX)
     # 헤더 파일 Include 디렉토리 (-I)
     include_directories()

     # 링크 라이브러리 디렉토리 (-L)
     link_directories()

     # 컴파일러 선택
     set(CMAKE_C_COMPILER "/usr/bin/gcc-12")
     set(CMAKE_CXX_COMPILER "/usr/bin/gcc-12")

     # 컴파일 옵션, 링크 옵션
     add_compile_options(-std=gnu++20 -g -Wall -fcoroutines)

     # 링크 라이브러리 (-l)
     link_libraries(stdc++)
endif()

# "Debug" 형상 한정 컴파일 옵션, 링크 옵션
set(CMAKE_C_FLAGS_DEBUG "-DDEBUG -DC_FLAGS")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-DDEBUG -DLINKER_FLAGS")
 
# "Release" 형상 한정 컴파일 옵션, 링크 옵션
set(CMAKE_C_FLAGS_RELEASE "-DRELEASE -DC_FLAGS")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-DRELEASE -DLINKER_FLAGS")
 
# 출력 디렉토리
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BUILD_TYPE})
SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BUILD_TYPE}/lib)
SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BUILD_TYPE}/lib)

# 빌드 대상 바이너리 추가
add_executable(${CPP20_TARGET_FILE} ${CPP20_SRC_FILES})