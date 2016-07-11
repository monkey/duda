set(src
  duda.c)

add_definitions(-DDUDA_LIB_CORE)
add_executable(duda ${src})
target_link_libraries(duda duda-static)
