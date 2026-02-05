qt_add_executable(YUViewUnitTest
    *.cpp, *.h,
)
target_include_directories(YUViewUnitTest PRIVATE
    /YUViewLib/src
    /YUViewUnitTest/common
    /submodules/googletest/googlemock/include
    /submodules/googletest/googletest/include
)

target_link_libraries(YUViewUnitTest PRIVATE
    # Remove: L/YUViewLib
    # Remove: L/submodules/googletest-qmake/gtest
    # Remove: L/submodules/googletest-qmake/gtest_main
    Qt::Core
    Qt::Gui
    Qt::Xml
    YUViewLib
    gtest
    gtest_main
)

install(TARGETS YUViewUnitTest
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
