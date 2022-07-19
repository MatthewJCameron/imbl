
add_library(shutterFE SHARED
  shutterFE.h
  shutterFE.cpp
)

target_include_directories(shutterFE
    PRIVATE ../../expander/expanderBase ../../shutter1A/shutter1Abase 
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

