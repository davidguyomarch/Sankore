/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef UBTOOLCONTROLLER_H
#define UBTOOLCONTROLLER_H

#include <QObject>
#include <QColor>
#include <QList>
#include "core/UB.h"  // UBStylusTool::Enum

class UBAbstractDrawRuler;
class UBSettings;

/**
 * ToolController — single source of truth for the active drawing tool,
 * colors, widths, and geometric instrument tracking.
 *
 * Replaces the former UBDrawingController singleton (issue #148).
 * Exposed to QML via Q_PROPERTY. Accessible globally via toolController().
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

    // --- Singleton access ---
    static UBToolController* toolController();
    static void destroy();

    // --- Active tool ---
    int activeTool() const;
    int stylusTool() const { return m_activeTool; }  // alias for legacy code
    void setActiveTool(int tool);
    bool isDrawingTool() const;
    int latestDrawingTool() const;

    // --- Pen ---
    QColor penColor() const;
    int penColorIndex() const;
    void setPenColorIndex(int index);
    int penWidthIndex() const;
    void setPenWidthIndex(int index);
    QList<QColor> penColors() const;
    void setPenColor(bool onDarkBackground, const QColor& color, int pIndex);

    // --- Marker ---
    QColor markerColor() const;
    int markerColorIndex() const;
    void setMarkerColorIndex(int index);
    int markerWidthIndex() const;
    void setMarkerWidthIndex(int index);
    QList<QColor> markerColors() const;
    void setMarkerColor(bool onDarkBackground, const QColor& color, int pIndex);
    void setMarkerAlpha(qreal alpha);

    // --- Eraser ---
    int eraserWidthIndex() const;
    void setEraserWidthIndex(int index);

    // --- Tool-aware convenience accessors ---
    QList<QColor> currentColors() const;
    int currentColorIndex() const;
    void setCurrentColorIndex(int index);
    int currentWidthIndex() const;
    void setCurrentWidthIndex(int index);
    int currentToolWidthIndex() const;
    qreal currentToolWidth() const;
    int currentToolColorIndex() const;
    QColor currentToolColor() const;
    QColor toolColor(bool onDarkBackground) const;

    bool showDrawingProps() const;

    // --- Shapes palette ---
    bool shapesVisible() const;
    void setShapesVisible(bool visible);
    Q_INVOKABLE void createShape(const QString& shape);
    Q_INVOKABLE void activateFillTool();
    Q_INVOKABLE void applyStrokeToSelection();
    Q_INVOKABLE void alignSelection();

    // --- Geometric instrument tracking ---
    UBAbstractDrawRuler* mActiveRuler = nullptr;

    // --- Desktop mode ---
    void setInDesktopMode(bool mode) { m_isDesktopMode = mode; }
    bool isInDesktopMode() const { return m_isDesktopMode; }

public slots:
    void toggleShapes();
    void undo();
    void redo();
    void setStylusTool(int tool);  // full side-effect version (QAction sync, deselect, etc.)
    void setLineWidthIndex(int index);
    void setColorIndex(int index);
    void deactivateCreationModeForGraphicsPathItems();
    void onActiveSceneChanged();

signals:
    void activeToolChanged();
    void stylusToolChanged(int tool);
    void colorPaletteChanged();
    void lineWidthIndexChanged(int index);
    void colorIndexChanged(int index);
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

private:
    explicit UBToolController(QObject* parent = nullptr);

    int m_activeTool;
    UBStylusTool::Enum m_latestDrawingTool;
    bool m_shapesVisible;
    bool m_isDesktopMode;
    UBSettings* mSettings;

    static UBToolController* sToolController;
};

#endif // UBTOOLCONTROLLER_H
