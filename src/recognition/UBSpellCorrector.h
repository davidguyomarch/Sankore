/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#ifndef UBSPELLCORRECTOR_H
#define UBSPELLCORRECTOR_H

#include <QString>
#include <QStringList>
#include <QSet>

/**
 * @brief Post-recognition spell corrector using Levenshtein distance.
 *
 * Loads a word list (one word per line) and provides correction by
 * finding the closest dictionary word within a maximum edit distance.
 *
 * Usage:
 *   UBSpellCorrector corrector;
 *   corrector.loadDictionary(":/dictionaries/en.txt");
 *   QString corrected = corrector.correct("HELO"); // → "HELLO"
 */
class UBSpellCorrector
{
public:
    UBSpellCorrector();

    /// Load a dictionary file (one word per line, UTF-8).
    /// Can be called multiple times to merge dictionaries.
    bool loadDictionary(const QString& path);

    /// Returns the number of words in the dictionary.
    int wordCount() const { return mWords.size(); }

    /// Check if a word is in the dictionary (case-insensitive).
    bool isKnown(const QString& word) const;

    /// Find the best correction for a word. Returns the word unchanged
    /// if it's already known or no close match is found.
    /// maxDistance: maximum Levenshtein distance to accept (default 2).
    QString correct(const QString& word, int maxDistance = 2) const;

    /// Correct all words in a sentence (split by spaces).
    QString correctSentence(const QString& text, int maxDistance = 2) const;

    /// Compute Levenshtein edit distance between two strings.
    static int levenshteinDistance(const QString& a, const QString& b);

private:
    QSet<QString> mWords; // lowercase dictionary
};

#endif // UBSPELLCORRECTOR_H
