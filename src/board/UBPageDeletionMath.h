/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef UBPAGEDELETIONMATH_H
#define UBPAGEDELETIONMATH_H

/**
 * Compute the active-scene index to switch to after deleting a page.
 *
 * #276: the previous code used `deletedIndex - 1`, which yields -1 when the
 * first page (index 0) is deleted. That negative index was not clamped by
 * setActiveDocumentScene, leaving the active scene dangling and crashing when
 * QML bindings were re-evaluated (especially with shapes/strokes on the page).
 *
 * Convention: stay at the same slot — which, after deletion, shows the page
 * that followed the deleted one — and if the deleted page was the last one,
 * move to the new last page. The result is always clamped to a valid index
 * in [0, pageCountBefore - 2] (the range of the document after one removal).
 *
 * Pure integer math, header-only, no dependencies — trivially unit-testable.
 *
 * @param deletedIndex     index of the page being deleted (0-based)
 * @param pageCountBefore  number of pages before the deletion (expected >= 2)
 * @return valid active-scene index after deletion, never negative
 */
inline int ubActiveIndexAfterPageDeletion(int deletedIndex, int pageCountBefore)
{
    const int newCount = pageCountBefore - 1;
    if (newCount <= 0)
        return 0;

    int index = deletedIndex;
    if (index > newCount - 1)
        index = newCount - 1;   // deleted the last page -> new last
    if (index < 0)
        index = 0;              // deleted the first page -> stay at 0 (was the -1 bug)
    return index;
}

#endif // UBPAGEDELETIONMATH_H
