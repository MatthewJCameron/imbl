
add_library(shutterISgui SHARED
  shutterISgui.h
  shutterISgui.cpp
  shutterISgui.ui
)

target_include_directories(shutterISgui
  PRIVATE ../shutterISbase
)

target_link_libraries(shutterISgui
  shutterIS
)

install(TARGETS shutterISgui
    LIBRARY DESTINATION lib/imbl
)

install(FILES shutterISgui.h
    DESTINATION include/imbl
)

