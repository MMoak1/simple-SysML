cmake_minimum_required(VERSION 3.16)
project(Helloworld VERSION 0.1 LANGUAGES CXX)

set(CMAKE_C_COMPILER "C:/Qt/Tools/mingw1310_64/bin/gcc.exe")
set(CMAKE_CXX_COMPILER "C:/Qt/Tools/mingw1310_64/bin/g++.exe")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "C:/Qt/6.9.1/mingw_64")

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Svg Network)

qt_standard_project_setup()

qt_add_executable(Helloworld
    src/main.cpp
    src/mainwindow.cpp
    headers/mainwindow.h
    headers/views/dropgraphicsview.h
    src/views/dropgraphicsview.cpp
    headers/views/blockmenuview.h
    src/views/blockmenuview.cpp
    headers/models/blockmodel.h
    src/models/blockmodel.cpp
    headers/views/blockview.h
    src/views/blockview.cpp
    headers/views/startmenu.h
    src/views/startmenu.cpp
    headers/controllers/dropcontroller.h
    src/controllers/dropcontroller.cpp
    headers/controllers/menucontroller.h
    src/controllers/menucontroller.cpp
)

target_include_directories(Helloworld PRIVATE headers)

set_target_properties(Helloworld PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(Helloworld PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
    Qt6::Network
)

include(GNUInstallDirs)
install(TARGETS Helloworld
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)