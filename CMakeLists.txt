cmake_minimum_required(VERSION 2.6)
PROJECT(MultiscreenManga)

SET(CMAKE_RUNTIME_OUTPUT_DIR ${PROJECT_BINARY_DIR}/bin)
SET(CMAKE_LIBRARY_OUTPUT_DIR ${PROJECT_BINARY_DIR}/lib)
SET(CMAKE_ARCHIVE_OUTPUT_DIR ${PROJECT_BINARY_DIR}/lib)

ADD_DEFINITIONS(-std=c++0x)
ADD_DEFINITIONS(-Werror)

# Include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)
# Autogen'd moc code appears in the BINARY_DIR I believe
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

# Source files
SET(MultiscreenManga_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mangavolume.cpp
    src/renderwidget.cpp
    src/configuration.cpp
    src/sidebar.cpp
    )
# Include files
SET(MultiscreenManga_HEADERS_TOMOC
    include/mainwindow.h
    include/mangavolume.h
    include/renderwidget.h
    include/sidebar.h
    )
SET(MultiscreenManga_HEADERS
    ${MultiscreenManga_HEADERS_TOMOC}
    include/configuration.h
    )
SET(MultiscreenManga_SOURCES ${MultiscreenManga_SOURCES} ${MultiscreenManga_HEADERS})

# Qt
SET(QT_QMAKE_EXECUTABLE qmake-qt4 qmake4 qmake)
FIND_PACKAGE(Qt4 REQUIRED)
SET(QT_USE_QTOPENGL TRUE)
QT4_WRAP_CPP(MultiscreenManga_HEADERS_MOC ${MultiscreenManga_HEADERS_TOMOC})
INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})

# Executable to build
ADD_EXECUTABLE(MultiscreenManga
    ${MultiscreenManga_SOURCES}
    ${MultiscreenManga_HEADERS}
    ${MultiscreenManga_HEADERS_MOC})
TARGET_LINK_LIBRARIES(MultiscreenManga ${QT_LIBRARIES} GL poppler-qt4 magic)

INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/MultiscreenManga DESTINATION bin)
