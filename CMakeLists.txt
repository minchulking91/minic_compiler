cmake_minimum_required(VERSION 3.6)
project(minic_compiler)

set(CMAKE_CXX_STANDARD 11)

find_package(FLEX)
find_package(BISON)

BISON_TARGET(MyParser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.c)
FLEX_TARGET(MyScanner scanner.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c)
ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)

add_executable(ast_gen Node.h ${BISON_MyParser_OUTPUTS} ${FLEX_MyScanner_OUTPUTS})

add_executable(ucode_gen main.c Node.h Symbol.h Ucode.h Symbol.c)