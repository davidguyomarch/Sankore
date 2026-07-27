#ifndef TST_UBTEXTDELEGATEDIALOGHANDLER_H
#define TST_UBTEXTDELEGATEDIALOGHANDLER_H

#include <QtTest>

class TestUBTextDelegateDialogHandler : public QObject
{
    Q_OBJECT

private slots:
    void testConnectDisconnect();
    void testSignalEmissionTriggersHandler();
    void testApplyFontModifiesTextItem();
    void testApplyTextColorModifiesTextItem();
    void testApplyBackgroundColorModifiesTextItem();
};

#endif // TST_UBTEXTDELEGATEDIALOGHANDLER_H
