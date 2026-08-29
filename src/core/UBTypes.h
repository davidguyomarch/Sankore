/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Shared enums and types used across multiple modules.
 * This file should have NO dependencies on other project headers
 * to avoid circular includes.
 */

#ifndef UBTYPES_H_
#define UBTYPES_H_

/**
 * Background image disposition for document pages.
 * Used by domain/ (UBGraphicsScene) and board/ (UBFeaturesController).
 */
enum UBFeatureBackgroundDisposition
{
    Center,
    Adjust,
    Mosaic,
    Fill,
    Extend
};

#endif // UBTYPES_H_
