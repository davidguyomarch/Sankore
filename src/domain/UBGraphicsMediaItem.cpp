/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 * Copyright (C) 2026 David Guyomarch
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */



#include "UBGraphicsGroupContainerItem.h"
#include <QGraphicsSceneMouseEvent>
#include <QFileInfo>
#include <QMimeDatabase>
#include "UBGraphicsMediaItem.h"
#include "UBGraphicsMediaItemDelegate.h"
#include "UBGraphicsScene.h"
#include "UBGraphicsDelegateFrame.h"
#include "document/UBDocumentProxy.h"
#include "core/UBApplication.h"
#include "board/UBBoardController.h"


UBAudioPresentationWidget::UBAudioPresentationWidget(QWidget *parent)
    : QWidget(parent)
    , mBorderSize(10)
{

}

void UBAudioPresentationWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(rect(), QBrush(Qt::white));

    QPen borderPen;
    borderPen.setWidth(2);
    borderPen.setColor(QColor(Qt::black));

    painter.setPen(borderPen);
    painter.drawRect(0,0, width(), height());

    if (QString() != mTitle)
    {
        painter.setPen(QPen(Qt::black));
        QRect titleRect = rect();
        titleRect.setX(mBorderSize);
        titleRect.setY(2);
        titleRect.setHeight(15);
        painter.drawText(titleRect, mTitle);
    }

    QWidget::paintEvent(event);
}

bool UBGraphicsMediaItem::sIsMutedByDefault = false;

UBGraphicsMediaItem::UBGraphicsMediaItem(const QUrl& pMediaFileUrl, QGraphicsItem *parent)
        : UBAbstractGraphicsProxyWidget(parent)
        , mVideoWidget(nullptr)
        , mAudioOutput(nullptr)
        , mAudioWidget(nullptr)
        , mMuted(sIsMutedByDefault)
        , mMutedByUserAction(sIsMutedByDefault)
        , mMediaFileUrl(pMediaFileUrl)
        , mLinkedImage(nullptr)
        , mInitialPos(0)
{
    update();

    mMediaObject = new QMediaPlayer(this);
    mAudioOutput = new QAudioOutput(this);

    mMediaType = detectMediaType(pMediaFileUrl);

    if (mMediaType == mediaType_Video)
    {
        mVideoWidget = new QVideoWidget(); // owned and destructed by the scene ...
        mMediaObject->setVideoOutput(mVideoWidget);

        if(mVideoWidget->sizeHint() == QSize(1,1)){
            mVideoWidget->resize(320,240);
        }

        mVideoWidget->setMinimumSize(140,26);

        haveLinkedImage = true;
    }
    else
    {
        // Default to audio (covers both explicit audio files and unknown types)
        mMediaType = mediaType_Audio;

        mAudioWidget = new UBAudioPresentationWidget();
        int borderSize = 0;
        UBAudioPresentationWidget* pAudioWidget = dynamic_cast<UBAudioPresentationWidget*>(mAudioWidget);
        if (pAudioWidget)
        {
            borderSize = pAudioWidget->borderSize();
        }

        mAudioWidget->resize(320,26+2*borderSize); //3*border size with enabled title
        mAudioWidget->setMinimumSize(150,26+borderSize);

        haveLinkedImage = false;
    }

    mMediaObject->setAudioOutput(mAudioOutput);

    mSource = QUrl(pMediaFileUrl);
    mMediaObject->setSource(mSource);

    // we should create delegate after media objects because delegate uses his properties at creation.
    setDelegate(new UBGraphicsMediaItemDelegate(this, mMediaObject));

    // delegate should be created earler because we setWidget calls resize event for graphics proxy widgt.
    // resize uses delegate.
    if (mediaType_Video == mMediaType)
        setWidget(mVideoWidget);
    else
        setWidget(mAudioWidget);

    // media widget should be created and placed on proxy widget here.
    Delegate()->init();

    if (mediaType_Audio == mMediaType)
        Delegate()->frame()->setOperationMode(UBGraphicsDelegateFrame::ResizingHorizontally);
    else
        Delegate()->frame()->setOperationMode(UBGraphicsDelegateFrame::Resizing);

    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::ObjectItem)); //Necessary to set if we want z value to be assigned correctly

    connect(Delegate(), &UBGraphicsItemDelegate::showOnDisplayChanged, this, &UBGraphicsMediaItem::showOnDisplayChanged);
    connect(mMediaObject, &QMediaPlayer::hasVideoChanged, this, &UBGraphicsMediaItem::hasMediaChanged);
}


