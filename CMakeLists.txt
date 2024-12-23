cmake_minimum_required(VERSION 3.25)
project(dummy_usb_demo)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

include_directories(${LIBUSB_INCLUDE_DIRS})
link_directories(${LIBUSB_LIBRARY_DIRS})


add_executable(dummy_usb_demo main.cpp)

target_link_libraries(dummy_usb_demo ${LIBUSB_LIBRARIES})