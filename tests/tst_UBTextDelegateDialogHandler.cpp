#include "tst_UBTextDelegateDialogHandler.h"

#include <QGraphicsTextItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QSignalSpy>
#include <QFont>
#include <QColor>

/**
 * Mock delegate that emits the same signals as UBGraphicsTextItemDelegate.
 * This allows testing the handler wiring without instantiating the full
 * domain class (which requires UBApplication, UBBoardController, etc.).
 */
class MockTextDelegate : public QObject
{
    Q_OBJECT
public:
    explicit MockTextDelegate(QObject* parent = nullptr) : QObject(parent) {}

    void triggerFontChange(const QFont& font) { emit fontChangeRequested(font); }
    void triggerTextColorChange(const QColor& color) { emit textColorChangeRequested(color); }
    void triggerBackgroundColorChange(const QColor& color) { emit backgroundColorChangeRequested(color); }

public slots:
    void applyFont(const QFont& font) { mLastFont = font; mFontApplied = true; }
    void applyTextColor(const QColor& color) { mLastTextColor = color; mTextColorApplied = true; }
    void applyBackgroundColor(const QColor& color) { mLastBgColor = color; mBgColorApplied = true; }

signals:
    void fontChangeRequested(const QFont& currentFont);
    void textColorChangeRequested(const QColor& currentColor);
    void backgroundColorChangeRequested(const QColor& currentColor);

public:
    QFont mLastFont;
    QColor mLastTextColor;
    QColor mLastBgColor;
    bool mFontApplied = false;
    bool mTextColorApplied = false;
    bool mBgColorApplied = false;
};

void TestUBTextDelegateDialogHandler::testConnectDisconnect()
{
    // Verify signal/slot connections work via the mock delegate
    MockTextDelegate delegate;

    // Connect signals to the delegate's own apply slots (simulating handler behavior)
    connect(&delegate, &MockTextDelegate::fontChangeRequested,
            &delegate, &MockTextDelegate::applyFont);
    connect(&delegate, &MockTextDelegate::textColorChangeRequested,
            &delegate, &MockTextDelegate::applyTextColor);
    connect(&delegate, &MockTextDelegate::backgroundColorChangeRequested,
            &delegate, &MockTextDelegate::applyBackgroundColor);

    // Emit and verify
    QFont testFont("Arial", 14, QFont::Bold);
    delegate.triggerFontChange(testFont);
    QVERIFY(delegate.mFontApplied);
    QCOMPARE(delegate.mLastFont.family(), QString("Arial"));
    QCOMPARE(delegate.mLastFont.pointSize(), 14);
    QVERIFY(delegate.mLastFont.bold());

    QColor testColor(Qt::red);
    delegate.triggerTextColorChange(testColor);
    QVERIFY(delegate.mTextColorApplied);
    QCOMPARE(delegate.mLastTextColor, QColor(Qt::red));

    QColor bgColor(Qt::blue);
    delegate.triggerBackgroundColorChange(bgColor);
    QVERIFY(delegate.mBgColorApplied);
    QCOMPARE(delegate.mLastBgColor, QColor(Qt::blue));

    // Disconnect and verify signals no longer reach slots
    disconnect(&delegate, &MockTextDelegate::fontChangeRequested,
               &delegate, &MockTextDelegate::applyFont);

    delegate.mFontApplied = false;
    delegate.triggerFontChange(QFont("Courier", 10));
    QVERIFY(!delegate.mFontApplied); // Should NOT have been called
}

void TestUBTextDelegateDialogHandler::testSignalEmissionTriggersHandler()
{
    // Test that QSignalSpy correctly captures delegate signals
    MockTextDelegate delegate;

    QSignalSpy fontSpy(&delegate, &MockTextDelegate::fontChangeRequested);
    QSignalSpy colorSpy(&delegate, &MockTextDelegate::textColorChangeRequested);
    QSignalSpy bgSpy(&delegate, &MockTextDelegate::backgroundColorChangeRequested);

    QVERIFY(fontSpy.isValid());
    QVERIFY(colorSpy.isValid());
    QVERIFY(bgSpy.isValid());

    // Emit signals
    QFont font("Georgia", 18);
    delegate.triggerFontChange(font);
    QCOMPARE(fontSpy.count(), 1);
    QFont capturedFont = fontSpy.at(0).at(0).value<QFont>();
    QCOMPARE(capturedFont.family(), QString("Georgia"));
    QCOMPARE(capturedFont.pointSize(), 18);

    QColor color(128, 64, 32);
    delegate.triggerTextColorChange(color);
    QCOMPARE(colorSpy.count(), 1);
    QColor capturedColor = colorSpy.at(0).at(0).value<QColor>();
    QCOMPARE(capturedColor, color);

    QColor bgColor(0, 255, 128);
    delegate.triggerBackgroundColorChange(bgColor);
    QCOMPARE(bgSpy.count(), 1);
}

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
    // Test that background color pattern works on a QGraphicsTextItem
    QGraphicsScene scene;
    QGraphicsTextItem* textItem = scene.addText("Background test");

    // QGraphicsTextItem doesn't have setBackgroundColor directly,
    // but we can test the document background
    QTextDocument* doc = textItem->document();
    QVERIFY(doc != nullptr);

    // Verify the text item is valid and can hold content
    QCOMPARE(textItem->toPlainText(), QString("Background test"));

    // The real UBGraphicsTextItem::setBackgroundColor uses a custom
    // implementation. Here we just verify the pattern is sound.
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

#include "tst_UBTextDelegateDialogHandler.moc"
