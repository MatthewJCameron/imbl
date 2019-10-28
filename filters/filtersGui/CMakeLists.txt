
add_library(filtersGui SHARED
  filtersGui.h
  filtersGui.cpp
  filtersGui.ui
  filters.qrc
  paddle.ui
)

target_include_directories(filtersGui
  PRIVATE ../filtersBase
  PRIVATE ../../shutterFE/shutterFEbase
)

target_link_libraries(filtersGui
  filters
  shutterFE
  qmotorstack
  ${QWT_LIBRARIES}
)

install(TARGETS filtersGui
    LIBRARY DESTINATION lib/imbl
)

install(FILES filtersGui.h
    DESTINATION include/imbl
)

