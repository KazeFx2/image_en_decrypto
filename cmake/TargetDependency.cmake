function(TargetDependency TARGET CURRENT_DIR)
    list(POP_FRONT ARGV TARGET CURRENT_DIR)
    foreach (DEPENDENT ${ARGV})
#        target_include_directories(
#                Core${TARGET}
#                PUBLIC
#                ${CURRENT_DIR}/${DEPENDENT}/include
#        )
        target_link_libraries(
                Core${TARGET}
                Core${DEPENDENT}
        )
    endforeach ()
endfunction()
