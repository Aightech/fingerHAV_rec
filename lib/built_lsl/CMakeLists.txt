cmake_minimum_required(VERSION 3.3)

###### LIBRARY NAME AND VERSION ######
include(CMakeConfig.txt)

if(NOT DEFINED DIR_DEPTH)
  set(DIR_DEPTH 0)
else ()
  MATH(EXPR DIR_DEPTH "${DIR_DEPTH}+1")
endif()
string(REPEAT "\t" ${DIR_DEPTH} PREFIX_MSG)
MATH(EXPR color_code "${DIR_DEPTH}+33")
MATH(EXPR color_code2 "${DIR_DEPTH}+34")

if (TARGET ${LIB_NAME})
  message("${PREFIX_MSG}-- ${ColourBold}${Esc}[${color_code}m${LIB_NAME}${ColourReset} is already built")
else ()

  # ###### C++ standard ######
  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED True)
  
  add_library(${LIB_NAME} SHARED IMPORTED GLOBAL)
  set_target_properties( ${LIB_NAME} PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/bin/liblsl64.so" )
  set_target_properties( ${LIB_NAME} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include" )

endif ()
