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

if(CHARLS_ROOT)
  execute_process(COMMAND gdcmimg ${CHARLS_ROOT}/test/SIEMENS-MR-RGB-16Bits.dcm
                          ${CMAKE_CURRENT_BINARY_DIR}/SIEMENS-MR-RGB-16Bits.pgm)
  execute_process(COMMAND gdcmraw ${CHARLS_ROOT}/test/SIEMENS-MR-RGB-16Bits.dcm
                          ${CMAKE_CURRENT_BINARY_DIR}/SIEMENS-MR-RGB-16Bits.raw)
  execute_process(
    COMMAND
      gdcmconv --raw --planar-configuration 1
      ${CHARLS_ROOT}/test/SIEMENS-MR-RGB-16Bits.dcm
      ${CMAKE_CURRENT_BINARY_DIR}/SIEMENS-MR-RGB-16Bits_pc1.dcm)

  set(testdata SIEMENS-MR-RGB-16Bits)
  foreach(mode none line sample)
    # prepare data:
    execute_process(
      COMMAND dcmcjpls --interleave-${mode} ${CHARLS_ROOT}/test/MR2_UNC
              ${CMAKE_CURRENT_BINARY_DIR}/MR2_UNC_${mode}.dcm)
    execute_process(
      COMMAND gdcmraw ${CMAKE_CURRENT_BINARY_DIR}/MR2_UNC_${mode}.dcm
              ${CMAKE_CURRENT_BINARY_DIR}/MR2_UNC_${mode}.jls)

    # test
    add_test(NAME jplsinfo_MR2_UNC_${mode}
             COMMAND jplsinfo --pretty -i
                     ${CMAKE_CURRENT_BINARY_DIR}/MR2_UNC_${mode}.jls)

    # tests:
    add_test(
      NAME cjpls_${testdata}_${mode}
      COMMAND cjpls -m ${mode} -i ${CMAKE_CURRENT_BINARY_DIR}/${testdata}.pgm
              -o ${CMAKE_CURRENT_BINARY_DIR}/${testdata}_${mode}.jls)
    add_test(
      NAME djpls_${testdata}_${mode}
      COMMAND djpls -i ${CMAKE_CURRENT_BINARY_DIR}/${testdata}_${mode}.jls -o
              ${CMAKE_CURRENT_BINARY_DIR}/${testdata}_${mode}.pgm)
    set_tests_properties(djpls_${testdata}_${mode}
                         PROPERTIES DEPENDS cjpls_${testdata}_${mode})
    add_test(
      NAME round_trip_${testdata}_${mode}
      COMMAND
        ${CMAKE_COMMAND} -E compare_files
        ${CMAKE_CURRENT_BINARY_DIR}/${testdata}.pgm
        ${CMAKE_CURRENT_BINARY_DIR}/${testdata}_${mode}.pgm)
    set_tests_properties(round_trip_${testdata}_${mode}
                         PROPERTIES DEPENDS djpls_${testdata}_${mode})
  endforeach()

  foreach(banny banny_normal banny_HP1 banny_HP2 banny_HP3)
    add_test(NAME jplsinfo_${banny}
             COMMAND jplsinfo --pretty -i
                     ${CHARLS_ROOT}/test/jlsimage/${banny}.jls)
  endforeach()
endif()
