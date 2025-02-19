cmake_minimum_required(VERSION 3.5)
project(autoplay)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets)

# Main service executable
add_executable(autoplay
    source/main.cpp
)
target_link_libraries(autoplay udev)

# GUI executable
add_executable(autoplay-gui
    source/gui.cpp
)
target_link_libraries(autoplay-gui Qt5::Widgets)

# Install rules
install(TARGETS autoplay autoplay-gui
    RUNTIME DESTINATION /usr/bin
)
install(FILES systemd/autoplay.service
    DESTINATION /etc/systemd/user
)
