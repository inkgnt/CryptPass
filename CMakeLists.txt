cmake_minimum_required(VERSION 3.16)
project(CryptPass VERSION 0.1 LANGUAGES CXX)

# ==========================================================
# Compiler setup
# ==========================================================
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_SOURCE_DIR}/ui
)

# ==========================================================
# Find Qt
# ==========================================================
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql)

# ==========================================================
# Find OpenSSL
# ==========================================================
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(OPENSSL_ROOT_DIR "${CMAKE_SOURCE_DIR}/external/openssl/x64")
else()
    set(OPENSSL_ROOT_DIR "${CMAKE_SOURCE_DIR}/external/openssl/x86")
endif()

set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)

# ==========================================================
# Sources
# ==========================================================
set(SOURCES
    src/main.cpp
    src/crypto.cpp
    src/keymanager.cpp
    src/mainapp.cpp
    src/registerwidget.cpp
    src/loginwidget.cpp
    src/mainwindowwidget.cpp
    src/databasemanager.cpp
    src/dialog.cpp
    src/passwordform.cpp
    src/passwordcardwidget.cpp
)

set(HEADERS
    src/crypto.h
    src/keymanager.h
    src/mainapp.h
    src/registerwidget.h
    src/loginwidget.h
    src/mainwindowwidget.h
    src/databasemanager.h
    src/dialog.h
    src/passwordform.h
    src/passwordcardwidget.h
)

set(UI
    ui/mainapp.ui
    ui/registerwidget.ui
    ui/loginwidget.ui
    ui/mainwindowwidget.ui
    ui/dialog.ui
    ui/passwordform.ui
    ui/passwordcardwidget.ui
)

set(RESOURCES
    resources/resource.qrc
)

# ==========================================================
# Target
# ==========================================================
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(CryptPass
        MANUAL_FINALIZATION
        ${SOURCES}
        ${HEADERS}
        ${UI}
        ${RESOURCES}
    )
    qt_finalize_executable(CryptPass)
endif()

target_link_libraries(CryptPass PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
)

# ==========================================================
# OpenSSL linking
# ==========================================================
if(OPENSSL_FOUND)
    target_link_libraries(CryptPass PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif()

# ==========================================================
# Target properties & install
# ==========================================================
set_target_properties(CryptPass PROPERTIES
    WIN32_EXECUTABLE TRUE                     # Убирает консольное окно у GUI-приложений (Windows)
)

include(GNUInstallDirs)

install(TARGETS CryptPass
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}   # .exe или ELF
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}   # .dll / .so
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}   # .lib / .a
)

# ==========================================================
# Deployment (Windows only)
# ==========================================================
if (WIN32)
    get_target_property(QT_QMAKE_EXECUTABLE Qt${QT_VERSION_MAJOR}::windeployqt IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
    set(WINDEPLOYQT_EXECUTABLE "${QT_BIN_DIR}/windeployqt.exe")

    add_custom_command(TARGET CryptPass POST_BUILD
        COMMAND "${WINDEPLOYQT_EXECUTABLE}" --release "$<TARGET_FILE:CryptPass>"
    )
endif()
