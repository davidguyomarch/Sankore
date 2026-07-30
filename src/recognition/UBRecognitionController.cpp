/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#include "UBRecognitionController.h"
#include "IHandwritingRecognizer.h"
#include "UBStrokeExtractor.h"

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsTextItem.h"

#include <QMessageBox>
#include <QGraphicsItem>

UBRecognitionController::UBRecognitionController(QObject* parent)
    : QObject(parent)
    , mRecognizer(IHandwritingRecognizer::createDefault())
{
}

UBRecognitionController::~UBRecognitionController()
{
    delete mRecognizer;
}

bool UBRecognitionController::isAvailable() const
{
    return mRecognizer && mRecognizer->isAvailable();
}

void UBRecognitionController::recognizeSelection()
{
    if (!mRecognizer || !mRecognizer->isAvailable())
    {
        UBApplication::showMessage(tr("Handwriting recognition is not available on this platform."));
        return;
    }

    UBGraphicsScene* scene = UBApplication::boardController->activeScene();
    if (!scene)
        return;

    QList<QGraphicsItem*> selection = scene->selectedItems();
    if (selection.isEmpty())
    {
        UBApplication::showMessage(tr("Please select strokes to recognize."));
        return;
    }

    // Extract strokes from selection
    QVector<UBRecognitionStroke> strokes = UBStrokeExtractor::extractFromSelection(selection);
    if (strokes.isEmpty())
    {
        UBApplication::showMessage(tr("No handwriting strokes found in selection."));
        return;
    }

    // Run recognition
    UBRecognitionResult result = mRecognizer->recognize(strokes);

    if (!result.success)
    {
        UBApplication::showMessage(tr("Recognition failed: %1").arg(result.errorMessage));
        return;
    }

    // Confirm with user
    QString message = tr("Recognized text: \"%1\"\n\nReplace selected strokes with this text?").arg(result.text);
    if (!result.candidates.isEmpty() && result.candidates.size() > 1)
    {
        message += "\n\n" + tr("Alternatives: ") + result.candidates.mid(1).join(", ");
    }

    QMessageBox::StandardButton reply = QMessageBox::question(nullptr,
        tr("Handwriting Recognition"), message,
        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    if (reply != QMessageBox::Yes)
        return;

    // Calculate bounding rect of selection for text placement
    QRectF selectionRect;
    for (QGraphicsItem* item : selection)
        selectionRect = selectionRect.united(item->sceneBoundingRect());

    // Remove selected items
    for (QGraphicsItem* item : selection)
    {
        scene->removeItem(item);
        delete item;
    }

    // Add text item at the center of where strokes were
    QPointF textPos(selectionRect.left(), selectionRect.top());
    UBGraphicsTextItem* textItem = scene->addTextWithFont(result.text, textPos, 24);
    if (textItem)
    {
        textItem->setSelected(true);
    }

    UBApplication::showMessage(tr("Text recognized: \"%1\"").arg(result.text));
}
