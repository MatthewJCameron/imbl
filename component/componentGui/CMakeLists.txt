
add_library(componentGui SHARED
  componentGui.h
  componentGui.cpp
)

target_include_directories(componentGui
    PRIVATE ../componentBase
)

target_link_libraries(componentGui
    component
)

install(TARGETS componentGui
    LIBRARY DESTINATION lib/imbl
)

install(FILES componentGui.h
    DESTINATION include/imbl
)

