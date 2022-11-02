
add_library(expanderGui SHARED
  expanderGui.h
  expanderGui.cpp
  expanderGui.ui
  expanderGui.qrc
  tuner.h
  tuner.cpp
  tuner.ui
  energysetter.ui

)

target_include_directories(expanderGui
  PRIVATE ../expanderBase
)

target_link_libraries(expanderGui
  expander
  qcamotorgui
)

install(TARGETS expanderGui
    LIBRARY DESTINATION lib/imbl
)

install(FILES expanderGui.h
    DESTINATION include/imbl
)

