/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#ifndef UBDRAWINGPROPERTIESCONTROLLER_H
#define UBDRAWINGPROPERTIESCONTROLLER_H

#include <QObject>
#include <QColor>
#include <QVariantList>

class UBSettings;
class UBDrawingController;

/**
 * @brief C++ bridge between QML DrawingProperties panel and UBDrawingController.
 *
 * Exposes colors (pen or marker, depending on active tool), width options,
 * eraser sizes, and active indices to QML.
 */
class UBDrawingPropertiesController : public QObject
{
    Q_OBJECT

    // Active tool section (pen, marker, eraser, other)
    Q_PROPERTY(QString activeTool READ activeTool NOTIFY activeToolChanged)
    Q_PROPERTY(bool isDrawingTool READ isDrawingTool NOTIFY activeToolChanged)

    // Color palette (4 colors, context-dependent)
    Q_PROPERTY(QVariantList colors READ colors NOTIFY colorsChanged)
    Q_PROPERTY(int colorIndex READ colorIndex NOTIFY colorIndexChanged)

    // Width (3 sizes: Fine, Medium, Strong)
    Q_PROPERTY(int widthIndex READ widthIndex NOTIFY widthIndexChanged)

    // Eraser
    Q_PROPERTY(int eraserWidthIndex READ eraserWidthIndex NOTIFY eraserWidthIndexChanged)

    // Visibility: panel shows only when pen/marker/eraser is active
    Q_PROPERTY(bool visible READ visible NOTIFY visibleChanged)

public:
    explicit UBDrawingPropertiesController(QObject* parent = nullptr);

    QString activeTool() const;
    bool isDrawingTool() const;

    QVariantList colors() const;
    int colorIndex() const;

    int widthIndex() const;
    int eraserWidthIndex() const;

    bool visible() const;

public slots:
    void setColorIndex(int index);
    void setWidthIndex(int index);
    void setEraserWidthIndex(int index);

signals:
    void activeToolChanged();
    void colorsChanged();
    void colorIndexChanged();
    void widthIndexChanged();
    void eraserWidthIndexChanged();
    void visibleChanged();

private slots:
    void onStylusToolChanged(int tool);
    void onColorPaletteChanged();
    void onColorIndexChangedFromController(int index);
    void onLineWidthIndexChanged(int index);

private:
    UBSettings* mSettings;
    UBDrawingController* mDrawingController;
    bool mVisible;
};

#endif // UBDRAWINGPROPERTIESCONTROLLER_H
