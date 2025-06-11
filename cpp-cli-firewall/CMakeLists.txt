project(cpp-cli-firewall)

set(CMAKE_CXX_STANDARD 11)

include_directories(src)

file(GLOB SOURCES
    src/*.cpp
)

add_executable(cpp-cli-firewall ${SOURCES})

install(TARGETS cpp-cli-firewall DESTINATION bin)