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



#ifndef UBVIDEOPLAYER_H_
#define UBVIDEOPLAYER_H_


#include <QWidget>
#include <QApplication>
#include <QPushButton>
#include <QPainter>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoWidget>
#include <QSlider>

class UBVideoPlayer : public QWidget
{
    Q_OBJECT;

    public:
        UBVideoPlayer(QWidget* pParent = 0);
        virtual ~UBVideoPlayer();

        QMediaPlayer* mediaPlayer()
        {
            return mMediaPlayer;
        }

        void loadMedia(QUrl url)
        {
            mMediaPlayer->setSource(url);
        }

    protected:

        QMediaPlayer* mMediaPlayer;
        QAudioOutput* mAudioOutput;
        QVideoWidget* mVideoWidget;
        QPushButton *mPlayPause;
        QSlider *mSeekSlider;

    private slots:

        void mediaStateChanged(QMediaPlayer::PlaybackState newState);
        void tooglePlayPause();
        void durationChanged(qint64 duration);
        void positionChanged(qint64 position);
        void seekTo(int position);

};


#endif /* UBVIDEOPLAYER_H_ */
