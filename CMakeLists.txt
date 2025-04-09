cmake_minimum_required(VERSION 3.16)

project(CryptPass VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql)
set(OPENSSL_USE_STATIC_LIBS FALSE)
find_package(OpenSSL REQUIRED)  # openssl

set(PROJECT_SOURCES
        main.cpp

      )

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(CryptPass
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}

        crypto.h crypto.cpp
        keymanager.h keymanager.cpp
        mainapp.h mainapp.cpp mainapp.ui
        registerwidget.h registerwidget.cpp registerwidget.ui
        loginwidget.h loginwidget.cpp loginwidget.ui
        mainwindowwidget.h mainwindowwidget.cpp mainwindowwidget.ui
        databasemanager.h databasemanager.cpp
        dialog.h dialog.cpp dialog.ui
        passwordform.h passwordform.cpp passwordform.ui
        passwordcardwidget.h passwordcardwidget.cpp passwordcardwidget.ui


    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET CryptPass APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(CryptPass SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(CryptPass
            ${PROJECT_SOURCES}
        )
    endif()
endif()


#sqlcipher
include_directories(${CMAKE_SOURCE_DIR}/external/include)
link_directories(${CMAKE_SOURCE_DIR}/external/lib)

target_link_libraries(CryptPass PRIVATE
  Qt${QT_VERSION_MAJOR}::Widgets
  Qt${QT_VERSION_MAJOR}::Sql
  ${CMAKE_SOURCE_DIR}/external/lib/sqlite3.lib
)

if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})  #openssl
    target_link_libraries(CryptPass PRIVATE OpenSSL::SSL OpenSSL::Crypto)  # openssl
endif()

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.CryptPass)
endif()
set_target_properties(CryptPass PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)

install(TARGETS CryptPass
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if (WIN32)
    get_target_property(QT_QMAKE_EXECUTABLE Qt${QT_VERSION_MAJOR}::windeployqt IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
    set(WINDEPLOYQT_EXECUTABLE "${QT_BIN_DIR}/windeployqt.exe")

    add_custom_command(TARGET CryptPass POST_BUILD
        COMMAND "${WINDEPLOYQT_EXECUTABLE}" --release "$<TARGET_FILE:CryptPass>"
    )
endif()

#external dlls
if(WIN32)
    set(EXTERNAL_DLL_DIR "${CMAKE_SOURCE_DIR}/external/dll")

    add_custom_command(TARGET CryptPass POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${EXTERNAL_DLL_DIR}/libcrypto-3-x64.dll" "$<TARGET_FILE_DIR:CryptPass>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${EXTERNAL_DLL_DIR}/libssl-3-x64.dll" "$<TARGET_FILE_DIR:CryptPass>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${EXTERNAL_DLL_DIR}/sqlite3.dll" "$<TARGET_FILE_DIR:CryptPass>"
    )
endif()
