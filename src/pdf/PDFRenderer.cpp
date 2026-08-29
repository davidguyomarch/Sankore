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



#include <QFile>
#include <QScreen>
#include <QGuiApplication>

#include "PDFRenderer.h"

// XPDFRenderer not available - PDF rendering via web plugin disabled
// #include "XPDFRenderer.h"

#include "core/UBApplication.h"


QMap< QUuid, QPointer<PDFRenderer> > PDFRenderer::sRenderers;

PDFRenderer::PDFRenderer() : dpiForRendering(96)
{
}

PDFRenderer::~PDFRenderer()
{
    // NOOP
}

PDFRenderer* PDFRenderer::rendererForUuid(const QUuid &uuid, const QString &filename, bool importingFile)
{
    if (sRenderers.contains(uuid))
    {
        return sRenderers.value(uuid);
    }
    else
    {
        // XPDFRenderer not available - return null
        Q_UNUSED(filename);
        Q_UNUSED(importingFile);
        return nullptr;
    }
}

void PDFRenderer::setRefCount(const QAtomicInt &refCount)
{
    mRefCount = refCount;
}

void PDFRenderer::setFileData(const QByteArray &fileData)
{
    mFileData = fileData;
}

void PDFRenderer::setFileUuid(const QUuid &fileUuid)
{
    mFileUuid = fileUuid;
}

void PDFRenderer::attach()
{
    mRefCount.ref();
}

void PDFRenderer::detach()
{
    mRefCount.deref();
    if (mRefCount == 0)
    {
        sRenderers.remove(mFileUuid);
        delete this;
    }
}
