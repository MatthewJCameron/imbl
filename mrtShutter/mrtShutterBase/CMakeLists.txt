
add_library(mrtShutter SHARED
  mrtShutter.h
  mrtShutter.cpp
)

target_link_libraries(mrtShutter
    component
    shutter1A
)

target_include_directories(mrtShutter
  PRIVATE ../../shutter1A/shutter1Abase
)

install(TARGETS mrtShutter
    LIBRARY DESTINATION lib/imbl
)

install(FILES mrtShutter.h
    DESTINATION include/imbl
)

