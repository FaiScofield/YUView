add_executable(YUView WIN32 MACOSX_BUNDLE
    src/yuviewapp.cpp
)

target_include_directories(YUView PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../YUViewLib/src
)

target_link_libraries(YUView PRIVATE
    Qt::Concurrent
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::OpenGL
    Qt::Widgets
    Qt::Xml
    YUViewLib
)

if(WIN32)
    target_compile_definitions(YUView PRIVATE
        NOMINMAX
    )
    # Add Windows resource file
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../images/WindowsAppIcon.rc")
        target_sources(YUView PRIVATE ../images/WindowsAppIcon.rc)
    endif()
endif()

install(TARGETS YUView
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)