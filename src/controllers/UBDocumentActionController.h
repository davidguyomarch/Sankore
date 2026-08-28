/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#ifndef UBDOCUMENTACTIONCONTROLLER_H
#define UBDOCUMENTACTIONCONTROLLER_H

#include <QObject>

class UBDocumentController;

/**
 * UBDocumentActionController — exposes document management actions to QML.
 *
 * Wraps UBDocumentController actions with Q_INVOKABLE methods for the
 * DocumentsTopBar QML component.
 */
class UBDocumentActionController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int activeMode READ activeMode WRITE setActiveMode NOTIFY activeModeChanged)
    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY selectionChanged)
    Q_PROPERTY(QString documentTitle READ documentTitle NOTIFY selectionChanged)

public:
    explicit UBDocumentActionController(QObject* parent = nullptr);

    // Mode: 0=Board, 1=Documents, 2=Desktop (matches UBAppController)
    int activeMode() const;
    void setActiveMode(int mode);

    bool hasSelection() const;
    QString documentTitle() const;

public slots:
    void newDocument();
    void newFolder();
    void importFile();
    void exportDocument();
    void renameItem();
    void duplicateItem();
    void deleteItem();
    void openInBoard();
    void quit();

signals:
    void activeModeChanged();
    void selectionChanged();
};

#endif // UBDOCUMENTACTIONCONTROLLER_H
