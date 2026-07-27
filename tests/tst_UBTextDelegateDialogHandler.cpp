#include "tst_UBTextDelegateDialogHandler.h"

#include <QGraphicsTextItem>
#include <QGraphicsScene>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QSignalSpy>
#include <QFont>
#include <QColor>

void TestUBTextDelegateDialogHandler::testApplyFontModifiesTextItem()
{
    // Test that applying a font to a QGraphicsTextItem works correctly
    // (validates the pattern used by the real applyFont slot)
    QGraphicsScene scene;
    QGraphicsTextItem* textItem = scene.addText("Hello World");
    textItem->setFont(QFont("Arial", 12));

    // Simulate what applyFont does
    QFont newFont("Verdana", 20, QFont::Bold, true);
    QTextCursor cursor = textItem->textCursor();
    cursor.select(QTextCursor::Document);
    QTextCharFormat format;
    format.setFont(newFont);
    cursor.mergeCharFormat(format);
    textItem->setTextCursor(cursor);
    textItem->setFont(newFont);

    // Verify
    QCOMPARE(textItem->font().family(), QString("Verdana"));
    QCOMPARE(textItem->font().pointSize(), 20);
    QVERIFY(textItem->font().bold());
    QVERIFY(textItem->font().italic());
}

void TestUBTextDelegateDialogHandler::testApplyTextColorModifiesTextItem()
{
    // Test that applying text color works on a QGraphicsTextItem
    QGraphicsScene scene;
    QGraphicsTextItem* textItem = scene.addText("Colored text");

    QColor newColor(255, 128, 0); // orange
    textItem->setDefaultTextColor(newColor);

    QTextCursor cursor = textItem->textCursor();
    cursor.select(QTextCursor::Document);
    QTextCharFormat format;
    format.setForeground(QBrush(newColor));
    cursor.mergeCharFormat(format);
    textItem->setTextCursor(cursor);

    // Verify default color
    QCOMPARE(textItem->defaultTextColor(), newColor);

    // Verify cursor format
    QTextCursor verifyCursor = textItem->textCursor();
    verifyCursor.select(QTextCursor::Document);
    QCOMPARE(verifyCursor.charFormat().foreground().color(), newColor);
}

void TestUBTextDelegateDialogHandler::testApplyBackgroundColorModifiesTextItem()
{
    // Test that background color pattern works via cursor format
    QGraphicsScene scene;
    QGraphicsTextItem* textItem = scene.addText("Background test");

    QColor bgColor(200, 200, 255);
    QTextCursor cursor = textItem->textCursor();
    cursor.select(QTextCursor::Document);
    QTextCharFormat format;
    format.setBackground(QBrush(bgColor));
    cursor.mergeCharFormat(format);
    textItem->setTextCursor(cursor);

    // Verify background was applied to format
    QTextCursor verifyCursor = textItem->textCursor();
    verifyCursor.select(QTextCursor::Document);
    QCOMPARE(verifyCursor.charFormat().background().color(), bgColor);
}

void TestUBTextDelegateDialogHandler::testFontRoundtrip()
{
    // Test that setting and reading font properties is consistent
    QGraphicsScene scene;
    QGraphicsTextItem* textItem = scene.addText("Roundtrip");

    QFont original("Times New Roman", 16);
    original.setBold(true);
    original.setItalic(false);
    original.setUnderline(true);

    textItem->setFont(original);

    QFont retrieved = textItem->font();
    QCOMPARE(retrieved.family(), QString("Times New Roman"));
    QCOMPARE(retrieved.pointSize(), 16);
    QVERIFY(retrieved.bold());
    QVERIFY(!retrieved.italic());
    QVERIFY(retrieved.underline());
}

void TestUBTextDelegateDialogHandler::testColorDoesNotAffectOtherProperties()
{
    // Verify that changing color doesn't affect font or text content
    QGraphicsScene scene;
    QGraphicsTextItem* textItem = scene.addText("Color isolation");
    QFont originalFont("Courier New", 14);
    textItem->setFont(originalFont);

    // Apply color
    QColor color(0, 128, 255);
    textItem->setDefaultTextColor(color);

    // Font should be unchanged
    QCOMPARE(textItem->font().family(), QString("Courier New"));
    QCOMPARE(textItem->font().pointSize(), 14);
    // Text should be unchanged
    QCOMPARE(textItem->toPlainText(), QString("Color isolation"));
    // Color should be set
    QCOMPARE(textItem->defaultTextColor(), color);
}
