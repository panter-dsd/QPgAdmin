set (PROJECT_OUTPUT_NAME QPgAdmin)
project(${PROJECT_OUTPUT_NAME})

cmake_minimum_required( VERSION 2.6 )

#SET (CMAKE_BUILD_TYPE Release)
SET (CMAKE_BUILD_TYPE Debug)

INCLUDE(FindQt4)
find_package( Qt4 REQUIRED )
SET(QT_USE_QTSQL 1)
include( ${QT_USE_FILE})
add_definitions(-DUNICODE)

include_directories(
src
src/widgets
src/dialogs
)

################################################################
# src
################################################################

set (src_SRC
src/main.cpp
src/mainwindow.cpp
)

set (src_HEADERS
src/mainwindow.h
)

################################################################
# widgets
################################################################

set (widgets_SRC
src/widgets/databasetree.cpp
src/widgets/edittablewidget.cpp
)

set (widgets_HEADERS
src/widgets/databasetree.h
src/widgets/edittablewidget.h
)

################################################################
# dialogs
################################################################

set (dialogs_SRC
src/dialogs/connectiondialog.cpp
)

set (dialogs_HEADERS
src/dialogs/connectiondialog.h
)

################################################################
# all
################################################################
set(
SOURCES
${src_SRC}
${widgets_SRC}
${dialogs_SRC}
)

set(
HEADERS
${src_HEADERS}
${widgets_HEADERS}
${dialogs_HEADERS}
)

qt4_wrap_cpp( MOC_SOURCES ${HEADERS} )

set( RESOURCES resource.qrc )

qt4_add_resources( QRC_SOURCES ${RESOURCES} )

if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
	add_executable( ${PROJECT_OUTPUT_NAME} ${SOURCES} ${MOC_SOURCES} ${QRC_SOURCES})
else()
	add_executable( ${PROJECT_OUTPUT_NAME} WIN32 ${SOURCES} ${MOC_SOURCES} ${QRC_SOURCES})
endif()

target_link_libraries( ${PROJECT_OUTPUT_NAME} ${QT_LIBRARIES} )
