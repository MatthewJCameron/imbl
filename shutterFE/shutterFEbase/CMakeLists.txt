
add_library(shutterFE SHARED
  shutterFE.h
  shutterFE.cpp
)

target_link_libraries(shutterFE
    component
)


install(TARGETS shutterFE
    LIBRARY DESTINATION lib/imbl
)

install(FILES shutterFE.h
    DESTINATION include/imbl
)

