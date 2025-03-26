cmake_minimum_required(VERSION 3.20)

set(TARGET_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(EXCLUDE_FILES include)
list(APPEND EXCLUDE_FILES C_component)

add_subdirectory(C_component)

file(GLOB SUB_FILES ${TARGET_DIR}/*)
foreach (SUB_FILE ${SUB_FILES})
    file(RELATIVE_PATH FILE_NAME ${TARGET_DIR} ${SUB_FILE})
    if (IS_DIRECTORY "${TARGET_DIR}/${FILE_NAME}")
        string(COMPARE EQUAL ${FILE_NAME} CMakeFiles VAR)
        # if (${FILE_NAME} STREQUAL CMakeFiles)
        if (NOT ${VAR})
            set(OK ON)
            foreach (ITEM ${EXCLUDE_FILES})
                string(COMPARE EQUAL ${FILE_NAME} ${ITEM} VAR)
                if (${VAR})
                    set(OK OFF)
                    break()
                endif ()
            endforeach ()
            if (${OK})
                include_directories(
                        ${TARGET_DIR}/${FILE_NAME}/include
                )
                add_subdirectory(${FILE_NAME})
            endif ()
        endif ()
    endif ()
endforeach ()

TargetDependency(Semaphore ${TARGET_DIR} Bitmap)
TargetDependency(Mutex ${TARGET_DIR} Semaphore)
TargetDependency(ConditionVariable ${TARGET_DIR} Semaphore)
TargetDependency(RWLock ${TARGET_DIR} Mutex Semaphore)
TargetDependency(ThreadPool ${TARGET_DIR} RWLock Mutex Semaphore Bitmap)
