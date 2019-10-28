
add_library(shutter1A SHARED
  shutter1A.h
  shutter1A.cpp
)

target_link_libraries(shutter1A
    component
)

install(TARGETS shutter1A
    LIBRARY DESTINATION lib/imbl
)

install(FILES shutter1A.h
    DESTINATION include/imbl
)

