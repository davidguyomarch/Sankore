/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#ifndef UBTOOLCONTROLLER_H
#define UBTOOLCONTROLLER_H

#include <QObject>
#include <QColor>
#include <QList>

/**
 * ToolController — single source of truth for the active drawing tool.
 *
 * Exposed to QML via Q_PROPERTY. No QAction involved.
 * The QML UI binds directly to activeTool, penColor, penWidthIndex, etc.
 * When a property changes, the underlying UBDrawingController is updated.
 */
class UBToolController : public QObject
{
    Q_OBJECT

    // Tool enum exposed to QML
    Q_PROPERTY(int activeTool READ activeTool WRITE setActiveTool NOTIFY activeToolChanged)

    // Pen properties
    Q_PROPERTY(QColor penColor READ penColor NOTIFY penColorChanged)
    Q_PROPERTY(int penColorIndex READ penColorIndex WRITE setPenColorIndex NOTIFY penColorChanged)
    Q_PROPERTY(int penWidthIndex READ penWidthIndex WRITE setPenWidthIndex NOTIFY penWidthChanged)
    Q_PROPERTY(QList<QColor> penColors READ penColors NOTIFY penColorsChanged)

    // Marker properties
    Q_PROPERTY(QColor markerColor READ markerColor NOTIFY markerColorChanged)
    Q_PROPERTY(int markerColorIndex READ markerColorIndex WRITE setMarkerColorIndex NOTIFY markerColorChanged)
    Q_PROPERTY(int markerWidthIndex READ markerWidthIndex WRITE setMarkerWidthIndex NOTIFY markerWidthChanged)
    Q_PROPERTY(QList<QColor> markerColors READ markerColors NOTIFY markerColorsChanged)

    // Eraser
    Q_PROPERTY(int eraserWidthIndex READ eraserWidthIndex WRITE setEraserWidthIndex NOTIFY eraserWidthChanged)

    // Tool-aware convenience properties for DrawingPropsBar QML
    Q_PROPERTY(QList<QColor> currentColors READ currentColors NOTIFY currentColorsChanged)
    Q_PROPERTY(int currentColorIndex READ currentColorIndex WRITE setCurrentColorIndex NOTIFY currentColorIndexChanged)
    Q_PROPERTY(int currentWidthIndex READ currentWidthIndex WRITE setCurrentWidthIndex NOTIFY currentWidthIndexChanged)

    // Drawing properties panel visibility
    Q_PROPERTY(bool showDrawingProps READ showDrawingProps NOTIFY activeToolChanged)

    // Shapes palette
    Q_PROPERTY(bool shapesVisible READ shapesVisible WRITE setShapesVisible NOTIFY shapesVisibleChanged)

public:
    // Mirror UBStylusTool::Enum for QML access
    enum Tool {
        Pen = 0,
        Eraser,
        Marker,
        Selector,
        Play,
        Hand,
        ZoomIn,
        ZoomOut,
        Pointer,
        Line,
        Text,
        Capture,
        RichText,
        ChangeFill,
        Drawing,
        Ocr
    };
    Q_ENUM(Tool)

    explicit UBToolController(QObject* parent = nullptr);

    int activeTool() const;
    void setActiveTool(int tool);

    QColor penColor() const;
    int penColorIndex() const;
    void setPenColorIndex(int index);
    int penWidthIndex() const;
    void setPenWidthIndex(int index);
    QList<QColor> penColors() const;

    QColor markerColor() const;
    int markerColorIndex() const;
    void setMarkerColorIndex(int index);
    int markerWidthIndex() const;
    void setMarkerWidthIndex(int index);
    QList<QColor> markerColors() const;

    int eraserWidthIndex() const;
    void setEraserWidthIndex(int index);

    // Tool-aware convenience accessors
    QList<QColor> currentColors() const;
    int currentColorIndex() const;
    void setCurrentColorIndex(int index);
    int currentWidthIndex() const;
    void setCurrentWidthIndex(int index);

    bool showDrawingProps() const;

    bool shapesVisible() const;
    void setShapesVisible(bool visible);

    // Shape creation — called from QML ShapesPaletteV2
    Q_INVOKABLE void createShape(const QString& shape);

public slots:
    void toggleShapes();
    void undo();
    void redo();

signals:
    void activeToolChanged();
    void penColorChanged();
    void penWidthChanged();
    void penColorsChanged();
    void markerColorChanged();
    void markerWidthChanged();
    void markerColorsChanged();
    void eraserWidthChanged();
    void shapesVisibleChanged();
    void currentColorsChanged();
    void currentColorIndexChanged();
    void currentWidthIndexChanged();

private slots:
    void onExternalToolChanged(int tool);
    void onExternalColorChanged(int index);
    void onExternalWidthChanged(int index);

private:
    int m_activeTool;
    bool m_shapesVisible;
};

#endif // UBTOOLCONTROLLER_H