UBGraphicsMediaItem::~UBGraphicsMediaItem()
{
    if (mMediaObject)
        mMediaObject->stop();
}


QVariant UBGraphicsMediaItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if ((change == QGraphicsItem::ItemEnabledChange)
            || (change == QGraphicsItem::ItemSceneChange)
            || (change == QGraphicsItem::ItemVisibleChange))
    {
        if (mMediaObject && (!isEnabled() || !isVisible() || !scene()))
        {
            mMediaObject->pause();
        }
    }
    else if (change == QGraphicsItem::ItemSceneHasChanged)
    {
        if (!scene())
        {
            mMediaObject->stop();
        }
        else
        {
            QString absoluteMediaFilename;

            QString localFile = mMediaFileUrl.toLocalFile();
            if(localFile.startsWith("audios/") || localFile.startsWith("videos/")
               || localFile.startsWith("audios\\") || localFile.startsWith("videos\\")){
                absoluteMediaFilename = scene()->document()->persistencePath() + "/"  + localFile;
            }
            else if(!QFileInfo(localFile).isAbsolute()){
                // Relative path without audios/videos prefix — try resolving against document
                QString subdir = isAudioUrl(mMediaFileUrl) ? "audios" : "videos";
                QString candidate = scene()->document()->persistencePath() + "/" + subdir + "/" + QFileInfo(localFile).fileName();
                if (QFile::exists(candidate))
                    absoluteMediaFilename = candidate;
                else
                    absoluteMediaFilename = scene()->document()->persistencePath() + "/" + localFile;
            }
            else{
                absoluteMediaFilename = localFile;
            }

            if (absoluteMediaFilename.length() > 0)
                mMediaObject->setSource(QUrl::fromLocalFile(absoluteMediaFilename));

        }
    }

    return UBAbstractGraphicsProxyWidget::itemChange(change, value);
}


void UBGraphicsMediaItem::setSourceUrl(const QUrl &pSourceUrl)
{
    UBAudioPresentationWidget* pAudioWidget = dynamic_cast<UBAudioPresentationWidget*>(mAudioWidget);
    if (pAudioWidget)
    {
       // pAudioWidget->setTitle(UBFileSystemUtils::lastPathComponent(pSourceUrl.toString()));
    }

    UBItem::setSourceUrl(pSourceUrl);
}

void UBGraphicsMediaItem::clearSource()
{
    QString path = mediaFileUrl().toLocalFile();
    //if path is absolute clean duplicated path string
    if (!path.contains(UBApplication::boardController->selectedDocument()->persistencePath()))
        path = UBApplication::boardController->selectedDocument()->persistencePath() + "/" + path;

    if (!UBFileSystemUtils::deleteFile(path))
        qDebug() << "cannot delete file: " << path;
}

void UBGraphicsMediaItem::toggleMute()
{
    mMuted = !mMuted;
    setMute(mMuted);
}

void UBGraphicsMediaItem::setMute(bool bMute)
{
    mMuted = bMute;
    mAudioOutput->setMuted(mMuted);
    mMutedByUserAction = mMuted;
    sIsMutedByDefault = mMuted;
}


void UBGraphicsMediaItem::hasMediaChanged(bool hasMedia)
{
    if(hasMedia && mMediaObject->isSeekable())
    {
    Q_UNUSED(hasMedia);
    mMediaObject->setPosition(mInitialPos);
        UBGraphicsMediaItemDelegate *med = dynamic_cast<UBGraphicsMediaItemDelegate *>(Delegate());
        if (med)
            med->updateTicker(initialPos());
    }
}


UBGraphicsScene* UBGraphicsMediaItem::scene()
{
    return qobject_cast<UBGraphicsScene*>(QGraphicsItem::scene());
}


void UBGraphicsMediaItem::activeSceneChanged()
{
    if (UBApplication::boardController->activeScene() != scene())
    {
        mMediaObject->pause();
    }
}


void UBGraphicsMediaItem::showOnDisplayChanged(bool shown)
{
    if (!shown)
    {
        mMuted = true;
        mAudioOutput->setMuted(mMuted);
    }
    else if (!mMutedByUserAction)
    {
        mMuted = false;
        mAudioOutput->setMuted(mMuted);
    }
}

