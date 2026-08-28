/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "UBSpellCorrector.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QVector>

UBSpellCorrector::UBSpellCorrector()
{
}

bool UBSpellCorrector::loadDictionary(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "UBSpellCorrector: cannot open dictionary" << path;
        return false;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    int count = 0;

    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed().toLower();
        if (!line.isEmpty() && !line.startsWith('#'))
        {
            mWords.insert(line);
            count++;
        }
    }

    file.close();
    qDebug() << "UBSpellCorrector: loaded" << count << "words from" << path
             << "(total:" << mWords.size() << ")";
    return true;
}

bool UBSpellCorrector::isKnown(const QString& word) const
{
    return mWords.contains(word.toLower());
}

int UBSpellCorrector::levenshteinDistance(const QString& a, const QString& b)
{
    int m = a.length();
    int n = b.length();

    if (m == 0) return n;
    if (n == 0) return m;

    // Use two rows for space efficiency
    QVector<int> prev(n + 1), curr(n + 1);

    for (int j = 0; j <= n; ++j)
        prev[j] = j;

    for (int i = 1; i <= m; ++i)
    {
        curr[0] = i;
        for (int j = 1; j <= n; ++j)
        {
            int cost = (a[i - 1].toLower() == b[j - 1].toLower()) ? 0 : 1;
            curr[j] = qMin(qMin(curr[j - 1] + 1,     // insertion
                               prev[j] + 1),          // deletion
                          prev[j - 1] + cost);        // substitution
        }
        prev = curr;
    }

    return prev[n];
}

QString UBSpellCorrector::correct(const QString& word, int maxDistance) const
{
    if (word.isEmpty())
        return word;

    // Already known — no correction needed
    if (isKnown(word))
        return word;

    // Short words (1-2 chars) — don't correct (too ambiguous)
    if (word.length() <= 2)
        return word;

    // Find the closest word in dictionary
    QString bestMatch;
    int bestDistance = maxDistance + 1;

    QString lowerWord = word.toLower();

    for (const QString& dictWord : mWords)
    {
        // Quick length filter — Levenshtein distance is at least |len difference|
        int lenDiff = qAbs(dictWord.length() - lowerWord.length());
        if (lenDiff > maxDistance)
            continue;

        int dist = levenshteinDistance(lowerWord, dictWord);
        if (dist < bestDistance)
        {
            bestDistance = dist;
            bestMatch = dictWord;
            if (dist == 1)
                break; // can't do better than 1 (0 would mean isKnown)
        }
    }

    if (bestDistance <= maxDistance && !bestMatch.isEmpty())
    {
        // Preserve original case style
        if (word == word.toUpper())
            return bestMatch.toUpper();
        if (word[0].isUpper())
            return bestMatch[0].toUpper() + bestMatch.mid(1);
        return bestMatch;
    }

    return word; // no correction found
}

QString UBSpellCorrector::correctSentence(const QString& text, int maxDistance) const
{
    QStringList words = text.split(' ', Qt::SkipEmptyParts);
    QStringList corrected;

    for (const QString& word : words)
    {
        // Strip punctuation from start/end for matching
        QString clean = word;
        QString prefix, suffix;

        while (!clean.isEmpty() && !clean[0].isLetterOrNumber())
        {
            prefix += clean[0];
            clean = clean.mid(1);
        }
        while (!clean.isEmpty() && !clean.back().isLetterOrNumber())
        {
            suffix.prepend(clean.back());
            clean.chop(1);
        }

        if (clean.isEmpty())
        {
            corrected.append(word);
        }
        else
        {
            corrected.append(prefix + correct(clean, maxDistance) + suffix);
        }
    }

    return corrected.join(' ');
}
