macro(set_compiler_flags)
if(MSVC)
  set(GRASS_C_FLAGS "/D_CRT_SECURE_NO_WARNINGS /DNOMINMAX")
  if(CMAKE_C_FLAGS)
    set(CMAKE_C_FLAGS "${GRASS_C_FLAGS} ${CMAKE_C_FLAGS}")
  endif()

  set(GRASS_CXX_FLAGS "/D_CRT_SECURE_NO_WARNINGS /DNOMINMAX")
  if(CMAKE_CXX_FLAGS)
    set(CMAKE_CXX_FLAGS "${GRASS_CXX_FLAGS} ${CMAKE_CXX_FLAGS}")
  endif()
endif()
endmacro()