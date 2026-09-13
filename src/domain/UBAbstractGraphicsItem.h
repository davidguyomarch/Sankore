#ifndef UBSHAPE_H
#define UBSHAPE_H

#include "UBItem.h"
#include "core/UB.h"

#include <QAbstractGraphicsShapeItem>

class UBAbstractGraphicsItem : public UBItem, public UBGraphicsItem, public QAbstractGraphicsShapeItem
{
public:
    UBAbstractGraphicsItem(QGraphicsItem *parent = 0);

    virtual ~UBAbstractGraphicsItem();

    bool hasFillingProperty() const;

    bool hasStrokeProperty() const;

    bool hasGradient() const;

    void setStyle(Qt::PenStyle penStyle);

    void setStyle(Qt::BrushStyle brushStyle);

    void setStyle(Qt::BrushStyle brushStyle, Qt::PenStyle penStyle);

    void setFillColor(const QColor& color);

    void setStrokeColor(const QColor& color);

    void setStrokeSize(int size);

    enum FillPattern{   // Warning : those values are persisted. Do NOT change this order. Only add new values at the end of enum.
        FillPattern_None,
        FillPattern_Diag1,
        FillPattern_Dot1
    };

    FillPattern fillPattern() const {return mFillPatern;}
    void setFillPattern(FillPattern pattern);

    // UBItem interface
    void setUuid(const QUuid &pUuid);

    void initializeFillingProperty();

    void initializeStrokeProperty();

    // #317/#319: force the stroke/fill capability flags. Used when copying an
    // item's parameters so a clone keeps the same capabilities as the source.
    void setHasStrokeProperty(bool has) { mHasStrokeProperty = has; }
    void setHasFillingProperty(bool has) { mHasFillingProperty = has; }

    //disambiguation from UBGraphicsItem and QabstractGraphicsShapeItem
    virtual int type() const = 0;

    //must be define, because the delegate use it
    virtual QRectF boundingRect() const{ return QRect(); }

    virtual void copyItemParameters(UBItem *copy) const;

protected:
    void setStyle(QPainter *painter);

    QRectF adjustBoundingRect(QRectF rect) const;

    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    FillPattern mFillPatern;
    // #317/#319: explicit capability flags. The old test `pen() != QPen()` was
    // broken: a plain black 1px stroke equals the default QPen sentinel, so a
    // freshly created shape reported hasStrokeProperty()==false and every
    // setStrokeColor/setStrokeSize (and the day/night recolor) was silently
    // skipped. The flags are set true by initializeStrokeProperty()/
    // initializeFillingProperty(), which shapes call in their constructors.
    bool mHasStrokeProperty = false;
    bool mHasFillingProperty = false;
    QBitmap patternPoint();
    QBitmap patternDiag();
};

#endif // UBSHAPE_H
