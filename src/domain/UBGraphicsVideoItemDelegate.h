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



#ifndef UBGRAPHICSVIDEOITEMDELEGATE_H_
#define UBGRAPHICSVIDEOITEMDELEGATE_H_

#include <QWidget>
#include <QApplication>
#include <QPainter>
#include <QMediaPlayer>

#include "core/UB.h"
#include "UBGraphicsItemDelegate.h"

class QGraphicsSceneMouseEvent;
class QGraphicsItem;
class UBGraphicsVideoItem;

class DelegateVideoControl: public QGraphicsRectItem
{
    public:

        DelegateVideoControl(UBGraphicsVideoItem* pDelegated, QGraphicsItem * parent = 0);

        virtual ~DelegateVideoControl()
        {
            // NOOP
        }

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                QWidget *widget);

        QPainterPath shape() const;

        void setAntiScale(qreal antiScale){ mAntiScale = antiScale; }

        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

        void updateTicker(qint64 time);
        void totalTimeChanged( qint64 newTotalTime);

   protected:


        void seekToMousePos(QPointF mousePos);

        UBGraphicsVideoItem* mDelegate;
        bool mDisplayCurrentTime;

        qreal mAntiScale;
        qint64 mCurrentTimeInMs;
        qint64 mTotalTimeInMs;

    private:
        int mStartWidth;
        QRectF mBalloon;
};


class UBGraphicsVideoItemDelegate : public UBGraphicsItemDelegate
{
    Q_OBJECT

    public:
        UBGraphicsVideoItemDelegate(UBGraphicsVideoItem* pDelegated, QMediaPlayer* pMedia, QObject * parent = 0);
        virtual ~UBGraphicsVideoItemDelegate();

        virtual void positionHandles();

    public slots:

        void toggleMute();
        void updateTicker(qint64 time);

    protected slots:

        virtual void remove(bool canUndo = true);

        void togglePlayPause();

        void mediaStateChanged ( QMediaPlayer::PlaybackState newstate, QMediaPlayer::PlaybackState oldstate );

        void updatePlayPauseState();

        void totalTimeChanged( qint64 newTotalTime);

    protected:

        virtual void buildButtons();

    private:

        UBGraphicsVideoItem* delegated();

        DelegateButton* mPlayPauseButton;
        DelegateButton* mStopButton;
        DelegateButton* mMuteButton;
        DelegateVideoControl *mVideoControl;

        QMediaPlayer* mMedia;
};


#endif /* UBGRAPHICSVIDEOITEMDELEGATE_H_ */
