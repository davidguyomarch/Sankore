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



#ifndef UBRESOURCES_H_
#define UBRESOURCES_H_

#include <QWidget>
#include <QApplication>
#include <QPainter>
#include <QColor>
#include <QPixmap>

class UBSettings;

class UBResources : public QObject
{
    Q_OBJECT;

    public:
         static UBResources* resources();
         QStringList customFontList() { return mCustomFontList; }


    private slots:
         // #439: rebuild theme-aware cursors when the UI theme changes so the
         // SVG tool cursors stay visible on both light and dark backgrounds,
         // then re-apply the active tool cursor.
         void updateThemedCursors();

    private:
    UBSettings* mSettings;
         UBResources(QObject* pParent = 0);
         virtual ~UBResources();

         void init();
         void buildThemedCursors();

         // #439: render an SVG resource recoloured to `glyph`, with a thin
         // contrasting `outline` halo so the cursor reads on any background.
         static QPixmap renderCursorSvg(const QString& svgResource, int size,
                                        const QColor& glyph, const QColor& outline);

         static UBResources* sSingleton;
         void buildFontList();
         QStringList mCustomFontList;

    public:

         QCursor penCursor;
         QCursor eraserCursor;
         QCursor markerCursor;
         QCursor pointerCursor;
         QCursor handCursor;
         QCursor zoomInCursor;
         QCursor zoomOutCursor;
         QCursor arrowCursor;
         QCursor playCursor;
         QCursor textCursor;
         QCursor richTextCursor;
         QCursor rotateCursor;
		 QCursor drawLineRulerCursor;
         QCursor ocrCursor;
         QCursor fillCursor;   // #429-followup: paint-bucket cursor for ChangeFill
};

#endif /* UBRESOURCES_H_ */
