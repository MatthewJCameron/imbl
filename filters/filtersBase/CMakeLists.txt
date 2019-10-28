
add_library(filters SHARED
  filters.h
  filters.cpp
)

target_link_libraries(filters
    component
    qcamotor
)

install(TARGETS filters
    LIBRARY DESTINATION lib/imbl
)

install(FILES filters.h
    DESTINATION include/imbl
)

