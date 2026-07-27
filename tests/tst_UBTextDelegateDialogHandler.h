#ifndef TST_UBTEXTDELEGATEDIALOGHANDLER_H
#define TST_UBTEXTDELEGATEDIALOGHANDLER_H

#include <QtTest>

class TestUBTextDelegateDialogHandler : public QObject
{
    Q_OBJECT

private slots:
    void testApplyFontModifiesTextItem();
    void testApplyTextColorModifiesTextItem();
    void testApplyBackgroundColorModifiesTextItem();
    void testFontRoundtrip();
    void testColorDoesNotAffectOtherProperties();
};

#endif // TST_UBTEXTDELEGATEDIALOGHANDLER_H
