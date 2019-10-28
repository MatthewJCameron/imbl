
add_library(hutch SHARED
  hutch.h
  hutch.cpp
)

target_link_libraries(hutch
    component
)

target_include_directories(hutch
  PRIVATE ../../component/componentBase
)

install(TARGETS hutch
    LIBRARY DESTINATION lib/imbl
)

install(FILES hutch.h
    DESTINATION include/imbl
)

