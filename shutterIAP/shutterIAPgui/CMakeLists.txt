
add_library(shutterIAPgui SHARED
  shutterIAPgui.h
  shutterIAPgui.cpp
  shutterIAPgui.ui
)

target_include_directories(shutterIAPgui
  PRIVATE ../shutterIAPbase
)

target_link_libraries(shutterIAPgui
  shutterIAP
)

install(TARGETS shutterIAPgui
    LIBRARY DESTINATION lib/imbl
)

install(FILES shutterIAPgui.h
    DESTINATION include/imbl
)

