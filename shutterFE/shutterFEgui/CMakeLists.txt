
add_library(shutterFEgui SHARED
  shutterFEgui.h
  shutterFEgui.cpp
  shutterFEgui.ui
)

target_include_directories(shutterFEgui
  PRIVATE ../shutterFEbase
)

target_link_libraries(shutterFEgui
  shutterFE
)

install(TARGETS shutterFEgui
    LIBRARY DESTINATION lib/imbl
)

install(FILES shutterFEgui.h
    DESTINATION include/imbl
)

