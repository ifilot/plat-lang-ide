cmake_minimum_required(VERSION 3.21)

project(plat_lang_ide VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg)

configure_file(
    src/app_version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/app_version.h
    @ONLY
)

configure_file(
    resources/windows/plat_lang_ide.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/plat_lang_ide.rc
    @ONLY
)

configure_file(
    resources/linux/plat-lang-ide.desktop.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/plat-lang-ide.desktop
    @ONLY
)

qt_standard_project_setup()

qt_add_executable(plat_lang_ide
    src/app/about_dialog.cpp
    src/app/about_dialog.h
    src/app/command_palette_dialog.cpp
    src/app/command_palette_dialog.h
    src/app/main_window.cpp
    src/app/main_window.h
    src/app/settings_dialog.cpp
    src/app/settings_dialog.h
    src/editor/code_editor.cpp
    src/editor/code_editor.h
    src/editor/code_highlighter.cpp
    src/editor/code_highlighter.h
    src/editor/editor_tabs.cpp
    src/editor/editor_tabs.h
    src/editor/find_replace_bar.cpp
    src/editor/find_replace_bar.h
    src/editor/text_file_io.cpp
    src/editor/text_file_io.h
    src/main.cpp
    src/panels/bottom_panel.cpp
    src/panels/bottom_panel.h
    src/panels/code_assistant_panel.cpp
    src/panels/code_assistant_panel.h
    src/panels/file_explorer_panel.cpp
    src/panels/file_explorer_panel.h
    src/panels/output_panel.cpp
    src/panels/output_panel.h
    src/panels/problems_panel.cpp
    src/panels/problems_panel.h
    src/panels/terminal_panel.cpp
    src/panels/terminal_panel.h
    src/services/app_fonts.cpp
    src/services/app_fonts.h
    src/services/compiler_toolchain.cpp
    src/services/compiler_toolchain.h
    src/services/file_icon_provider.cpp
    src/services/file_icon_provider.h
    src/ui/theme_manager.cpp
    src/ui/theme_manager.h
    src/ui/title_bar.cpp
    src/ui/title_bar.h
)

if(WIN32)
    target_sources(plat_lang_ide PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/generated/plat_lang_ide.rc
    )
endif()

qt_add_resources(plat_lang_ide app_resources
    PREFIX "/"
    FILES
        assets/fonts/space-mono/SpaceMono-Regular.ttf
        assets/fonts/space-mono/SpaceMono-Bold.ttf
        assets/icons/fontawesome/solid/file.svg
        assets/icons/fontawesome/solid/folder.svg
        assets/icons/fontawesome/solid/xmark.svg
        assets/themes/dark.qss
        assets/themes/light.qss
        logos/plat-lang-logo.svg
)

target_link_libraries(plat_lang_ide
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Svg
)

target_include_directories(plat_lang_ide
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/generated
        ${CMAKE_CURRENT_SOURCE_DIR}/src/app
        ${CMAKE_CURRENT_SOURCE_DIR}/src/editor
        ${CMAKE_CURRENT_SOURCE_DIR}/src/panels
        ${CMAKE_CURRENT_SOURCE_DIR}/src/services
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
)

set_target_properties(plat_lang_ide PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

if(WIN32 AND MINGW)
    target_link_options(plat_lang_ide PRIVATE -mwindows)
endif()

install(TARGETS plat_lang_ide
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(WIN32)
    qt_generate_deploy_app_script(
        TARGET plat_lang_ide
        OUTPUT_SCRIPT qt_deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${qt_deploy_script})
endif()

install(FILES
    LICENSE
    THIRD_PARTY_NOTICES.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
)

if(UNIX AND NOT APPLE)
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/generated/plat-lang-ide.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    )

    foreach(icon_size 16 32 48 64 128 256)
        install(FILES
            assets/icons/app/hicolor/${icon_size}x${icon_size}/apps/plat-lang-ide.png
            DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/${icon_size}x${icon_size}/apps
        )
    endforeach()

    install(FILES
        logos/plat-lang-logo.svg
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
        RENAME plat-lang-ide.svg
    )
endif()

set(CPACK_PACKAGE_NAME "plat-lang IDE")
set(CPACK_PACKAGE_VENDOR "plat-lang")
set(CPACK_PACKAGE_CONTACT "plat-lang")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A Qt-based IDE for the platlang language.")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "plat-lang IDE")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_FILE_NAME "plat-lang-ide-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")

set(CPACK_NSIS_DISPLAY_NAME "plat-lang IDE")
set(CPACK_NSIS_PACKAGE_NAME "plat-lang IDE")
set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/logos/plat-lang-logo.ico")
set(CPACK_NSIS_INSTALLED_ICON_NAME "${CMAKE_INSTALL_BINDIR}\\\\plat_lang_ide.exe")
set(CPACK_NSIS_EXECUTABLES_DIRECTORY "${CMAKE_INSTALL_BINDIR}")
set(CPACK_PACKAGE_EXECUTABLES "plat_lang_ide" "plat-lang IDE")
set(CPACK_CREATE_DESKTOP_LINKS "plat_lang_ide")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
set(CPACK_NSIS_MODIFY_PATH OFF)

include(CPack)
