# Set up AUTOMOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON) # include the  AUTOUIC generated header dir

# 没有.cpp文件对应的.h头文件也要被加入以便触发UIC
file(GLOB_RECURSE COMMON_SRC "src/common/*")
file(GLOB_RECURSE DATASOURCE_SRC "src/datasource/*")
file(GLOB_RECURSE DECODER_SRC "src/decoder/*")
file(GLOB_RECURSE FFMPEG_SRC "src/ffmpeg/*")
file(GLOB_RECURSE FILESOURCE_SRC "src/filesource/*")
file(GLOB_RECURSE HANDLER_SRC "src/handler/*")
file(GLOB_RECURSE PARSER_SRC "src/parser/*")
file(GLOB_RECURSE PLAYLISTITEM_SRC "src/playlistitem/*")
file(GLOB_RECURSE STATISTICS_SRC "src/statistics/*")
file(GLOB_RECURSE VIDEO_SRC "src/video/*")
file(GLOB_RECURSE UI_SRC "src/ui/*" "src/views/*" "src/widgets/*")
file(GLOB_RECURSE UI_FILES "ui/*")
# message("UI_FILES: ${UI_FILES}")
# message("UI_SRC: ${UI_SRC}")

# Wrap UI files
# qt5_wrap_ui(UI_HEADERS ${UI_FILES})

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

    ${CMAKE_CURRENT_BINARY_DIR}/
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

if(ENABLE_SPDLOG)
    target_link_libraries(YUViewLib PUBLIC ${SPDLOG_LIB} ws2_32)
endif()

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

# install(TARGETS YUViewLib
#     LIBRARY DESTINATION ${RUNTIME_OUTPUT_DIR}
#     ARCHIVE DESTINATION ${RUNTIME_OUTPUT_DIR}
#     FRAMEWORK DESTINATION ${RUNTIME_OUTPUT_DIR}
#     RUNTIME DESTINATION ${RUNTIME_OUTPUT_DIR}
# )