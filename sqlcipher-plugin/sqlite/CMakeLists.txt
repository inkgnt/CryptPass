# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## QSQLiteDriverPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QSQLiteDriverPlugin
    OUTPUT_NAME qsqlite
    PLUGIN_TYPE sqldrivers
    SOURCES
        qsql_sqlite.cpp qsql_sqlite_p.h
        qsql_sqlite_vfs.cpp qsql_sqlite_vfs_p.h
        smain.cpp
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
        QT_NO_CONTEXTLESS_CONNECT
    LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::SqlPrivate
)

## Scopes:
#####################################################################

qt_internal_extend_target(QSQLiteDriverPlugin CONDITION QT_FEATURE_system_sqlite
    LIBRARIES
        SQLite::SQLite3
)

if (NOT QT_FEATURE_system_sqlite)
    # On newer compilers compiling sqlite.c produces warnings
    qt_disable_warnings(QSQLiteDriverPlugin)
endif()

if(QT_FEATURE_system_sqlite)
    qt_internal_force_macos_intel_arch(QSQLiteDriverPlugin)
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(OPENSSL_ARCH "x64")
else()
    set(OPENSSL_ARCH "x86")
endif()

qt_internal_extend_target(QSQLiteDriverPlugin CONDITION NOT QT_FEATURE_system_sqlite
    SOURCES
        ${CMAKE_SOURCE_DIR}/sqlcipher_am/sqlite3.c
    DEFINES
		_CRT_SECURE_NO_WARNINGS
		HAVE_STDINT_H
		SQLITE_HAS_CODEC
		SQLITE_EXTRA_INIT=sqlcipher_extra_init
		SQLITE_EXTRA_SHUTDOWN=sqlcipher_extra_shutdown
		SQLITE_TEMP_STORE=3
		SQLITE_ENABLE_FTS3=1
		SQLITE_ENABLE_FTS4=1
		SQLITE_ENABLE_FTS5=1
		SQLITE_ENABLE_JSON1=1
		SQLITE_ENABLE_RTREE=1
		SQLITE_ENABLE_GEOPOLY=1
		SQLITE_ENABLE_MATH_FUNCTIONS=1
		SQLITE_ENABLE_SESSION=1
		SQLITE_ENABLE_PREUPDATE_HOOK=1
		SQLITE_ENABLE_SERIALIZE=1
		SQLITE_ENABLE_COLUMN_METADATA=1
		SQLITE_ENABLE_FTS3_PARENTHESIS=1
		SQLITE_OMIT_COMPLETE=1
    INCLUDE_DIRECTORIES
        ${CMAKE_SOURCE_DIR}/sqlcipher_am
		${CMAKE_SOURCE_DIR}/../external/openssl/${OPENSSL_ARCH}/include
    ATTRIBUTION_FILE_DIR_PATHS
        ${CMAKE_SOURCE_DIR}/sqlcipher_am

    LIBRARIES
        ${CMAKE_SOURCE_DIR}/../external/openssl/${OPENSSL_ARCH}/lib/libcrypto.lib
        ${CMAKE_SOURCE_DIR}/../external/openssl/${OPENSSL_ARCH}/lib/libssl.lib
        ws2_32.lib
        advapi32.lib
        crypt32.lib
        user32.lib
)

qt_internal_extend_target(QSQLiteDriverPlugin CONDITION CMAKE_BUILD_TYPE STREQUAL Release AND NOT QT_FEATURE_system_sqlite
    DEFINES
        NDEBUG
)

qt_internal_extend_target(QSQLiteDriverPlugin CONDITION NOT QT_FEATURE_largefile AND NOT QT_FEATURE_system_sqlite
    DEFINES
        SQLITE_DISABLE_LFS
)

qt_internal_extend_target(QSQLiteDriverPlugin CONDITION QT_FEATURE_posix_fallocate AND NOT QT_FEATURE_system_sqlite
    DEFINES
        HAVE_POSIX_FALLOCATE=1
)

qt_internal_extend_target(QSQLiteDriverPlugin CONDITION QNX AND NOT QT_FEATURE_system_sqlite
    DEFINES
        _QNX_SOURCE
)

qt_internal_extend_target(QSQLiteDriverPlugin CONDITION UNIX AND NOT QT_FEATURE_system_sqlite
    DEFINES
        HAVE_USLEEP=1
)

qt_internal_extend_target(QSQLiteDriverPlugin CONDITION INTEGRITY
    COMPILE_OPTIONS -include qplatformdefs.h
)

qt_internal_extend_target(QSQLiteDriverPlugin CONDITION QT_FEATURE_dlopen AND NOT QT_FEATURE_system_sqlite
    LIBRARIES
        ${CMAKE_DL_LIBS}
)

qt_internal_extend_target(QSQLiteDriverPlugin CONDITION NOT QT_FEATURE_dlopen AND NOT QT_FEATURE_system_sqlite
    DEFINES
        SQLITE_OMIT_LOAD_EXTENSION
)

qt_internal_extend_target(QSQLiteDriverPlugin CONDITION NOT QT_FEATURE_thread AND NOT QT_FEATURE_system_sqlite
    DEFINES
        SQLITE_THREADSAFE=0
)
