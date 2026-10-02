/*
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "UBPresentationController.h"

#include <QCoreApplication>
#include <QFile>
#include <QTextStream>

UBPresentationController::UBPresentationController(QObject* parent)
    : QObject(parent)
{
}

const char* UBPresentationController::stateName(State s)
{
    switch (s)
    {
    case State::Board:             return "Board";
    case State::DesktopAnnotation: return "DesktopAnnotation";
    case State::Documents:         return "Documents";
    case State::Web:               return "Web";
    }
    return "?";
}

void UBPresentationController::setState(State next)
{
    if (next == mState)
        return;  // no-op on same state (the future single guarded entry point)

    const State previous = mState;
    mState = next;

    // #399 BRICK 2 shadow diagnostic: trace every real presentation transition.
    // This is what finally shows the redundant transitions on the VM (e.g. the
    // measured triple hideWindow on return). Later bricks make this the single
    // place transitions happen; the log can stay as a structural diagnostic.
    {
        QFile f(QCoreApplication::applicationDirPath() + "/startup.log");
        if (f.open(QIODevice::Append | QIODevice::Text))
            QTextStream(&f) << "[STATE] " << stateName(previous)
                            << " -> " << stateName(next) << "\n";
    }

    emit stateChanged(previous, next);
}
