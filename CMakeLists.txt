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
    src/savestate.cpp
    src/remotesync.cpp
    )
# Include files
SET(MultiscreenManga_HEADERS_TOMOC
    include/mainwindow.h
    include/mangavolume.h
    include/renderwidget.h
    include/sidebar.h
    include/savestate.h
    include/remotesync.h
    )
SET(MultiscreenManga_HEADERS
    ${MultiscreenManga_HEADERS_TOMOC}
    include/configuration.h
    )
SET(MultiscreenManga_SOURCES ${MultiscreenManga_SOURCES} ${MultiscreenManga_HEADERS})

# Qt
#SET(CMAKE_INCLUDE_CURRENT_DIR ON)
#SET(CMAKE_AUTOMOC ON)
#FIND_PACKAGE(Qt5Widgets)
##INCLUDE_DIRECTORIES(${Qt5Widgets_INCLUDES})
##ADD_DEFINITIONS(${Qt5Widgets_Definitions})
##SET(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

#SET(QT_QMAKE_EXECUTABLE qmake-qt4 qmake4 qmake)
FIND_PACKAGE(Qt4 REQUIRED)
SET(QT_USE_QTOPENGL TRUE)
QT4_WRAP_CPP(MultiscreenManga_HEADERS_MOC ${MultiscreenManga_HEADERS_TOMOC})
INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})

FIND_PACKAGE(Protobuf)



IF ( PROTOBUF_FOUND )
    ADD_DEFINITIONS( -DUSE_SERIALIZATION )
    FIND_LIBRARY(ZEROMQ_LIB LIBRARY NAMES zmq )
    FIND_PATH(ZEROMQ_INCLUDE LIBRARY NAMES zmq.hpp)
    IF(ZEROMQ_INCLUDE)
        ADD_DEFINITIONS( -DUSE_NETWORKING )
    ELSE()
        IF(ZEROMQ_LIB)
            MESSAGE(No zeromq cpp bindings, please install)
        ELSE()
            MESSAGE(No networking support, install zeromq and its cpp bindings)
        ENDIF(ZEROMQ_LIB)
    ENDIF(ZEROMQ_INCLUDE)
    SET(NETWORK_LIBS ${PROTOBUF_LIBRARY} zmq )
    INCLUDE_DIRECTORIES( ${PROTOBUF_INCLUDE_DIR} )
    SET(PROTOBUF_IMPORT_DIRS protobuf)
    PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS protobuf/savestate.proto)
ELSE()
    MESSAGE(Don't have both protobuf no serialization or network sync support added.)
ENDIF ( PROTOBUF_FOUND )
    


# Executable to build
ADD_EXECUTABLE(MultiscreenManga
    ${MultiscreenManga_SOURCES}
    ${MultiscreenManga_HEADERS}
        ${MultiscreenManga_HEADERS_MOC}
        ${PROTO_SRCS} ${PROTO_HDRS}
    )

TARGET_LINK_LIBRARIES(MultiscreenManga ${QT_LIBRARIES} GL poppler-qt4 magic ${NETWORK_LIBS}  )

#TARGET_LINK_LIBRARIES(MultiscreenManga GL poppler-qt4 magic)
#TARGET_LINK_LIBRARIES(MultiscreenManga ${Qt5Widgets_LIBRARIES} GL poppler-qt4 magic)
#QT5_USE_MODULES(MultiscreenManga Widgets)
#QT5_USE_MODULES(MultiscreenManga Core)

INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/MultiscreenManga DESTINATION bin)
