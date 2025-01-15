cmake_minimum_required(VERSION 3.30)

set(TARGET_DIR ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB SUB_FILES ${TARGET_DIR}/*)
foreach (SUB_FILE ${SUB_FILES})
    file(RELATIVE_PATH FILE_NAME ${TARGET_DIR} ${SUB_FILE})
    if (IS_DIRECTORY "${TARGET_DIR}/${FILE_NAME}")
        string(COMPARE EQUAL ${FILE_NAME} CMakeFiles VAR)
        # if (${FILE_NAME} STREQUAL CMakeFiles)
        if (NOT ${VAR})
            include_directories(
                    ${TARGET_DIR}/${FILE_NAME}/include
            )
            add_subdirectory(${FILE_NAME})
        endif ()
    endif ()
endforeach ()

TargetDependency(Mutex ${TARGET_DIR} Bitmap Semaphore)
TargetDependency(RWLock ${TARGET_DIR} Mutex Semaphore Bitmap)
TargetDependency(ThreadPool ${TARGET_DIR} RWLock Mutex Semaphore Bitmap)
