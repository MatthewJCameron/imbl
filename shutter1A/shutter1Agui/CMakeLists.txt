
add_library(shutter1Agui SHARED
  shutter1Agui.h
  shutter1Agui.cpp
  shutter1Agui.ui
)

target_include_directories(shutter1Agui
  PRIVATE ../shutter1Abase
)

target_link_libraries(shutter1Agui
  shutter1A
)

install(TARGETS shutter1Agui
    LIBRARY DESTINATION lib/imbl
)

install(FILES shutter1Agui.h
    DESTINATION include/imbl
)

