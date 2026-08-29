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



#include "UBVideoPlayer.h"

#include <QWidget>
#include <QApplication>
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoWidget>


UBVideoPlayer::UBVideoPlayer(QWidget* pParent)
    : QWidget(pParent)
    , mMediaPlayer(nullptr)
    , mAudioOutput(nullptr)
    , mVideoWidget(nullptr)
{
    mMediaPlayer = new QMediaPlayer(this);
    mAudioOutput = new QAudioOutput(this);
    mVideoWidget = new QVideoWidget(this);

    mMediaPlayer->setAudioOutput(mAudioOutput);
    mMediaPlayer->setVideoOutput(mVideoWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);

    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(mVideoWidget);

    QHBoxLayout* transportLayout = new QHBoxLayout();
    mainLayout->addLayout(transportLayout);

    mPlayPause = new QPushButton(this);
    transportLayout->addWidget(mPlayPause);

    mSeekSlider = new QSlider(Qt::Horizontal, this);
    mSeekSlider->setRange(0, 0);
    transportLayout->addWidget(mSeekSlider);

    connect(mPlayPause, &QPushButton::clicked,
            this, [this]() { tooglePlayPause(); });

    connect(mMediaPlayer, &QMediaPlayer::playbackStateChanged,
            this, &UBVideoPlayer::mediaStateChanged);

    connect(mMediaPlayer, &QMediaPlayer::durationChanged,
            this, &UBVideoPlayer::durationChanged);

    connect(mMediaPlayer, &QMediaPlayer::positionChanged,
            this, &UBVideoPlayer::positionChanged);

    connect(mSeekSlider, &QSlider::sliderMoved,
            this, &UBVideoPlayer::seekTo);
}


UBVideoPlayer::~UBVideoPlayer()
{
    // NOOP
}


void UBVideoPlayer::tooglePlayPause()
{
    if(mMediaPlayer->playbackState() == QMediaPlayer::PlayingState)
    {
        mMediaPlayer->pause();
    }
    else
    {
         mMediaPlayer->play();
    }
}


void UBVideoPlayer::mediaStateChanged(QMediaPlayer::PlaybackState newState)
{
    if (newState == QMediaPlayer::PlayingState)
    {
        mPlayPause->setIcon(QPixmap(":/images/pause.svg"));
    }
    else
    {
        mPlayPause->setIcon(QPixmap(":/images/play.svg"));
    }
}

void UBVideoPlayer::durationChanged(qint64 duration)
{
    mSeekSlider->setRange(0, static_cast<int>(duration / 1000));
}

void UBVideoPlayer::positionChanged(qint64 position)
{
    mSeekSlider->setValue(static_cast<int>(position / 1000));
}

void UBVideoPlayer::seekTo(int position)
{
    mMediaPlayer->setPosition(static_cast<qint64>(position) * 1000);
}
