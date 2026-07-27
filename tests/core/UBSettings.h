// Shim: redirect core/UBSettings.h to the test stub
// This allows real source files (UBMetadataLoader.cpp) to compile
// against the test stub without modifying their #include directives.
#ifndef UBSETTINGS_STUB_SHIM_H
#define UBSETTINGS_STUB_SHIM_H
#include "stubs/UBSettings_stub.h"
#endif
