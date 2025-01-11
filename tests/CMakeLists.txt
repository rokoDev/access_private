cmake_minimum_required(VERSION ${cmake_version})

set(ProjectName ${ProjectName}_tests)
project(${ProjectName})

function(package_add_test)
  set(prefix ARG)
  set(noValues)
  set(singleValues TEST_TARGET_NAME)
  set(multiValues
    TEST_SOURCES
    EXTRA_TARGETS
    DEFS
    )

  cmake_parse_arguments(${prefix}
                        "${noValues}"
                        "${singleValues}"
                        "${multiValues}"
                        ${ARGN})

  foreach(arg IN LISTS noValues singleValues multiValues)
      set(${arg} ${${prefix}_${arg}})
  endforeach()

  add_executable(${TEST_TARGET_NAME})
  target_sources(${TEST_TARGET_NAME} PRIVATE ${TEST_SOURCES})
  target_include_directories(${TEST_TARGET_NAME} PUBLIC include)
  foreach(target_to_link IN LISTS EXTRA_TARGETS)
      target_link_libraries(${TEST_TARGET_NAME} PRIVATE ${target_to_link})
  endforeach()

  foreach(define IN LISTS DEFS)
      target_compile_definitions(${TEST_TARGET_NAME} PRIVATE ${define})
  endforeach()

  add_test(NAME ${TEST_TARGET_NAME} COMMAND ${TEST_TARGET_NAME})

  # Place all tests targets under "test" source group in IDE
  set_target_properties(${TEST_TARGET_NAME} PROPERTIES FOLDER tests)

  # If colours in googletest are not disabled then in some versions of commandline tools which is used in Xcode 13.3 this
  # can lead to output with escape sequences. Because of that its better to disable colours in googletest.
  set_target_properties(${TEST_TARGET_NAME} PROPERTIES XCODE_GENERATE_SCHEME ON)
  set_target_properties(${TEST_TARGET_NAME} PROPERTIES XCODE_SCHEME_ARGUMENTS "--gtest_color=no")
endfunction()

set(test_src
  src/test.cpp
  )
package_add_test(
  TEST_TARGET_NAME access_private_tests
  TEST_SOURCES ${test_src}
  EXTRA_TARGETS access_private
  )

set(test_src
  include/a.hpp
  src/x.cpp
  src/y.cpp
  )
package_add_test(
  TEST_TARGET_NAME access_private_x_y_tests
  TEST_SOURCES ${test_src}
  EXTRA_TARGETS access_private
  )
