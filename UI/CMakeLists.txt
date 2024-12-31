cmake_minimum_required(VERSION 3.30)

project(ImageEn_DecryptoUI)

set(CMAKE_CXX_STANDARD 17)

set(Qt5_DIR /usr/local/Cellar/qt@5/5.15.16/lib/cmake/Qt5)

find_package(Qt5 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED
)

include_directories(
        include
)
