  TARGET = OpenSankoreTests
TEMPLATE = app

CONFIG += testcase console c++17
CONFIG -= app_bundle

QT += core gui widgets testlib xml network

# Paths — tests/ shims take priority over ../src for stubbed headers
INCLUDEPATH += .
INCLUDEPATH += ../src

# We only compile the utility sources under test (no app dependencies)
# This avoids pulling in UBApplication, UBPersistenceManager, etc.

# Headers under test
HEADERS += ../src/frameworks/UBStringUtils.h \
           ../src/frameworks/UBGeometryUtils.h \
           ../src/frameworks/UBVersion.h \
           ../src/frameworks/UBBase32.h \
           ../src/core/UB.h \
           ../src/web/UBOEmbedUtils.h \
           ../src/adaptors/UBSvgTransformUtils.h \
           ../src/adaptors/UBMetadataLoader.h

# Sources under test (only self-contained utilities)
SOURCES += ../src/frameworks/UBStringUtils.cpp \
           ../src/frameworks/UBGeometryUtils.cpp \
           ../src/frameworks/UBVersion.cpp \
           ../src/frameworks/UBBase32.cpp \
           ../src/adaptors/UBIniFileParser.cpp \
           ../src/adaptors/UBSvgTransformUtils.cpp \
           ../src/core/UBForeignObjectsUtils.cpp

# For UBFileSystemUtils we need a minimal version without OpenSSL/QuaZip deps
# We provide a test stub instead (not in HEADERS to avoid moc issues on Linux)
# Real source instead of stub (no QuaZip dependency)
SOURCES += ../src/frameworks/UBFileSystemUtilsCore.cpp
SOURCES += ../src/domain/UBSceneContext.cpp

# UBCryptoUtils stub (bypasses UBApplication singleton)
# Not in HEADERS to avoid moc parsing system headers on Linux
SOURCES += stubs/UBCryptoUtils_stub.cpp

# UBOEmbedParser — tests compile the real UBOEmbedUtils.cpp (parsing logic)
SOURCES += ../src/web/UBOEmbedUtils.cpp

# UBMetadataDcSubsetAdaptor — tests compile the real UBMetadataLoader.cpp (load logic)
SOURCES += ../src/adaptors/UBMetadataLoader.cpp

# UBDocumentProxy — compile real source with header shims for deps
# To regenerate moc: moc -I. -I../src ../src/document/UBDocumentProxy.h -o premoc/moc_UBDocumentProxy.cpp
SOURCES += stubs/UBSettings_stub.cpp \
           stubs/UBDocumentManager_stub.cpp \
           ../src/document/UBDocumentProxy.cpp \
           premoc/moc_UBSettings_stub.cpp \
           premoc/moc_UBDocumentManager_stub.cpp \
           premoc/moc_UBDocumentProxy_stub.cpp

# UBPlatformUtils stub
SOURCES += stubs/UBPlatformUtils_stub.cpp

# Test headers — simple ones that moc can handle directly
# NOTE: moved to premoc/ due to moc bug on Linux CI (cannot parse system C++ headers)

# Test headers — not in HEADERS (premoc used instead to avoid moc issues on Linux)

# UBBoardSubControllers — stub with static methods only (no app deps)

# Test sources
SOURCES += main.cpp \
           tst_UBStringUtils.cpp \
           tst_UBFileSystemUtils.cpp \
           tst_UBGeometryUtils.cpp \
           tst_UBCryptoUtils.cpp \
           tst_UBDocumentProxy.cpp \
           tst_UBSettings.cpp \
           tst_UBVersion.cpp \
           tst_UBBase32.cpp \
           tst_UBIniFileParser.cpp \
           tst_UBMetadataDcSubsetAdaptor.cpp \
           tst_UBOEmbedParser.cpp \
           tst_UBSvgTransform.cpp \
           tst_UBForeignObjectsHelper.cpp \
           tst_UBGraphicsScene.cpp \
           premoc/moc_tst_UBStringUtils.cpp \
           premoc/moc_tst_UBFileSystemUtils.cpp \
           premoc/moc_tst_UBGeometryUtils.cpp \
           premoc/moc_tst_UBCryptoUtils.cpp \
           premoc/moc_tst_UBDocumentProxy.cpp \
           premoc/moc_tst_UBSettings.cpp \
           premoc/moc_tst_UBVersion.cpp \
           premoc/moc_tst_UBBase32.cpp \
           premoc/moc_tst_UBIniFileParser.cpp \
           premoc/moc_tst_UBMetadataDcSubsetAdaptor.cpp \
           premoc/moc_tst_UBOEmbedParser.cpp \
           premoc/moc_tst_UBSvgTransform.cpp \
           premoc/moc_tst_UBForeignObjectsHelper.cpp \
           premoc/moc_tst_UBGraphicsScene.cpp \
           tst_UBGraphicsScene.cpp \


# Build output
DESTDIR = build
OBJECTS_DIR = build/objects
MOC_DIR = build/moc

# OpenSSL for UBCryptoUtils
win32-msvc* {
    # Static linking to avoid DLL loading issues on CI
    OPENSSL_STATIC_PATH = $$(VCPKG_ROOT)/installed/x64-windows-static
    !isEmpty(OPENSSL_STATIC_PATH):exists($$OPENSSL_STATIC_PATH/lib/libcrypto.lib) {
        INCLUDEPATH += $$OPENSSL_STATIC_PATH/include
        LIBS += -L$$OPENSSL_STATIC_PATH/lib -llibcrypto
        LIBS += -lws2_32 -lcrypt32 -ladvapi32 -luser32
    } else {
        # Fallback to dynamic
        VCPKG_ROOT_PATH = $$(VCPKG_ROOT)
        INCLUDEPATH += $$VCPKG_ROOT_PATH/installed/x64-windows/include
        LIBS += -L$$VCPKG_ROOT_PATH/installed/x64-windows/lib -llibcrypto
    }
} else {
    LIBS += -lcrypto
}

# Code coverage (GCC/Clang only — not available on MSVC)
!win32-msvc* {
    QMAKE_CXXFLAGS += --coverage
    QMAKE_LFLAGS += --coverage
}
