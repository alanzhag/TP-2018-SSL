cmake_minimum_required(VERSION 3.12)
project(TP_2018_SSL C)
include(InstallRequiredSystemLibraries)
include(CTest)
include(CPack)

set(CTEST_PROJECT_NAME "TP_2018_SSL")

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${Tutorial_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${Tutorial_VERSION_MINOR}")

set(TP_2018_SSL_VERSION_MAJOR 1)
set(TP_2018_SSL_VERSION_MINOR 0)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address ")
#set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak ")
#set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined ")

add_executable(TP_2018_SSL main.c)