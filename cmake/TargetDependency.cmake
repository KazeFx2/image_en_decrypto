function(BuildExportSize PROJECT_NAME SIZE_SOURCE)
    list(POP_FRONT ARGV PROJECT_NAME SIZE_SOURCE)
    add_library(${PROJECT_NAME}_impl STATIC ${ARGV})
    target_include_directories(
            ${PROJECT_NAME}_impl
            PRIVATE
            include/private
    )
    target_include_directories(
            ${PROJECT_NAME}_impl
            PUBLIC
            ../include/public
    )
    target_include_directories(
            ${PROJECT_NAME}_impl
            INTERFACE
            include/public
    )

    add_executable(
            ${PROJECT_NAME}_size
            ${SIZE_SOURCE})
    target_link_libraries(
            ${PROJECT_NAME}_size
            PRIVATE
            ${PROJECT_NAME}_impl
    )
    target_include_directories(
            ${PROJECT_NAME}_size
            PRIVATE
            ../include/private
    )

    add_custom_target(
            run_${PROJECT_NAME}_size
            COMMAND ${PROJECT_NAME}_size > ${CMAKE_CURRENT_SOURCE_DIR}/../include/public/${PROJECT_NAME}_private.h
            DEPENDS ${PROJECT_NAME}_size
            COMMENT "Generating '${PROJECT_NAME}' private types..."
    )

    add_library(
            ${PROJECT_NAME}
            INTERFACE
    )
    target_link_libraries(
            ${PROJECT_NAME}
            INTERFACE
            ${PROJECT_NAME}_impl
    )
    add_dependencies(
            ${PROJECT_NAME}
            run_${PROJECT_NAME}_size)
endfunction()

function(TargetDependency TARGET CURRENT_DIR)
    list(POP_FRONT ARGV TARGET CURRENT_DIR)
    foreach (DEPENDENT ${ARGV})
        #        target_include_directories(
        #                Core${TARGET}
        #                PUBLIC
        #                ${CURRENT_DIR}/${DEPENDENT}/include
        #        )
        target_link_libraries(
                Core${TARGET}_impl
                PUBLIC
                Core${DEPENDENT}
        )
    endforeach ()
endfunction()
