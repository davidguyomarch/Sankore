/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#ifndef UBSTYLUSCONTROLLER_H
#define UBSTYLUSCONTROLLER_H

#include <QObject>
#include <QVariantList>

class QAction;

/**
 * @brief C++ bridge between the QML StylusPalette and the existing action system.
 *
 * Exposes tool list and active tool index to QML.
 * Routes clicks back to the QAction instances that drive UBDrawingController.
 */
class UBStylusController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int activeToolIndex READ activeToolIndex NOTIFY activeToolChanged)
    Q_PROPERTY(QVariantList tools READ tools CONSTANT)
    Q_PROPERTY(bool vertical READ vertical NOTIFY orientationChanged)

public:
    explicit UBStylusController(QObject* parent = nullptr);

    int activeToolIndex() const { return mActiveIndex; }
    QVariantList tools() const { return mTools; }
    bool vertical() const { return mVertical; }

    void setVertical(bool v);

    /** Register a tool with its icon resource path and QAction. */
    void addTool(const QString& name, const QString& iconSource, QAction* action, bool isToggle = false);

    /** Rebuild after all tools added. */
    void finalize();

public slots:
    /** Called from QML when a tool button is clicked. */
    void selectTool(int index);

signals:
    void activeToolChanged();
    void orientationChanged();
    void toolDoubleClicked(int toolIndex);

private slots:
    void onActionToggled(bool checked);

private:
    void updateActiveFromActions();

    struct ToolEntry {
        QString name;
        QString iconSource;
        QAction* action;
        bool isToggle; // toggle buttons don't participate in exclusive group
    };

    QList<ToolEntry> mEntries;
    QVariantList mTools;       // exposed to QML (list of {name, iconSource, isToggle})
    int mActiveIndex = -1;
    bool mVertical = true;
};

#endif // UBSTYLUSCONTROLLER_H
