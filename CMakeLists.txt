set(src
  src/duda.c
  src/duda_gc.c
  src/duda_mem.c
  src/duda_conf.c
  src/duda_api.c
  src/duda_event.c
  src/duda_body_buffer.c
  src/duda_sendfile.c
  src/duda_debug.c
  src/duda_global.c
  src/duda_package.c
  src/duda_param.c
  src/duda_session.c
  src/duda_cookie.c
  src/duda_request.c
  src/duda_response.c
  src/duda_queue.c
  src/duda_console.c
  src/duda_log.c
  src/duda_qs.c
  src/duda_worker.c
  src/duda_stats.c
  src/duda_fconf.c
  src/duda_data.c
  src/duda_xtime.c
  src/duda_utils.c
  src/duda_dthread.c
  src/duda_router.c
  src/duda_dthread_channel.c
  )

MONKEY_PLUGIN(duda "${src}")

#add_subdirectory(src)

#add_subdirectory(packages)
#add_subdirectory(docs)