UBItem* UBGraphicsMediaItem::deepCopy() const
{
    QUrl url = this->mediaFileUrl();
    UBGraphicsMediaItem *copy = new UBGraphicsMediaItem(url, parentItem());

    copy->setUuid(QUuid::createUuid()); // this is OK for now as long as Widgets are imutable

    copyItemParameters(copy);

    return copy;
}

void UBGraphicsMediaItem::copyItemParameters(UBItem *copy) const
{
    UBGraphicsMediaItem *cp = dynamic_cast<UBGraphicsMediaItem*>(copy);
    if (cp)
    {
        cp->setPos(this->pos());
        cp->setTransform(this->transform());
        cp->setFlag(QGraphicsItem::ItemIsMovable, true);
        cp->setFlag(QGraphicsItem::ItemIsSelectable, true);
        cp->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));
        cp->setData(UBGraphicsItemData::ItemLocked, this->data(UBGraphicsItemData::ItemLocked));
        cp->setSourceUrl(this->sourceUrl());
        cp->resize(this->size());

        connect(UBApplication::boardController, &UBBoardController::activeSceneChanged, cp, &UBGraphicsMediaItem::activeSceneChanged);
        // TODO UB 4.7 complete all members
    }
}

void UBGraphicsMediaItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (Delegate())
    {
        Delegate()->mousePressEvent(event);
        if (parentItem() && UBGraphicsGroupContainerItem::Type == parentItem()->type())
        {
            UBGraphicsGroupContainerItem *group = qgraphicsitem_cast<UBGraphicsGroupContainerItem*>(parentItem());
            if (group)
            {
                QGraphicsItem *curItem = group->getCurrentItem();
                if (curItem && this != curItem)
                {
                    group->deselectCurrentItem();
                }
                group->setCurrentItem(this);
                this->setSelected(true);
                Delegate()->positionHandles();
            }

        }
    }

    if (parentItem() && parentItem()->type() == UBGraphicsGroupContainerItem::Type)
    {
        mShouldMove = false;
        if (!Delegate()->mousePressEvent(event))
        {
            event->accept();
        }
    }
    else
    {
        mShouldMove = (event->buttons() & Qt::LeftButton);
        mMousePressPos = event->scenePos();
        mMouseMovePos = mMousePressPos;

        event->accept();
        setSelected(true);
    }

}

void UBGraphicsMediaItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(mShouldMove && (event->buttons() & Qt::LeftButton))
    {
        QPointF offset = event->scenePos() - mMousePressPos;

        if (offset.toPoint().manhattanLength() > QApplication::startDragDistance())
        {
            QPointF mouseMovePos = mapFromScene(mMouseMovePos);
            QPointF eventPos = mapFromScene( event->scenePos());

            QPointF translation = eventPos - mouseMovePos;
            moveBy(translation.x(), translation.y());
        }

        mMouseMovePos = event->scenePos();
    }

    event->accept();

}


UBGraphicsMediaItem::mediaType UBGraphicsMediaItem::detectMediaType(const QUrl &url)
{
    // First try: use QMimeDatabase for reliable detection
    QString filePath = url.toLocalFile();
    if (filePath.isEmpty())
        filePath = url.toString();

    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(filePath, QMimeDatabase::MatchExtension);
    QString mimeName = mime.name();

    if (mimeName.startsWith("video/"))
        return mediaType_Video;
    if (mimeName.startsWith("audio/"))
        return mediaType_Audio;

    // Fallback: check well-known extensions
    QString suffix = QFileInfo(filePath).suffix().toLower();
    static const QStringList videoExts = {"mp4", "avi", "mkv", "webm", "mov", "wmv", "flv", "ogv", "m4v"};
    static const QStringList audioExts = {"mp3", "wav", "ogg", "flac", "aac", "wma", "m4a", "opus"};

    if (videoExts.contains(suffix))
        return mediaType_Video;

    // Legacy fallback: check if path contains "videos" directory name
    if (filePath.toLower().contains("/videos/") || filePath.toLower().contains("\\videos\\"))
        return mediaType_Video;

    // Default to audio for all other cases (including unknown)
    return mediaType_Audio;
}

bool UBGraphicsMediaItem::isAudioUrl(const QUrl &url)
{
    return detectMediaType(url) == mediaType_Audio;
}
