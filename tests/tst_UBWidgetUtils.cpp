/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "tst_UBWidgetUtils.h"
#include "frameworks/UBWidgetUtils.h"

#include <QTreeWidget>
#include <QTreeWidgetItem>

void TestUBWidgetUtils::testNullTreeReturnsZero()
{
    QTreeWidget tree;
    QTreeWidgetItem item(&tree);
    QCOMPARE(UBWidgetUtils::getTreeWidgetItemVisualHeight(nullptr, &item), 0);
}

void TestUBWidgetUtils::testNullItemReturnsZero()
{
    QTreeWidget tree;
    QCOMPARE(UBWidgetUtils::getTreeWidgetItemVisualHeight(&tree, nullptr), 0);
}

void TestUBWidgetUtils::testCollapsedItemIgnoresChildren()
{
    QTreeWidget tree;
    tree.setColumnCount(1);

    auto* parent = new QTreeWidgetItem(&tree);
    parent->setText(0, QStringLiteral("parent"));
    auto* child = new QTreeWidgetItem(parent);
    child->setText(0, QStringLiteral("child"));

    parent->setExpanded(false);

    // Collapsed: only the parent's own visual height contributes (no child sum).
    int height = UBWidgetUtils::getTreeWidgetItemVisualHeight(&tree, parent);
    int parentOnly = tree.visualItemRect(parent).height();
    QCOMPARE(height, parentOnly);
}

void TestUBWidgetUtils::testExpandedItemSumsChildren()
{
    QTreeWidget tree;
    tree.setColumnCount(1);

    auto* parent = new QTreeWidgetItem(&tree);
    parent->setText(0, QStringLiteral("parent"));
    auto* child1 = new QTreeWidgetItem(parent);
    child1->setText(0, QStringLiteral("child1"));
    auto* child2 = new QTreeWidgetItem(parent);
    child2->setText(0, QStringLiteral("child2"));

    parent->setExpanded(true);

    int total = UBWidgetUtils::getTreeWidgetItemVisualHeight(&tree, parent);
    int expected = tree.visualItemRect(parent).height()
                 + tree.visualItemRect(child1).height()
                 + tree.visualItemRect(child2).height();

    // Expanded: parent's height plus both children's heights.
    QCOMPARE(total, expected);
}
