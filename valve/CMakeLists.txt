
add_library(valve SHARED
  valve.h
  valve.cpp
)

target_link_libraries(valve
    component
)

target_include_directories(valve
  PRIVATE ../../component/componentBase
)

install(TARGETS valve
    LIBRARY DESTINATION lib/imbl
)

install(FILES valve.h
    DESTINATION include/imbl
)

