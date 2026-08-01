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
#include "domain/UBGraphicsPolygonItem.h"
#include "domain/UBGraphicsStrokesGroup.h"

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

void UBRecognitionController::recognizeZone(const QRectF& sceneRect)
{
    if (!mRecognizer || !mRecognizer->isAvailable())
    {
        UBApplication::showMessage(tr("Handwriting recognition is not available on this platform."));
        return;
    }

    UBGraphicsScene* scene = UBApplication::boardController->activeScene();
    if (!scene)
        return;

    // Find all items in the zone
    QList<QGraphicsItem*> itemsInZone = scene->items(sceneRect, Qt::IntersectsItemBoundingRect);
    if (itemsInZone.isEmpty())
    {
        UBApplication::showMessage(tr("No strokes found in the selected zone."));
        return;
    }

    // Debug: log what we found
    qDebug() << "OCR zone:" << sceneRect << "- found" << itemsInZone.size() << "items";
    for (QGraphicsItem* item : itemsInZone)
        qDebug() << "  item type:" << item->type() << "bounds:" << item->sceneBoundingRect();

    // Extract strokes from items in zone
    QVector<UBRecognitionStroke> strokes = UBStrokeExtractor::extractFromSelection(itemsInZone);

    // Debug: log extraction results
    qDebug() << "OCR extracted" << strokes.size() << "strokes";
    for (int i = 0; i < strokes.size(); i++)
        qDebug() << "  stroke" << i << ":" << strokes[i].points.size() << "points";

    if (strokes.isEmpty())
    {
        UBApplication::showMessage(tr("No handwriting strokes found in the selected zone."));
        return;
    }

    // Show diagnostic in message
    QString diagMsg = QString("Found %1 items, extracted %2 strokes").arg(itemsInZone.size()).arg(strokes.size());
    for (int i = 0; i < strokes.size() && i < 3; i++)
        diagMsg += QString(", s%1=%2pts").arg(i).arg(strokes[i].points.size());

    // Run recognition
    UBRecognitionResult result = mRecognizer->recognize(strokes);

    if (!result.success)
    {
        UBApplication::showMessage(tr("Recognition failed: %1\n\nDiag: %2").arg(result.errorMessage).arg(diagMsg));
        UBApplication::showMessage(tr("Recognition failed: %1").arg(result.errorMessage));
        return;
    }

    // Confirm with user
    QString message = tr("Recognized text: \"%1\"\n\nReplace strokes with this text?").arg(result.text);
    QMessageBox::StandardButton reply = QMessageBox::question(nullptr,
        tr("Handwriting Recognition"), message,
        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    if (reply != QMessageBox::Yes)
        return;

    // Remove stroke items in zone
    for (QGraphicsItem* item : itemsInZone)
    {
        if (dynamic_cast<UBGraphicsStrokesGroup*>(item) ||
            dynamic_cast<UBGraphicsPolygonItem*>(item))
        {
            scene->removeItem(item);
            delete item;
        }
    }

    // Add text at zone position
    QPointF textPos(sceneRect.left(), sceneRect.top());
    UBGraphicsTextItem* textItem = scene->addTextWithFont(result.text, textPos, 24);
    if (textItem)
        textItem->setSelected(true);

    UBApplication::showMessage(tr("Text recognized: \"%1\"").arg(result.text));
}
