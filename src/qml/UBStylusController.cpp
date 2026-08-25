/*
 * Copyright (C) 2024 Open-Sankoré contributors
 * License: GPLv3
 */

#include "UBStylusController.h"

#include <QAction>
#include <QActionGroup>
#include <QVariantMap>

UBStylusController::UBStylusController(QObject* parent)
    : QObject(parent)
{
}

void UBStylusController::addTool(const QString& name, const QString& iconSource, QAction* action, bool isToggle)
{
    ToolEntry entry;
    entry.name = name;
    entry.iconSource = iconSource;
    entry.action = action;
    entry.isToggle = isToggle;
    mEntries.append(entry);
}

void UBStylusController::finalize()
{
    mTools.clear();

    // Create an exclusive action group for non-toggle tool actions
    QActionGroup* exclusiveGroup = new QActionGroup(this);
    exclusiveGroup->setExclusive(true);

    for (int i = 0; i < mEntries.size(); ++i)
    {
        const ToolEntry& e = mEntries[i];

        QVariantMap tool;
        tool["name"] = e.name;
        tool["iconSource"] = e.iconSource;
        tool["isToggle"] = e.isToggle;
        mTools.append(tool);

        if (e.action)
        {
            // Non-toggle actions participate in exclusive group
            if (!e.isToggle)
                exclusiveGroup->addAction(e.action);

            // Listen for external changes (keyboard shortcuts, other UI elements)
            connect(e.action, &QAction::toggled, this, &UBStylusController::onActionToggled);
            connect(e.action, &QAction::changed, this, [this]() { updateActiveFromActions(); });
        }
    }

    updateActiveFromActions();
}

void UBStylusController::selectTool(int index)
{
    if (index < 0 || index >= mEntries.size())
        return;

    const ToolEntry& entry = mEntries[index];
    if (!entry.action)
        return;

    if (entry.isToggle)
    {
        // Toggle buttons just toggle their state
        entry.action->toggle();
        return;
    }

    // Trigger the action (this goes through UBDrawingController)
    entry.action->trigger();

    // The action's toggled signal will update mActiveIndex via onActionToggled
}

void UBStylusController::onActionToggled(bool checked)
{
    if (checked)
        updateActiveFromActions();
}

void UBStylusController::updateActiveFromActions()
{
    // Find the first non-toggle action that is checked
    for (int i = 0; i < mEntries.size(); ++i)
    {
        const ToolEntry& e = mEntries[i];
        if (e.isToggle)
            continue;
        if (e.action && e.action->isChecked())
        {
            if (mActiveIndex != i)
            {
                mActiveIndex = i;
                emit activeToolChanged();
            }
            return;
        }
    }
}

void UBStylusController::setVertical(bool v)
{
    if (mVertical != v)
    {
        mVertical = v;
        emit orientationChanged();
    }
}
