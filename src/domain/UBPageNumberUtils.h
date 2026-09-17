/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef UBPAGENUMBERUTILS_H
#define UBPAGENUMBERUTILS_H

/**
 * Pure helpers converting between a 0-based scene index (the model, as used by
 * UBBoardController::setActiveDocumentScene) and the 1-based page number shown
 * to the user (#357).
 *
 * Header-only, no widget deps — unit testable.
 *
 * Background (#357): the "Go to page" link action used the 0-based scene index
 * both as the combo-box label AND as the navigation target, so the picker
 * showed "0, 1, 2, ..." and jumping was off by one relative to what the user
 * read. Display must be 1-based; navigation must stay 0-based. Keep the two
 * concerns separated by converting explicitly here.
 */
namespace UBPageNumber
{
    /// 0-based scene index → 1-based page number for display.
    inline int sceneIndexToPageNumber(int sceneIndex)
    {
        return sceneIndex + 1;
    }

    /// 1-based page number (as shown to the user) → 0-based scene index.
    inline int pageNumberToSceneIndex(int pageNumber)
    {
        return pageNumber - 1;
    }
}

#endif // UBPAGENUMBERUTILS_H
