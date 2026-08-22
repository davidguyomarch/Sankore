/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#include "UBShapesController.h"

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "domain/UBShapeFactory.h"
#include "domain/UBAlignObjectManager.h"
#include "domain/UBAbstractGraphicsPathItem.h"

#include <QColorDialog>

UBShapesController::UBShapesController(QObject* parent)
    : QObject(parent)
    , mVisible(false)
{
}

void UBShapesController::setVisible(bool v)
{
    if (mVisible != v)
    {
        mVisible = v;
        emit visibleChanged();
    }
}

QColor UBShapesController::strokeColor() const
{
    return UBApplication::boardController->shapeFactory().strokeColor();
}

QColor UBShapesController::fillColor() const
{
    return UBApplication::boardController->shapeFactory().fillFirstColor();
}

int UBShapesController::strokeThickness() const
{
    // No public getter on UBShapeFactory for thickness; default to Medium
    return 1;
}

// --- Shape creation ---

void UBShapesController::createEllipse()
{
    UBApplication::boardController->shapeFactory().createEllipse(true);
}

void UBShapesController::createCircle()
{
    UBApplication::boardController->shapeFactory().createCircle(true);
}

void UBShapesController::createRectangle()
{
    UBApplication::boardController->shapeFactory().createRectangle(true);
}

void UBShapesController::createSquare()
{
    UBApplication::boardController->shapeFactory().createSquare(true);
}

void UBShapesController::createRegularPolygon(int sides)
{
    UBApplication::boardController->shapeFactory().createRegularPolygon(sides);
}

void UBShapesController::createPolygon()
{
    UBApplication::boardController->shapeFactory().createPolygon(true);
}

void UBShapesController::createLine()
{
    UBApplication::boardController->shapeFactory().createLine(true);
}

void UBShapesController::createPen()
{
    UBApplication::boardController->shapeFactory().createPen(true);
}

// --- Stroke properties ---

void UBShapesController::setStrokeThickness(int index)
{
    static const int thicknesses[] = { 3, 5, 10 };
    if (index >= 0 && index < 3)
    {
        UBApplication::boardController->shapeFactory().setThickness(thicknesses[index]);
        emit strokeThicknessChanged();
    }
}

void UBShapesController::setStrokeStyle(int style)
{
    Qt::PenStyle penStyle = Qt::SolidLine;
    switch (style)
    {
        case 0: penStyle = Qt::SolidLine; break;
        case 1: penStyle = Qt::DotLine; break;
        case 2: penStyle = Qt::CustomDashLine; break;
    }
    UBApplication::boardController->shapeFactory().setStrokeStyle(penStyle);
}

void UBShapesController::pickStrokeColor()
{
    QColor color = QColorDialog::getColor(strokeColor(), nullptr, tr("Stroke Color"));
    if (color.isValid())
    {
        UBApplication::boardController->shapeFactory().setStrokeColor(color);
        emit strokeColorChanged();
    }
}

// --- Fill properties ---

void UBShapesController::setFillTransparent()
{
    UBApplication::boardController->shapeFactory().setFillType(UBShapeFactory::Transparent);
}

void UBShapesController::setFillSolid()
{
    UBApplication::boardController->shapeFactory().setFillType(UBShapeFactory::Full);
}

void UBShapesController::setFillDense()
{
    UBApplication::boardController->shapeFactory().setFillType(UBShapeFactory::Dense);
}

void UBShapesController::setFillDiag()
{
    UBApplication::boardController->shapeFactory().setFillType(UBShapeFactory::Diag);
}

void UBShapesController::setFillGradient()
{
    UBApplication::boardController->shapeFactory().setFillType(UBShapeFactory::Gradient);
}

void UBShapesController::pickFillColor()
{
    QColor color = QColorDialog::getColor(fillColor(), nullptr, tr("Fill Color"));
    if (color.isValid())
    {
        UBApplication::boardController->shapeFactory().setFillingFirstColor(color);
        emit fillColorChanged();
    }
}

void UBShapesController::pickFillColor2()
{
    QColor color = QColorDialog::getColor(Qt::white, nullptr, tr("Gradient End Color"));
    if (color.isValid())
    {
        UBApplication::boardController->shapeFactory().setFillingSecondColor(color);
    }
}

// --- Arrows ---

void UBShapesController::setStartArrow(int type)
{
    UBApplication::boardController->shapeFactory().setStartArrowType(
        static_cast<UBAbstractGraphicsPathItem::ArrowType>(type));
}

void UBShapesController::setEndArrow(int type)
{
    UBApplication::boardController->shapeFactory().setEndArrowType(
        static_cast<UBAbstractGraphicsPathItem::ArrowType>(type));
}

// --- Alignment ---

void UBShapesController::alignLeft()
{
    UBAlignObjectManager mgr;
    mgr.alignToLeft();
}

void UBShapesController::alignRight()
{
    UBAlignObjectManager mgr;
    mgr.alignToRight();
}

void UBShapesController::alignTop()
{
    UBAlignObjectManager mgr;
    mgr.alignToTop();
}

void UBShapesController::alignBottom()
{
    UBAlignObjectManager mgr;
    mgr.alignToBottom();
}

void UBShapesController::alignHCenter()
{
    UBAlignObjectManager mgr;
    mgr.horizontalAlign();
}

void UBShapesController::alignVCenter()
{
    UBAlignObjectManager mgr;
    mgr.verticalAlign();
}

// --- Paint bucket ---

void UBShapesController::changeFill()
{
    UBApplication::boardController->shapeFactory().prepareChangeFill();
}

// --- Toggle ---

void UBShapesController::toggle()
{
    setVisible(!mVisible);
}
