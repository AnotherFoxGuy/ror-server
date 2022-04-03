cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0087 NEW)

project(servergui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(FORMS servergui.ui)

set(SOURCES servergui.cpp)

set(HEADERS servergui.h)

set(TRANSLATIONS languages/servergui.ts)

set_source_files_properties(${TRANSLATIONS} PROPERTIES OUTPUT_LOCATION "${RUNTIME_OUTPUT_DIRECTORY}translations")

find_package(Qt5LinguistTools)
if (Qt5LinguistTools_FOUND)
    if (UPDATE_TRANSLATIONS)
        qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR}/src ${TRANSLATIONS})
    else ()
        qt5_add_translation(QM_FILES ${TRANSLATIONS})
    endif ()
    install(FILES ${QM_FILES} DESTINATION ./translations)
endif ()

add_executable(
        ${PROJECT_NAME}
        ${FORMS}
        ${HEADERS}
        ${SOURCES}
        ${QM_FILES}
)

target_link_libraries(${PROJECT_NAME} Qt5::Widgets)


if (WIN32)
    get_target_property(Qt5_Core_Location Qt5::Core LOCATION)
    get_filename_component(QT_BIN_DIR ${Qt5_Core_Location} DIRECTORY)

    add_custom_target(
            copy_dll
            COMMAND ${QT_BIN_DIR}/windeployqt.exe $<TARGET_FILE:${PROJECT_NAME}> --release --no-compiler-runtime --no-translations --no-opengl-sw
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)

    install(CODE "execute_process(COMMAND ${QT_BIN_DIR}/windeployqt.exe $<TARGET_FILE:${PROJECT_NAME}> --release --no-compiler-runtime --no-translations --no-opengl-sw --dir \${CMAKE_INSTALL_PREFIX})")

endif ()
