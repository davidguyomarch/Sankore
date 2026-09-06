/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

/**
 * @file main.cpp
 * @brief Point d'entrée des tests unitaires Open-Sankoré.
 *
 * Exécute tous les tests enregistrés via QTest.
 */

#include <QtTest>
#include <QApplication>

#include "tst_UBStringUtils.h"
#include "tst_UBFileSystemUtils.h"
#include "tst_UBGeometryUtils.h"
#include "tst_UBCryptoUtils.h"
#include "tst_UBDocumentProxy.h"
#include "tst_UBSettings.h"
#include "tst_UBVersion.h"
#include "tst_UBBase32.h"
#include "tst_UBIniFileParser.h"
#include "tst_UBMetadataDcSubsetAdaptor.h"
#include "tst_UBOEmbedParser.h"
#include "tst_UBSvgTransform.h"
#include "tst_UBForeignObjectsHelper.h"
#include "tst_UBTextTools.h"
#include "tst_UBStrokeProperty.h"
#include "tst_UBWidgetUtils.h"
#include "tst_UBGraphicsStroke.h"
#include "tst_UBGraphicsScene.h"
#include "tst_UBVisualRegression.h"
#include "tst_UBRecognition.h"
#include "tst_UBSmoothStrokeItem.h"
#include "tst_UBKeyboardPaletteColors.h"
#include "tst_UBDisplayManager.h"
#include "tst_UBExportSelection.h"
#include "tst_UBPageDeletion.h"
// #include "tst_UBBoardSubControllers.h" -- disabled until premoc is fixed

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("OpenSankoreTests");

    int status = 0;

    {
        TestUBStringUtils test;
        status |= QTest::qExec(&test, argc, argv);
    }

    {
        TestUBFileSystemUtils test;
        status |= QTest::qExec(&test, argc, argv);
    }

    {
        TestUBGeometryUtils test;
        status |= QTest::qExec(&test, argc, argv);
    }

    {
        TestUBCryptoUtils test;
        status |= QTest::qExec(&test, argc, argv);
    }

    {
        TestUBDocumentProxy test;
        status |= QTest::qExec(&test, argc, argv);
    }

    {
        TestUBSettings test;
        status |= QTest::qExec(&test, argc, argv);
    }

    {
        TestUBVersion test;
        status |= QTest::qExec(&test, argc, argv);
    }

    {
        TestUBBase32 test;
        status |= QTest::qExec(&test, argc, argv);
    }

    {
        TestUBIniFileParser test;
        status |= QTest::qExec(&test, argc, argv);
    }

    {
        TestUBMetadataDcSubsetAdaptor test;
        status |= QTest::qExec(&test, argc, argv);
    }

    {
        TestUBOEmbedParser test;
        status |= QTest::qExec(&test, argc, argv);
    }

    {
        TestUBSvgTransform test;
        status |= QTest::qExec(&test, argc, argv);
    }

    {
        TestUBForeignObjectsHelper test;
        status |= QTest::qExec(&test, argc, argv);
    }
    {
        TestUBTextTools test;
        status |= QTest::qExec(&test, argc, argv);
    }
    {
        TestUBStrokeProperty test;
        status |= QTest::qExec(&test, argc, argv);
    }
    {
        TestUBWidgetUtils test;
        status |= QTest::qExec(&test, argc, argv);
    }
    {
        TestUBGraphicsStroke test;
        status |= QTest::qExec(&test, argc, argv);
    }
    {
        TestUBGraphicsScene test;
        status |= QTest::qExec(&test, argc, argv);
    }
    {
        TestUBVisualRegression test;
        status |= QTest::qExec(&test, argc, argv);
    }
    {
        TestUBRecognition test;
        status |= QTest::qExec(&test, argc, argv);
    }
    {
        TestUBSmoothStrokeItem test;
        status |= QTest::qExec(&test, argc, argv);
    }
    {
        TestUBKeyboardPaletteColors test;
        status |= QTest::qExec(&test, argc, argv);
    }
    {
        TestUBDisplayManager test;
        status |= QTest::qExec(&test, argc, argv);
    }
    {
        TestUBExportSelection test;
        status |= QTest::qExec(&test, argc, argv);
    }
    {
        TestUBPageDeletion test;
        status |= QTest::qExec(&test, argc, argv);
    }

    // TestUBBoardSubControllers disabled until premoc is fixed
    // {
    //     TestUBBoardSubControllers test;
    //     status |= QTest::qExec(&test, argc, argv);
    // }

    return status;
}
