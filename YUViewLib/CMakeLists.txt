file(GLOB_RECURSE COMMON_SRC "src/common/*.cpp")
file(GLOB_RECURSE DATASOURCE_SRC "src/datasource/*.cpp")
file(GLOB_RECURSE DECODER_SRC "src/decoder/*.cpp")
file(GLOB_RECURSE FFMPEG_SRC "src/ffmpeg/*.cpp")
file(GLOB_RECURSE FILESOURCE_SRC "src/filesource/*.cpp")
file(GLOB_RECURSE HANDLER_SRC "src/handler/*.cpp")
file(GLOB_RECURSE PARSER_SRC "src/parser/*.cpp")
file(GLOB_RECURSE PLAYLISTITEM_SRC "src/playlistitem/*.cpp")
file(GLOB_RECURSE STATISTICS_SRC "src/statistics/*.cpp")
file(GLOB_RECURSE VIDEO_SRC "src/video/*.cpp")
file(GLOB_RECURSE UI_SRC "src/ui/*.cpp" "src/views/*.cpp" "src/widgets/*.cpp")
file(GLOB_RECURSE UI_FILES "ui/*.ui")

add_library(YUViewLib
    ${COMMON_SRCS}
    ${DATASOURCE}
    ${DECODER_SRCS}
    ${COMMON_SRC}
    ${DATASOURCE_SRC}
    ${DECODER_SRC}
    ${FFMPEG_SRC}
    ${FILESOURCE_SRC}
    ${HANDLER_SRC}
    ${PARSER_SRC}
    ${PLAYLISTITEM_SRC}
    ${STATISTICS_SRC}
    ${VIDEO_SRC}
    ${UI_SRC}
    ${UI_FILES}
    images/images.qrc
    docs/docs.qrc
)

set_target_properties(YUViewLib PROPERTIES
    AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/ui"
)

target_compile_definitions(YUViewLib PRIVATE UNICODE _UNICODE)

target_include_directories(YUViewLib PUBLIC
    src
    src/common
    src/datasource
    src/decoder
    src/decoder/externalHeader
    src/decoder/externalHeader/dav1d
    src/decoder/externalHeader/libde265
    src/decoder/externalHeader/vvdec
    src/ffmpeg
    src/filesource
    src/handler
    src/parser
    src/playlistitem
    src/statistics
    src/video
    src/ui
    src/ui/views
    src/ui/widgets
    ui

    ${CMAKE_CURRENT_BINARY_DIR}/YUViewLib_autogen/include
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_BINARY_DIR}/YUViewLib_autogen/include/include_Debug>
    $<$<CONFIG:Release>:${CMAKE_CURRENT_BINARY_DIR}/YUViewLib_autogen/include/include_Release>
)

target_link_libraries(YUViewLib PUBLIC
    Qt5::Concurrent
    Qt5::Core
    Qt5::Gui
    Qt5::Network
    Qt5::OpenGL
    Qt5::Widgets
    Qt5::Xml
)

# Resources:
aux_source_directory(images images_resource_files)
aux_source_directory(docs docs_resource_files)

# qt5_add_resources(YUViewLib "images"
#     PREFIX
#         "/"
#     BASE
#         "images"
#     FILES
#         ${images_resource_files}
# )

# qt5_add_resources(YUViewLib "docs"
#     PREFIX
#         "/"
#     BASE
#         "docs"
#     FILES
#         ${docs_resource_files}
# )

if(WIN32)
    target_compile_definitions(YUViewLib PUBLIC
        NOMINMAX
    )
endif()

install(TARGETS YUViewLib
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)