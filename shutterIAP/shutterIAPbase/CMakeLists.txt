
add_library(shutterIAP SHARED
  shutterIAP.h
  shutterIAP.cpp
)

target_link_libraries(shutterIAP
    component
)

install(TARGETS shutterIAP
    LIBRARY DESTINATION lib/imbl
)

install(FILES shutterIAP.h
    DESTINATION include/imbl
)

