// Shim: include the real UBDocumentProxy.h with protected→public for test access
#ifndef UBDOCUMENTPROXY_TEST_SHIM_H
#define UBDOCUMENTPROXY_TEST_SHIM_H

// Include all Qt headers first (before the macro trick)
#include <QWidget>
#include <QApplication>
#include <QPainter>

// Now make protected members accessible
#define protected public
#include "../../src/document/UBDocumentProxy.h"
#undef protected

#endif // UBDOCUMENTPROXY_TEST_SHIM_H
