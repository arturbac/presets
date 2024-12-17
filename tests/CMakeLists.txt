  CPMAddPackage(
    ut
    GITHUB_REPOSITORY arturbac/ut-ext
    GIT_TAG        v2.0.1_7
  )
# add_custom_target(copy_jsons ALL
#     COMMAND ${CMAKE_COMMAND} -E copy_if_different
#     ${CMAKE_CURRENT_SOURCE_DIR}/presets
#     ${CMAKE_CURRENT_BINARY_DIR}
# )
add_custom_target(copy_presets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/presets"
        "${CMAKE_CURRENT_BINARY_DIR}/presets"
    COMMENT "Copying presets directory to build directory"
    VERBATIM
)
add_ut_test(presets_ut.cc)
target_compile_definitions( presets_ut PRIVATE BOOST_UT_DISABLE_MODULE)
add_dependencies(presets_ut copy_presets)
