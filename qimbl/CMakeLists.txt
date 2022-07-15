
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/imbl")

add_executable(qimbl
  main.cpp
  qimbl.h
  qimbl.cpp
  qimbl.ui
  qimbl.qrc
  columnresizer.h
  columnresizer.cpp
)

target_include_directories(qimbl
  PRIVATE ../valve
  PRIVATE ../hutch
  PRIVATE ../shutterFE/shutterFEgui
  PRIVATE ../shutterFE/shutterFEbase
  PRIVATE ../filters/filtersBase
  PRIVATE ../filters/filtersGui
  PRIVATE ../mono/monoGui
  PRIVATE ../mono/monoBase
  PRIVATE ../expander/expanderGui
  PRIVATE ../expander/expanderBase
  PRIVATE ../shutter1A/shutter1Agui
  PRIVATE ../shutter1A/shutter1Abase
  PRIVATE ../mrtShutter/mrtShutterGui
  PRIVATE ../mrtShutter/mrtShutterBase
  PRIVATE ../shutterIS/shutterISgui
  PRIVATE ../shutterIS/shutterISbase
  PRIVATE ../component/componentBase
  PRIVATE ../component/componentGui
)

target_link_libraries(qimbl
  Qt5::Widgets  
  Qt5::Svg
  qtpv
  qtpvwidgets
  valve
  hutch
  shutterFE
  shutterFEgui
  filters
  filtersGui
  mono
  monoGui
  expander
  expanderGui
  shutter1A
  shutter1Agui
  shutterIS
  shutterISgui
  mrtShutter
  mrtShutterGui
  component
  componentGui
)


install(TARGETS qimbl
    DESTINATION bin
)

install(FILES imbl.png
    DESTINATION share/icons
)

install(FILES IMBL.desktop
    DESTINATION share/applications
)
