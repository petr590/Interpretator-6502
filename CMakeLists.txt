cmake_minimum_required(VERSION 3.3)
project(Interpretator6502 VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_SOURCE_DIR, src)

include_directories(include)

add_compile_options(
	-Wall
	-Wextra
	-Wpedantic

	-Wcast-align
	-Wcast-qual
	-Wenum-compare

	-Wctor-dtor-privacy
	-Wnon-virtual-dtor
	-Woverloaded-virtual
	-Wredundant-decls
	-Wsign-promo
	
	-Wduplicated-branches
	-Wduplicated-cond
	-Wshadow=compatible-local
	-Wzero-as-null-pointer-constant
#	-Wconversion
)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
	message("Debug build")
	
	add_compile_options(-g)
	add_compile_definitions(DEBUG)

elseif(CMAKE_BUILD_TYPE MATCHES "Release")
	message("Release build")
	
	add_compile_options(-O3)
	
else()
	message("Unknown build: " ${CMAKE_BUILD_TYPE})
endif()


set(SOURCES
	src/translator.cpp
	src/insn.cpp
	
	src/executor.cpp
	src/draw.cpp
	src/main.cpp
)


add_executable(int6502 ${SOURCES})

find_library(NCURSES_LIBRARY ncurses)
find_library(PTHREAD_LIBRARY pthread)

target_link_libraries(int6502 ${NCURSES_LIBRARY} ${PTHREAD_LIBRARY})


find_program(iwyu_path NAMES include-what-you-use iwyu)

if(NOT iwyu_path)
	message(FATAL_ERROR "Could not find the program include-what-you-use")
endif()

set_property(TARGET int6502 PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path})
