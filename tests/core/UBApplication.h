// Shim: stub out core/UBApplication.h for test builds
// UBDocumentProxy.cpp includes this but doesn't use any symbols from it.
#ifndef UBAPPLICATION_STUB_SHIM_H
#define UBAPPLICATION_STUB_SHIM_H
// Empty — no symbols needed from UBApplication in UBDocumentProxy.cpp
#endif
