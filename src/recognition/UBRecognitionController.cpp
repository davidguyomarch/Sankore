/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#include "UBRecognitionController.h"
#include "IHandwritingRecognizer.h"
#include "UBStrokeExtractor.h"

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsTextItem.h"
#include "domain/UBGraphicsPolygonItem.h"
#include "domain/UBGraphicsStrokesGroup.h"

#include <QMessageBox>
#include <QGraphicsItem>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>

UBRecognitionController::UBRecognitionController(QObject* parent)
    : QObject(parent)
    , mRecognizer(IHandwritingRecognizer::createDefault())
    , mAutoMode(false)
    , mAutoTimer(new QTimer(this))
{
    mAutoTimer->setSingleShot(true);
    mAutoTimer->setInterval(2000); // 2 seconds pause before auto-recognition
    connect(mAutoTimer, &QTimer::timeout, this, &UBRecognitionController::onAutoTimerExpired);

    // Log diagnostic info about available recognizers at startup
    if (mRecognizer)
    {
        QString diag = mRecognizer->diagnosticInfo();
        if (!diag.isEmpty())
            qDebug().noquote() << "OCR diagnostic:\n" << diag;

        if (!mRecognizer->isAvailable())
        {
            qWarning() << "OCR: Handwriting recognition is NOT available."
                       << "Install a handwriting recognition pack in Windows Settings.";
        }
    }
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

    // Force release mouse/tablet state — the modal dialog eats the release event
    UBApplication::boardController->controlView()->forcedTabletRelease();

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
        QString msg = tr("Handwriting recognition is not available on this platform.");
        QString diag = mRecognizer ? mRecognizer->diagnosticInfo() : "";
        if (!diag.isEmpty())
            msg += "\n\n" + diag;
        UBApplication::showMessage(msg);
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

    // Dump strokes to file for debugging
    {
        QString dumpPath = QCoreApplication::applicationDirPath() + "/ocr_strokes_dump.txt";
        QFile dumpFile(dumpPath);
        if (dumpFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&dumpFile);
            out << "STROKES " << strokes.size() << "\n";
            for (int i = 0; i < strokes.size(); i++)
            {
                out << "STROKE " << i << " POINTS " << strokes[i].points.size() << "\n";
                for (const QPointF& p : strokes[i].points)
                    out << p.x() << " " << p.y() << "\n";
            }
            dumpFile.close();
        }
    }

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

    // Force release mouse/tablet state — the modal dialog eats the release event
    UBApplication::boardController->controlView()->forcedTabletRelease();

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

void UBRecognitionController::setAutoMode(bool enabled)
{
    mAutoMode = enabled;
    if (!mAutoMode)
        mAutoTimer->stop();

    if (enabled)
        UBApplication::showMessage(tr("Auto-recognition ON — write and pause to recognize"));
    else
        UBApplication::showMessage(tr("Auto-recognition OFF"));
}

void UBRecognitionController::onStrokeFinished()
{
    if (!mAutoMode)
        return;

    // Reset timer — recognition happens after 2s of no new strokes
    mAutoTimer->start();
}

void UBRecognitionController::onAutoTimerExpired()
{
    if (!mAutoMode || !mRecognizer || !mRecognizer->isAvailable())
        return;

    UBGraphicsScene* scene = UBApplication::boardController->activeScene();
    if (!scene)
        return;

    // Collect recently added stroke items (those added in the last few seconds)
    // Strategy: find all UBGraphicsStrokesGroup items and recognize the most recent ones
    QList<QGraphicsItem*> allItems = scene->items();

    // Get strokes that were drawn recently — use the last N strokes groups
    QList<QGraphicsItem*> recentStrokes;
    int count = 0;
    for (QGraphicsItem* item : allItems)
    {
        if (dynamic_cast<UBGraphicsStrokesGroup*>(item))
        {
            recentStrokes.append(item);
            count++;
            if (count >= 20) // max 20 strokes back
                break;
        }
    }

    if (recentStrokes.isEmpty())
        return;

    // Calculate bounding rect of recent strokes
    QRectF recentRect;
    for (QGraphicsItem* item : recentStrokes)
        recentRect = recentRect.united(item->sceneBoundingRect());

    // Run recognition on this zone
    recognizeZone(recentRect);
}
