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

#ifndef UBPRESENTATIONCONTROLLER_H_
#define UBPRESENTATIONCONTROLLER_H_

#include <QObject>

/**
 * #399 (ADR-0008 D2) — single source of truth for the application's
 * *presentation* state (which top-level surface is on screen), as opposed to the
 * page/document *content*.
 *
 * BRICK 2 (this file) introduces the state in **shadow mode**: it is updated in
 * mirror of the existing transition code (showBoard/showDesktop/hideDesktop/
 * showInternet/showDocument), but nothing *consumes* `stateChanged` yet, so there
 * is no behaviour change. It exists to:
 *  - give a single, explicit vocabulary for the mode (replacing the scattered
 *    `mMainMode` + `mIsShowingDesktop` + `setInDesktopMode` flags), and
 *  - provide a `[STATE]` log that finally shows every real transition (including
 *    the redundant ones — e.g. the measured triple `hideWindow` on return).
 *
 * Later bricks will make the existing transitions route *through* this controller
 * (a single guarded `goToState()`), and make the palettes/overlay *subscribe* to
 * `stateChanged` instead of being pushed around.
 *
 * The state logic itself is Qt-UI-free and unit-testable (ADR-0008 D1 spirit).
 */
class UBPresentationController : public QObject
{
    Q_OBJECT

    public:
        enum class State
        {
            Board,              ///< The interactive whiteboard page.
            DesktopAnnotation,  ///< The see-through desktop annotation overlay.
            Documents,          ///< The documents browser.
            Web                 ///< The web browser.
        };
        Q_ENUM(State)

        explicit UBPresentationController(QObject* parent = nullptr);
        ~UBPresentationController() override = default;

        /// The presentation state currently on screen.
        State state() const { return mState; }

        /// Human-readable name, for logs/diagnostics.
        static const char* stateName(State s);

        /// Record a transition to \p next. In shadow mode this only updates the
        /// stored state, logs `[STATE] from -> to`, and emits stateChanged (which
        /// nobody consumes yet). A transition to the current state is a no-op
        /// (no log, no signal) — this is where the single guarded entry point of
        /// a later brick will also live.
        void setState(State next);

    signals:
        void stateChanged(UBPresentationController::State from,
                          UBPresentationController::State to);

    private:
        State mState = State::Board;
};

#endif /* UBPRESENTATIONCONTROLLER_H_ */
