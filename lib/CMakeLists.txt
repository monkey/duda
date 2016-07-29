set(src
  # Library core
  duda.c
  duda_service.c
  duda_request.c
  duda_old_launcher.c
  duda_conf.c
  duda_api.c
  duda_event.c
  duda_body_buffer.c
  duda_sendfile.c
  duda_debug.c
  duda_package.c
  duda_queue.c
  duda_stats.c
  duda_fconf.c
  duda_utils.c

  # API Objects
  objects/duda_gc.c
  objects/duda_log.c
  objects/duda_param.c
  objects/duda_session.c
  objects/duda_cookie.c
  objects/duda_global.c
  objects/duda_request.c
  objects/duda_response.c
  objects/duda_mem.c
  objects/duda_console.c
  objects/duda_qs.c
  objects/duda_worker.c
  objects/duda_data.c
  objects/duda_xtime.c
  objects/duda_dthread.c
  objects/duda_dthread_channel.c
  objects/duda_router.c
  )

add_definitions(-DDUDA_LIB_CORE)
add_library(duda-static STATIC ${src})
target_link_libraries(duda-static pthread dl monkey-core-static)
set_target_properties(duda-static PROPERTIES OUTPUT_NAME duda)
