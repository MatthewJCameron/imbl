
add_library(expander SHARED
  expander.h
  expander.cpp
)

target_include_directories(expander
    PRIVATE ../../shutterFE/shutterFEbase
)

target_link_libraries(expander
    component
    shutterFE
    qcamotor
)

install(TARGETS expander
    LIBRARY DESTINATION lib/imbl
)

install(FILES expander.h
    DESTINATION include/imbl
)

