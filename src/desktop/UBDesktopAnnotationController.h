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


#ifndef UBUNINOTESWINDOWCONTROLLER_H_
#define UBUNINOTESWINDOWCONTROLLER_H_

#include <QWidget>
#include <QApplication>
#include <QPainter>

class UBSettings;

class UBBoardView;
class UBGraphicsScene;
class QQuickWidget;

/**
 * The uninotes controller. This object allocates a uninotes window and implements all the actions corresponding to
 * the desktop toolbar (DesktopToolbar.qml, issue #336):
 * - Go to Uninotes (return to board)
 * - Custom capture (part of the screen)
 * - Window capture
 * - Screen capture
 *
 * Since #336 the toolbar is the V2 QML DesktopToolbar hosted in a QQuickWidget
 * parented to mTransparentDrawingView. The legacy UBDesktopPalette (QAction /
 * UBFloatingPalette based, with pen/marker/eraser hold-timer property popups)
 * has been removed; tool selection now binds directly to UBToolController and
 * color/width is handled by the shared DrawingPropsBar.
 */
class UBDesktopAnnotationController : public QObject
{
    Q_OBJECT;

    public:
        UBDesktopAnnotationController(QObject *parent);
        virtual ~UBDesktopAnnotationController();
        void showWindow();
        void hideWindow();

        // Rounded region covering the QML toolbar, so UBBoardView lets clicks
        // on the toolbar pass through instead of drawing on the overlay.
        QPainterPath desktopPalettePath() const;
        UBBoardView *drawingView();

        void TransparentWidgetResized();


    public slots:

        void screenLayoutChanged();
        // Invokable from DesktopToolbar.qml via the desktopController context object.
        void goToUniboard();
        void customCapture();
        void windowCapture();
        void screenCapture();
        void updateShowHideState(bool pEnabled);

        void close();

        void stylusToolChanged(int tool);
        void updateBackground();

    signals:
        /**
         * This signal is emitted once the screenshot has been performed. This signal is also emitted when user
         * click on go to uniboard button. In this case pCapturedPixmap is an empty pixmap.
         * @param pCapturedPixmap QPixmap corresponding to the capture.
         */
        void imageCaptured(const QPixmap& pCapturedPixmap, bool pageMode);
        void restoreUniboard();

    protected:
        QPixmap getScreenPixmap();

        UBBoardView* mTransparentDrawingView;
        UBGraphicsScene* mTransparentDrawingScene;

    private slots:
        void onTransparentWidgetResized();
        void refreshMask();

    private:
        void updateMask(bool bTransparent);
        void setupToolbar();
        void positionToolbar();
        void showToolbar();
        void hideToolbarForCapture();
        void restoreToolbarAfterCapture();

        UBSettings* mSettings;

        // V2 QML desktop toolbar (DesktopToolbar.qml), parented to mTransparentDrawingView.
        QQuickWidget* mToolbarQml;

        bool mIsFullyTransparent;

        int mBoardStylusTool;
        int mDesktopStylusTool;

        QPixmap mMask;

};

#endif /* UBUNINOTESWINDOWCONTROLLER_H_ */
