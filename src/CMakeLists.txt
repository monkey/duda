set(src
  duda.c)

add_executable(duda ${src})
target_link_libraries(duda duda-static)
