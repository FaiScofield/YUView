if(ENABLE_CONSOLE)
    add_executable(YUView MACOSX_BUNDLE
        src/yuviewapp.cpp
    )
else()
    add_executable(YUView WIN32 MACOSX_BUNDLE
        src/yuviewapp.cpp
    )
endif()

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

if(ENABLE_CONSOLE)
    target_link_options(YUView PRIVATE "/SUBSYSTEM:CONSOLE")
    message(STATUS "Add console subsystem to target ${PROJECT_NAME} !")
endif()

if(WIN32)
    target_compile_definitions(YUView PRIVATE
        NOMINMAX
    )
    target_sources(YUView PRIVATE images/WindowsAppIcon.rc)
endif()

install(TARGETS YUView
    BUNDLE DESTINATION ${RUNTIME_OUTPUT_DIR}
    RUNTIME DESTINATION ${RUNTIME_OUTPUT_DIR}
)
install(FILES ${CMAKE_SOURCE_DIR}/LICENSE.GPL3 DESTINATION ${RUNTIME_OUTPUT_DIR})
