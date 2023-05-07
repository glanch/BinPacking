cmake_minimum_required(VERSION 3.0.0)
project(BPP VERSION 0.0.1)

find_package(SCIP REQUIRED)
include_directories(${SCIP_INCLUDE_DIRS})

include(CTest)
enable_testing()

#add every .cpp - file
add_executable(BPP
    main.cpp
    CompactModel.cpp
    Instance.cpp
    Master.cpp
    Pricer.cpp
    SubProblem.cpp
)

target_link_libraries(BPP ${SCIP_LIBRARIES} stdc++fs)

if( TARGET examples )
    add_dependencies( examples dicbap )
endif()

enable_testing()


