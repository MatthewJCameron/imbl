
add_library(component SHARED
  component.h
  component.cpp
)

install(TARGETS component
    LIBRARY DESTINATION lib/imbl
)

install(FILES component.h
    DESTINATION include/imbl
)

