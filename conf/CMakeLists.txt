set(conf_dir "${MK_PATH_CONF}/plugins/duda/")
install(DIRECTORY DESTINATION ${conf_dir})

configure_file(
  "${PROJECT_SOURCE_DIR}/plugins/duda/conf/duda.conf.in"
  "${PROJECT_SOURCE_DIR}/plugins/duda/conf/duda.conf"
  )

if(BUILD_LOCAL)
  file(COPY duda.conf DESTINATION ${conf_dir})
else()
  install(FILES duda.conf DESTINATION ${conf_dir})
endif()
