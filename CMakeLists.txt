project(little_parser)

set(CXX_FLAGS
 -g
 -DDEBUG=1
 # -DVALGRIND
 # -DMUDUO_STD_STRING
 -DCHECK_PTHREAD_RETURN_VALUE
 -D_FILE_OFFSET_BITS=64
 -Wall
 -Wextra
 -Werror
 -Wconversion
 -Wno-unused-parameter
 -Wold-style-cast
 -Woverloaded-virtual
 -Wpointer-arith
 -Wshadow
 -Wwrite-strings
 -march=native
 # -MMD
 # -std=c++0x
 -rdynamic
 -std=c++11
 #-stdlib=libstdc++
 )

string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

add_executable(little_parser main.cpp lexer.cc parser.cc grammar.cc)
set_target_properties(little_parser PROPERTIES COMPILE_FLAGS "-std=c++11")
install(TARGETS little_parser RUNTIME DESTINATION bin)
