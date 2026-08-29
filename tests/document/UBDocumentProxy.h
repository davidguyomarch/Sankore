/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

// Shim: include the real UBDocumentProxy.h and provide a test helper
// to access protected methods
#ifndef UBDOCUMENTPROXY_TEST_SHIM_H
#define UBDOCUMENTPROXY_TEST_SHIM_H

#include "../../src/document/UBDocumentProxy.h"

/**
 * Test helper that exposes protected methods of UBDocumentProxy.
 * Used only in tests to access setPageCount/incPageCount/decPageCount.
 */
class UBDocumentProxyTestHelper : public UBDocumentProxy
{
public:
    UBDocumentProxyTestHelper() : UBDocumentProxy() {}
    UBDocumentProxyTestHelper(const QString& path) : UBDocumentProxy(path) {}

    using UBDocumentProxy::setPageCount;
    using UBDocumentProxy::incPageCount;
    using UBDocumentProxy::decPageCount;
};

#endif // UBDOCUMENTPROXY_TEST_SHIM_H
