
add_library(monoGui SHARED
  monoGui.h
  monoGui.cpp
  monoGui.ui
  monoGui.qrc
  tuner.h
  tuner.cpp
  tuner.ui
  wtf.ui
  energysetter.ui
)

target_include_directories(monoGui
  PRIVATE ../monoBase
)

target_link_libraries(monoGui
  mono
  qcamotorgui
)

install(TARGETS monoGui
    LIBRARY DESTINATION lib/imbl
)

install(FILES monoGui.h
    DESTINATION include/imbl
)

