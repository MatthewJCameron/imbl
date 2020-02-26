cmake_minimum_required(VERSION 3.0)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_CXX_STANDARD 11)


project(IMBLui
        VERSION 1.0)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 COMPONENTS Widgets Svg REQUIRED)
include_directories(${Qt5Widgets_INCLUDE_DIRS})

find_package(QwtQt5 6.0 REQUIRED)
include_directories(${QWT_INCLUDE_DIRS})

find_path(QEPICSPV_INC qtpv.h)
if(NOT QEPICSPV_INC)
  message(FATAL_ERROR ": Can't find folder containing qtpv.h")
endif()
include_directories(${QEPICSPV_INC})

find_path(QCAMOTOR_INC qcamotorgui.h)
if(NOT QCAMOTOR_INC)
  message(FATAL_ERROR ": Can't find folder containing qcamotorgui.h")
endif()
include_directories(${QCAMOTOR_INC})


include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(component/componentBase)
add_subdirectory(component/componentGui)

include_directories(PRIVATE ${CMAKE_SOURCE_DIR}/component/componentBase)
include_directories(PRIVATE ${CMAKE_SOURCE_DIR}/component/componentGui)
link_directories(PRIVATE ${CMAKE_SOURCE_DIR}/component/componentBase)
link_directories(PRIVATE ${CMAKE_SOURCE_DIR}/component/componentGui)

add_subdirectory(shutterFE/shutterFEbase)
add_subdirectory(shutterFE/shutterFEgui)
add_subdirectory(mono/monoBase)
add_subdirectory(mono/monoGui)
add_subdirectory(filters/filtersBase)
add_subdirectory(filters/filtersGui)
add_subdirectory(shutter1A/shutter1Abase)
add_subdirectory(shutter1A/shutter1Agui)
add_subdirectory(mrtShutter/mrtShutterBase)
add_subdirectory(mrtShutter/mrtShutterGui)
add_subdirectory(hutch)
add_subdirectory(valve)

add_subdirectory(qimbl)
