add_executable(dockertarballloader EXCLUDE_FROM_ALL dockertarballloader_standalone.cc)
target_link_libraries(dockertarballloader aktualizr_lib torizon_dockercompose_secondary)

aktualizr_source_file_checks(dockertarballloader_standalone.cc)
