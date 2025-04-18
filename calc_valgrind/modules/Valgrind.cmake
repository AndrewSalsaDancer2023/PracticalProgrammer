function(AddMemeoryCheck target)
  find_program(VALGRIND_PATH valgrind REQUIRED)
  add_custom_target(valgrind
    COMMAND ${VALGRIND_PATH} --leak-check=yes
            $<TARGET_FILE:${target}>
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )
endfunction()


function(AddThreadCheck target)
  find_program(VALGRIND_PATH valgrind REQUIRED)
  add_custom_target(threadcheck
    COMMAND ${VALGRIND_PATH} --tool=helgrind
            $<TARGET_FILE:${target}>
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )
endfunction()
