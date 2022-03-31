# https://jeromebelleman.gitlab.io/posts/publishing/manpages/
find_program(PANDOC_EXECUTABLE pandoc)
# TODO, reformat: pandoc -f markdown -t gfm clean.md

foreach(manpage jplsinfo cjpls djpls jplstran)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${manpage}.md.in
                 ${CMAKE_CURRENT_BINARY_DIR}/${manpage}.md @ONLY)

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${manpage}.1
    COMMAND ${PANDOC_EXECUTABLE} -o ${CMAKE_CURRENT_BINARY_DIR}/${manpage}.1 -s
            -t man ${CMAKE_CURRENT_BINARY_DIR}/${manpage}.md
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${manpage}.md)
  list(APPEND manpages_output ${CMAKE_CURRENT_BINARY_DIR}/${manpage}.1)
endforeach()
add_custom_target(manpages ALL DEPENDS ${manpages_output})
install(FILES ${manpages_output} DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
