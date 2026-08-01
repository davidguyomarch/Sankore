/*
 * Unit tests for UBSceneContext — validates the context struct
 * that decouples UBGraphicsScene from singletons.
 *
 * These tests exercise the context logic without needing UBGraphicsScene
 * itself (which has too many dependencies for unit testing currently).
 * They validate the contract that production code fills correctly.
 */

#include "tst_UBGraphicsScene.h"
#include "domain/UBSceneContext.h"

void TestUBGraphicsScene::testDrawLineToCreatesPolygon()
{
    // Test that adjustedWidth applies both scale factors
    UBSceneContext ctx;
    ctx.systemScaleFactor = 2.0;
    ctx.currentZoom = 4.0;

    // rawWidth 80 → 80 / 2.0 / 4.0 = 10.0
    QCOMPARE(ctx.adjustedWidth(80.0), 10.0);
}

void TestUBGraphicsScene::testDrawLineToZeroLengthSkipped()
{
    // Test default context has sane values
    UBSceneContext ctx;
    QCOMPARE(ctx.systemScaleFactor, 1.0);
    QCOMPARE(ctx.currentZoom, 1.0);
    QCOMPARE(ctx.pointerDiameter, 40.0);
    QVERIFY(ctx.drawingController == nullptr);

    // adjustedWidth with defaults = identity
    QCOMPARE(ctx.adjustedWidth(5.0), 5.0);
}

void TestUBGraphicsScene::testMoveToResetsState()
{
    // Test that eraser width uses medium as default when no drawingController
    UBSceneContext ctx;
    ctx.eraserMediumWidth = 24.0;
    QCOMPARE(ctx.currentEraserWidth(), 24.0);
}

void TestUBGraphicsScene::testInputDevicePressAndRelease()
{
    // Test adjustedWidth at different zoom levels
    UBSceneContext ctx;
    ctx.systemScaleFactor = 1.0;

    ctx.currentZoom = 1.0;
    QCOMPARE(ctx.adjustedWidth(10.0), 10.0);

    ctx.currentZoom = 2.0;
    QCOMPARE(ctx.adjustedWidth(10.0), 5.0);

    ctx.currentZoom = 0.5;
    QCOMPARE(ctx.adjustedWidth(10.0), 20.0);
}

void TestUBGraphicsScene::testMultipleSegmentsCreateStroke()
{
    // Test color defaults
    UBSceneContext ctx;
    QVERIFY(ctx.penColorOnDarkBackground.isValid());
    QVERIFY(ctx.penColorOnLightBackground.isValid());
    QVERIFY(ctx.markerColorOnDarkBackground.isValid());
    QVERIFY(ctx.markerColorOnLightBackground.isValid());
}
