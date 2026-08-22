/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#ifndef UBSHAPESCONTROLLER_H
#define UBSHAPESCONTROLLER_H

#include <QObject>
#include <QColor>

/**
 * @brief C++ bridge between the QML ShapesPalette and UBShapeFactory.
 *
 * Exposes shape creation, stroke/fill properties, and alignment tools to QML.
 */
class UBShapesController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(QColor strokeColor READ strokeColor NOTIFY strokeColorChanged)
    Q_PROPERTY(QColor fillColor READ fillColor NOTIFY fillColorChanged)
    Q_PROPERTY(int strokeThickness READ strokeThickness NOTIFY strokeThicknessChanged)

public:
    explicit UBShapesController(QObject* parent = nullptr);

    bool visible() const { return mVisible; }
    void setVisible(bool v);

    QColor strokeColor() const;
    QColor fillColor() const;
    int strokeThickness() const;

public slots:
    // Shape creation
    void createEllipse();
    void createCircle();
    void createRectangle();
    void createSquare();
    void createRegularPolygon(int sides);
    void createPolygon();
    void createLine();
    void createPen();

    // Stroke properties
    void setStrokeThickness(int index);  // 0=Fine(3), 1=Medium(5), 2=Large(10)
    void setStrokeStyle(int style);      // 0=Solid, 1=Dot, 2=CustomDash
    void pickStrokeColor();

    // Fill properties
    void setFillTransparent();
    void setFillSolid();
    void setFillDense();
    void setFillDiag();
    void setFillGradient();
    void pickFillColor();
    void pickFillColor2();

    // Arrows
    void setStartArrow(int type);  // 0=None, 1=Arrow, 2=Round
    void setEndArrow(int type);

    // Alignment
    void alignLeft();
    void alignRight();
    void alignTop();
    void alignBottom();
    void alignHCenter();
    void alignVCenter();

    // Paint bucket
    void changeFill();

    // Toggle visibility
    void toggle();

signals:
    void visibleChanged();
    void strokeColorChanged();
    void fillColorChanged();
    void strokeThicknessChanged();

private:
    bool mVisible;
};

#endif // UBSHAPESCONTROLLER_H
