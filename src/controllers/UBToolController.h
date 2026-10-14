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
#include <QString>
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
    // #475: the active tool's custom (free, non-palette) colour, or an invalid
    // QColor when none is set. Drives the 5th "custom" swatch in the
    // DrawingPropsBar: invisible at rest (invalid), shows the chosen colour and
    // is highlighted when active. NOTIFY shared with the selection signal since
    // picking a custom colour / selecting a swatch both change it.
    Q_PROPERTY(QColor currentCustomColor READ currentCustomColor NOTIFY currentColorIndexChanged)
    // Whether the active tool currently has a custom colour (QColor validity is
    // not exposed to QML, so expose an explicit bool for the 5th swatch).
    Q_PROPERTY(bool hasCurrentCustomColor READ hasCurrentCustomColor NOTIFY currentColorIndexChanged)

    // Drawing properties panel visibility
    Q_PROPERTY(bool showDrawingProps READ showDrawingProps NOTIFY activeToolChanged)

    // #369: line-shape endpoint (arrow) + stroke-style options, shown in the
    // DrawingPropsBar only when the active shape is the Line.
    Q_PROPERTY(bool showLineOptions READ showLineOptions NOTIFY currentShapeChanged)
    Q_PROPERTY(int startArrowType READ startArrowType NOTIFY lineOptionsChanged)
    Q_PROPERTY(int endArrowType READ endArrowType NOTIFY lineOptionsChanged)
    Q_PROPERTY(bool lineDashed READ lineDashed NOTIFY lineOptionsChanged)

    // Shapes palette
    Q_PROPERTY(bool shapesVisible READ shapesVisible WRITE setShapesVisible NOTIFY shapesVisibleChanged)
    // #318: the currently selected shape action ("ellipse", "rectangle", ...),
    // so ShapesPaletteV2 can highlight it in blue. Empty when no shape is active.
    Q_PROPERTY(QString currentShape READ currentShape NOTIFY currentShapeChanged)

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
    QColor currentCustomColor() const;   // #475: active tool's free colour (invalid if none)
    bool hasCurrentCustomColor() const;  // #475: whether a custom colour is active
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
    QString currentShape() const { return m_currentShape; }
    Q_INVOKABLE void createShape(const QString& shape);
    Q_INVOKABLE void activateFillTool();
    Q_INVOKABLE void alignSelection();

    // #369: line-shape endpoint + stroke-style options for the V2 props bar.
    // Arrow type ints match UBAbstractGraphicsPathItem::ArrowType
    // (0 = None, 1 = Round, 2 = Arrow).
    bool showLineOptions() const;
    int startArrowType() const;
    int endArrowType() const;
    bool lineDashed() const;
    Q_INVOKABLE void setStartArrowType(int arrowType);
    Q_INVOKABLE void setEndArrowType(int arrowType);
    Q_INVOKABLE void setLineDashed(bool dashed);

    // #475: open the unified colour picker (UBColorPickerDialog) and apply the
    // chosen free colour to the ACTIVE tool, without touching the 4 fixed
    // palette swatches:
    //  - Pen/Line/Marker → a transient free-colour override in UBSettings read
    //    by the draw pipeline (see UBSettings::setTransient{Pen,Marker}Color);
    //  - Drawing (shapes) → shapeFactory().setStrokeColor (plain QColor, no slot);
    //  - ChangeFill (bucket) → shapeFactory().setFillingFirstColor.
    // No-op if the user cancels the dialog.
    Q_INVOKABLE void pickCustomColor();

    // #492: edit one of the 4 FIXED palette swatches. Opens the unified picker
    // seeded with the slot's current colour and, on accept, overwrites that slot
    // for the CURRENT background (pen or marker palette depending on the active
    // tool) — persisted. Triggered by a long-press / right-click on a swatch;
    // distinct from pickCustomColor (which never touches the swatches).
    Q_INVOKABLE void editColorSwatch(int index);

    // #475: explicit swatch selection, called from the DrawingPropsBar on a REAL
    // click. Prefer this over writing the currentColorIndex property: a property
    // write-back from a QML binding re-evaluation (e.g. a Repeater delegate
    // rebuild on the software backend) is indistinguishable from a user click and
    // would silently reset the selection. Routing clicks through an explicit
    // invokable makes a binding re-eval a no-op (it does not call this).
    Q_INVOKABLE void selectColorIndex(int index);

    // --- Eraser options (issue #249) ---
    Q_INVOKABLE void eraseAllInk();     // erase all strokes/annotations on the page
    Q_INVOKABLE void eraseWholePage();  // erase the entire page content

    // --- Geometric instrument tracking ---
    UBAbstractDrawRuler* mActiveRuler = nullptr;

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
    void currentShapeChanged();
    void lineOptionsChanged();   // #369
    void currentColorsChanged();
    void currentColorIndexChanged();
    void currentWidthIndexChanged();

private:
    explicit UBToolController(QObject* parent = nullptr);

    // #475: the colour the custom picker should open on for the active tool —
    // the free colour if one is set, else the tool's effective colour (pen slot,
    // marker slot, shape stroke, or fill). Used by pickCustomColor().
    QColor seedColorForActiveTool() const;

    int m_activeTool;
    UBStylusTool::Enum m_latestDrawingTool;
    bool m_shapesVisible;
    UBSettings* mSettings;

    // #319: shape tool stroke color/width (the shape factory stores the color
    // and width but not the palette indices the DrawingPropsBar binds to).
    int m_shapeColorIndex = 0;
    int m_shapeWidthIndex = 1;
    // #429-followup: shape FILL color index (fill tool, separate from stroke).
    int m_shapeFillColorIndex = 0;

    // #475: custom (free, non-palette) colour per shape-side tool. Pen/Marker use
    // the transient channel in UBSettings instead; these cover Drawing (stroke)
    // and ChangeFill (fill), which push straight to the factory with no slot.
    // Invalid QColor = no custom colour → the fixed palette index is in effect.
    QColor m_shapeCustomColor;
    QColor m_shapeFillCustomColor;

    // #318: currently selected shape action, for the palette highlight.
    QString m_currentShape;

    static UBToolController* sToolController;
};

#endif // UBTOOLCONTROLLER_H
