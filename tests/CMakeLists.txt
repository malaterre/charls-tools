# tests

# version/help
foreach(cmd jplsinfo cjpls djpls)
  foreach(opt version help)
    add_test(NAME ${cmd}_${opt} COMMAND ${cmd} --${opt})
  endforeach()
endforeach()

# no input
add_test(NAME jplsinfo_invalid COMMAND jplsinfo -i /root/root/root)
set_tests_properties(jplsinfo_invalid PROPERTIES WILL_FAIL TRUE)

# charls-test-data:
if(CHARLS_TEST_DATA)
  set(t87_data
      t87/T16E0.JLS
      t87/T16E3.JLS
      t87/T8C0E0.JLS
      t87/T8C0E3.JLS
      t87/T8C1E0.JLS
      t87/T8C1E3.JLS
      t87/T8C2E0.JLS
      t87/T8C2E3.JLS
      t87/T8NDE0.JLS
      t87/T8NDE3.JLS
      # t87/T8SSE0.JLS t87/T8SSE3.JLS
  )
  set(random_data
      random/2bit-parrot-150x200.jls
      random/4bit-monochrome.jls
      random/banny_HP1.jls
      random/banny_HP2.jls
      random/banny_HP3.jls
      random/banny_normal.jls
      random/tulips-gray-8bit-512-512.jls)
  foreach(dir jplsinfo djpls cjpls roundtrip)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${dir}/t87)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${dir}/random)
  endforeach()
  foreach(filename ${t87_data} ${random_data})
    get_filename_component(testname ${filename} NAME_WE)
    get_filename_component(dirname ${filename} DIRECTORY)
    # jplsinfo:
    add_test(
      NAME jplsinfo_${testname}
      COMMAND
        jplsinfo --pretty --format json --hash crc32 -i
        ${CHARLS_TEST_DATA}/data/${filename} -o
        ${CMAKE_CURRENT_BINARY_DIR}/jplsinfo/${dirname}/${testname}.json)
    add_test(
      NAME jplsinfo_${testname}_compare
      COMMAND
        ${CMAKE_COMMAND} -E compare_files
        ${CHARLS_TEST_DATA}/info/${dirname}/${testname}.json
        ${CMAKE_CURRENT_BINARY_DIR}/jplsinfo/${dirname}/${testname}.json)
    # roundtrip: djpls -> cjpls
    add_test(
      NAME djpls_${testname}
      COMMAND djpls -i ${CHARLS_TEST_DATA}/data/${filename} -o
              ${CMAKE_CURRENT_BINARY_DIR}/djpls/${dirname}/${testname}.ppm)
    add_test(
      NAME cjpls_${testname}
      COMMAND
        cjpls -i ${CMAKE_CURRENT_BINARY_DIR}/djpls/${dirname}/${testname}.ppm
        -o ${CMAKE_CURRENT_BINARY_DIR}/cjpls/${dirname}/${testname}.jls)
    add_test(
      NAME jplsinfo_roundtrip_${testname}
      COMMAND
        jplsinfo --pretty --format json --hash crc32 -i
        ${CMAKE_CURRENT_BINARY_DIR}/cjpls/${dirname}/${testname}.jls -o
        ${CMAKE_CURRENT_BINARY_DIR}/roundtrip/${dirname}/${testname}.json)
    # add_test( NAME jplsinfo_roundtrip_${testname}_compare COMMAND
    # ${CMAKE_COMMAND} -E compare_files
    # ${CHARLS_TEST_DATA}/info/${dirname}/${testname}.json
    # ${CMAKE_CURRENT_BINARY_DIR}/roundtrip/${dirname}/${testname}.json)
  endforeach()
endif()
