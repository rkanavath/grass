macro(build_gui_in_subdir dir_name)
  build_py_module(NAME ${dir_name}
    DST_DIR gui/wxpython
    TYPE "GUI")
endmacro()


macro(build_script_in_subdir dir_name)
  build_py_module(NAME ${dir_name}
    DST_DIR etc
    TYPE "SCRIPT")
endmacro()


function(build_py_module)
  cmake_parse_arguments(G
    ""
    "NAME;SRC_DIR;SRC_REGEX;DST_DIR;TYPE;HTML_FILE_NAME"
    "SOURCES;DEPENDS" ${ARGN} )

  if(NOT G_SRC_DIR)
    set(G_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${G_NAME})
  endif()

  if(NOT G_SRC_REGEX)
    set(G_SRC_REGEX "*.py")
  endif()

  if(NOT G_TYPE)
    message(FATAL_ERROR "TYPE argument is required")
  endif()

  set(types "GUI;SCRIPT")
  if(NOT "${G_TYPE}" IN_LIST types)
	message(FATAL_ERROR "TYPE is '${G_TYPE}'. Supported values are ${types}")
  endif()

  set(G_TARGET_NAME_PREFIX "")
  if(G_TYPE STREQUAL "GUI")
	set(G_TARGET_NAME_PREFIX "g.gui.")
  endif()
  
  set(G_TARGET_NAME ${G_TARGET_NAME_PREFIX}${G_NAME})
 
  if(G_HTML_FILE_NAME)
    set(HTML_FILE_NAME ${G_HTML_FILE_NAME})
  else()
    set(HTML_FILE_NAME ${G_TARGET_NAME})
  endif()

  
  file(GLOB PYTHON_FILES "${G_SRC_DIR}/${G_SRC_REGEX}")
  if(NOT PYTHON_FILES)
    message(FATAL_ERROR "[${G_TARGET_NAME}]: No PYTHON_FILES found.")
  endif()


#  message("PYTHON_FILES=${PYTHON_FILES}")
  ##################### TRANSLATE STRING FOR SCRIPTS AND GUI #####################
if(NOT PY_MODULE_FILE)
	set(PY_MODULE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${G_NAME}/${G_TARGET_NAME}.py)
	if(EXISTS "${PY_MODULE_FILE}")
		file(COPY ${PY_MODULE_FILE} DESTINATION ${CMAKE_BINARY_DIR}/scripts/)
		set(MAIN_SCRIPT_FILE ${CMAKE_BINARY_DIR}/scripts/${G_TARGET_NAME}.py)
	else()
	set(PY_MODULE_FILE "")
	set(MAIN_SCRIPT_FILE "")
	endif()
endif()

if(NOT G_TYPE STREQUAL "LIB")
	if (NOT EXISTS ${PY_MODULE_FILE})
		message(FATAL_ERROR "${PY_MODULE_FILE} does not exists")
    endif()
endif()#  if(NOT G_TYPE STREQUAL "LIB")

  ######################## TRANSLATE STRING FOR SCRIPTS #########################
  set(TRANSLATE_C_FILE "")

  if(G_TYPE STREQUAL "SCRIPT")
    set(TRANSLATE_C_FILE
      ${CMAKE_SOURCE_DIR}/locale/scriptstrings/${G_NAME}_to_translate.c)  

    add_custom_command(
      OUTPUT ${TRANSLATE_C_FILE}
      DEPENDS g.parser ${PY_MODULE_FILE}
      COMMAND ${CMAKE_COMMAND}
      -DINPUT_FILE=${MAIN_SCRIPT_FILE}
      -DOUTPUT_FILE=${TRANSLATE_C_FILE}
      -DBIN_DIR=${CMAKE_BINARY_DIR}
      -P ${CMAKE_SOURCE_DIR}/cmake/locale_strings.cmake
      COMMENT "Generating ${TRANSLATE_C_FILE}"
      VERBATIM)
  endif()

 ## message("Adding python taret ${G_TARGET_NAME}")


   if(G_TYPE STREQUAL "SCRIPT")
     add_custom_target(${G_TARGET_NAME} ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PY_MODULE_FILE} ${GISBASE}/scripts/
    DEPENDS ${TRANSLATE_C_FILE} )
   else()
  add_custom_target(${G_TARGET_NAME} ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${GISBASE}/${G_DST_DIR}/${G_NAME}/    
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PYTHON_FILES} ${GISBASE}/${G_DST_DIR}/${G_NAME}/
    DEPENDS ${PY_MODULE_FILE} )
	   endif()
  #get_property(MODULE_LIST GLOBAL PROPERTY MODULE_LIST)
  #add_dependencies(${G_NAME} ${MODULE_LIST})

  if(G_TYPE STREQUAL "GUI")
  set_target_properties (${G_TARGET_NAME} PROPERTIES FOLDER gui)
  endif()

  if(G_TYPE STREQUAL "SCRIPT")
  set_target_properties (${G_TARGET_NAME} PROPERTIES FOLDER scripts)
  endif()


  install(PROGRAMS ${MAIN_SCRIPT_FILE} DESTINATION scripts)


 if(WITH_DOCS)

  set_target_properties(${G_TARGET_NAME} PROPERTIES G_SRC_DIR "${G_SRC_DIR}")
  set_target_properties(${G_TARGET_NAME} PROPERTIES G_TARGET_FILE "${MAIN_SCRIPT_FILE}")
  set_target_properties(${G_TARGET_NAME} PROPERTIES RUN_HTML_DESCR TRUE)
  set_target_properties(${G_TARGET_NAME} PROPERTIES PYTHON_SCRIPT TRUE)
  set_target_properties(${G_TARGET_NAME} PROPERTIES G_RUNTIME_OUTPUT_DIR "${GISBASE}/scripts")
  set_target_properties(${G_TARGET_NAME} PROPERTIES G_HTML_FILE_NAME "${HTML_FILE_NAME}.html")

	build_docs(${G_TARGET_NAME})
	   
	#add_dependencies(${G_TARGET_NAME} pylib.script pylib.exceptions)
	add_dependencies(${G_TARGET_NAME} g.parser)

 endif(WITH_DOCS)

 install(FILES ${PYTHON_FILES} DESTINATION etc/${G_NAME})

endfunction()