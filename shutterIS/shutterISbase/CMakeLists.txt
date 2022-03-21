
add_library(shutterIS SHARED
  shutterIS.h
  shutterIS.cpp
)

target_link_libraries(shutterIS
    component
)

install(TARGETS shutterIS
    LIBRARY DESTINATION lib/imbl
)

install(FILES shutterIS.h
    DESTINATION include/imbl
)

