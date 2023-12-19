project(cloud)

set( ${PROJECT_NAME}_SOURCES
    brwsfldr.cpp
    cloud.cpp
    dos_cgui.cpp
    dos_neti.cpp
    dos_pwds.cpp
    dos_scrn.cpp
    filelist.cpp
    guistate.cpp
    inputip.cpp
    list.cpp
    logstrm.cpp
    net_stts.cpp
    prot.cpp
)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES})