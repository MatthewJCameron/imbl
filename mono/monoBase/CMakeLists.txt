
add_library(mono SHARED
  mono.h
  mono.cpp
)

target_include_directories(mono
    PRIVATE ../../shutterFE/shutterFEbase
)

target_link_libraries(mono
    component
    shutterFE
    qcamotor
)

install(TARGETS mono
    LIBRARY DESTINATION lib/imbl
)

install(FILES mono.h
    DESTINATION include/imbl
)

