#include "UBDrawingArrowsPropertiesPalette.h"

#include "core/UBApplication.h"
#include "UBMainWindow.h"
#include "board/UBBoardController.h"
#include "domain/UBShapeFactory.h"

UBDrawingArrowsPropertiesPalette::UBDrawingArrowsPropertiesPalette(Qt::Orientation orient, QWidget *parent)
    :UBAbstractSubPalette(orient, parent)
    ,mButtonGroupStartArrowStyle(nullptr)
    ,mButtonGroupEndArrowStyle(nullptr)
{
    hide();

    QHBoxLayout * layoutStartArrowStyle = new QHBoxLayout();
    layoutStartArrowStyle->setSpacing(0);

    QHBoxLayout * layoutEndArrowStyle = new QHBoxLayout();
    layoutEndArrowStyle->setSpacing(0);

    QHBoxLayout* mainLayout = dynamic_cast<QHBoxLayout*>(layout());
    if (mainLayout)
    {
        mainLayout->addLayout(layoutStartArrowStyle);
        mainLayout->addLayout(layoutEndArrowStyle);
    }

    UBActionPaletteButton* btnStartArrowStyleNone = new UBActionPaletteButton(UBApplication::mainWindow->actionStartArrowStyleNone, this);
    btnStartArrowStyleNone->setStyleSheet(styleSheetLeftGroupedButton);
    layoutStartArrowStyle->addWidget(btnStartArrowStyleNone);
    UBApplication::mainWindow->actionStartArrowStyleNone->setChecked(true);

    UBActionPaletteButton* btnStartArrowStyleArrow = new UBActionPaletteButton(UBApplication::mainWindow->actionStartArrowStyleArrow, this);
    btnStartArrowStyleArrow->setStyleSheet(styleSheetCenterGroupedButton);
    layoutStartArrowStyle->addWidget(btnStartArrowStyleArrow);

    UBActionPaletteButton* btnStartArrowStyleRound = new UBActionPaletteButton(UBApplication::mainWindow->actionStartArrowStyleRound, this);
    btnStartArrowStyleRound->setStyleSheet(styleSheetRightGroupedButton);
    layoutStartArrowStyle->addWidget(btnStartArrowStyleRound);


    UBActionPaletteButton* btnEndArrowStyleNone = new UBActionPaletteButton(UBApplication::mainWindow->actionEndArrowStyleNone, this);
    btnEndArrowStyleNone->setStyleSheet(styleSheetLeftGroupedButton);
    layoutEndArrowStyle->addWidget(btnEndArrowStyleNone);
    UBApplication::mainWindow->actionEndArrowStyleNone->setChecked(true);

    UBActionPaletteButton* btnEndArrowStyleArrow = new UBActionPaletteButton(UBApplication::mainWindow->actionEndArrowStyleArrow, this);
    btnEndArrowStyleArrow->setStyleSheet(styleSheetCenterGroupedButton);
    layoutEndArrowStyle->addWidget(btnEndArrowStyleArrow);

    UBActionPaletteButton* btnEndArrowStyleRound = new UBActionPaletteButton(UBApplication::mainWindow->actionEndArrowStyleRound, this);
    btnEndArrowStyleRound->setStyleSheet(styleSheetRightGroupedButton);
    layoutEndArrowStyle->addWidget(btnEndArrowStyleRound);

    // Group buttons
    mButtonGroupStartArrowStyle = new QButtonGroup(this);
    mButtonGroupStartArrowStyle->addButton(btnStartArrowStyleNone);
    mButtonGroupStartArrowStyle->addButton(btnStartArrowStyleArrow);
    mButtonGroupStartArrowStyle->addButton(btnStartArrowStyleRound);

    mButtonGroupEndArrowStyle = new QButtonGroup(this);
    mButtonGroupEndArrowStyle->addButton(btnEndArrowStyleNone);
    mButtonGroupEndArrowStyle->addButton(btnEndArrowStyleArrow);
    mButtonGroupEndArrowStyle->addButton(btnEndArrowStyleRound);

    // Connect buttons
    connect(btnStartArrowStyleNone, &QAbstractButton::clicked, this, [this]() { onBtnStartArrowStyleNone(); });
    connect(btnStartArrowStyleArrow, &QAbstractButton::clicked, this, [this]() { onBtnStartArrowStyleArrow(); });
    connect(btnStartArrowStyleRound, &QAbstractButton::clicked, this, [this]() { onBtnStartArrowStyleRound(); });

    connect(btnEndArrowStyleNone, &QAbstractButton::clicked, this, [this]() { onBtnEndArrowStyleNone(); });
    connect(btnEndArrowStyleArrow, &QAbstractButton::clicked, this, [this]() { onBtnEndArrowStyleArrow(); });
    connect(btnEndArrowStyleRound, &QAbstractButton::clicked, this, [this]() { onBtnEndArrowStyleRound(); });

    adjustSizeAndPosition();
}

UBDrawingArrowsPropertiesPalette::~UBDrawingArrowsPropertiesPalette()
{
    delete mButtonGroupStartArrowStyle;
    mButtonGroupStartArrowStyle = nullptr;

    delete mButtonGroupEndArrowStyle;
    mButtonGroupEndArrowStyle = nullptr;
}

void UBDrawingArrowsPropertiesPalette::onBtnStartArrowStyleNone()
{
    UBApplication::boardController->shapeFactory().setStartArrowType(UBAbstractGraphicsPathItem::ArrowType_None);
}

void UBDrawingArrowsPropertiesPalette::onBtnStartArrowStyleArrow()
{
    UBApplication::boardController->shapeFactory().setStartArrowType(UBAbstractGraphicsPathItem::ArrowType_Arrow);
}

void UBDrawingArrowsPropertiesPalette::onBtnStartArrowStyleRound()
{
    UBApplication::boardController->shapeFactory().setStartArrowType(UBAbstractGraphicsPathItem::ArrowType_Round);
}

void UBDrawingArrowsPropertiesPalette::onBtnEndArrowStyleNone()
{
    UBApplication::boardController->shapeFactory().setEndArrowType(UBAbstractGraphicsPathItem::ArrowType_None);
}

void UBDrawingArrowsPropertiesPalette::onBtnEndArrowStyleArrow()
{
    UBApplication::boardController->shapeFactory().setEndArrowType(UBAbstractGraphicsPathItem::ArrowType_Arrow);
}

void UBDrawingArrowsPropertiesPalette::onBtnEndArrowStyleRound()
{
    UBApplication::boardController->shapeFactory().setEndArrowType(UBAbstractGraphicsPathItem::ArrowType_Round);
}
