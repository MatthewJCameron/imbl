
add_library(mrtShutterGui SHARED
  mrtShutterGui.h
  mrtShutterGui.cpp
  mrtShutterGui.ui
)

target_include_directories(mrtShutterGui
  PRIVATE ../mrtShutterBase
)

target_link_libraries(mrtShutterGui
  mrtShutter
)

install(TARGETS mrtShutterGui
    LIBRARY DESTINATION lib/imbl
)

install(FILES mrtShutterGui.h
    DESTINATION include/imbl
)

