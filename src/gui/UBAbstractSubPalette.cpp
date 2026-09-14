#include "UBAbstractSubPalette.h"
#include "qml/UBThemeManager.h"

// #297: grouped-button stylesheets built from the theme (were fixed light-grey
// gradients: #d3d3d3/#c4c4c4 normal, #c3c3c3/#b4b4b4 checked, #444444 border).
// Normal = surfaceVariant->surfaceHover gradient; checked = surfaceHover->surface
// (a touch different so the pressed state reads); border = theme border.
namespace
{
    QString groupedButtonQss(const QString& sideRules)
    {
        auto* tm = UBThemeManager::instance();
        const QString c1 = UBThemeManager::css(tm->surfaceVariant());
        const QString c2 = UBThemeManager::css(tm->surfaceHover());
        const QString k1 = UBThemeManager::css(tm->surfaceHover());
        const QString k2 = UBThemeManager::css(tm->surface());
        const QString brd = UBThemeManager::css(tm->border());
        return QString(
            "QToolButton{background: qlineargradient(x1: 0, y1: 0.49, x2: 0, y2: 0.5, "
            "stop: 0 %1, stop: 1 %2); margin-top: 1px; border: 1px solid %5; "
            "height: 24px; %6}"
            "QToolButton:checked{background: qlineargradient(x1: 0, y1: 0.49, x2: 0, y2: 0.5, "
            "stop: 0 %3, stop: 1 %4); border: 1px solid %5;}")
            .arg(c1, c2, k1, k2, brd, sideRules);
    }
}

QString UBAbstractSubPalette::styleSheetLeftGroupedButton()
{
    return groupedButtonQss(
        "border-right: 1px solid transparent; "
        "border-top-left-radius: 3px; border-bottom-left-radius: 3px;");
}

QString UBAbstractSubPalette::styleSheetCenterGroupedButton()
{
    return groupedButtonQss(
        "border-right: 1px solid transparent; border-left: 1px solid transparent;");
}

QString UBAbstractSubPalette::styleSheetRightGroupedButton()
{
    return groupedButtonQss(
        "border-left: 1px solid transparent; "
        "border-top-right-radius: 3px; border-bottom-right-radius: 3px;");
}

UBAbstractSubPalette::UBAbstractSubPalette(QWidget *parent, Qt::Orientation orient) :
    UBActionPalette(Qt::TopLeftCorner, parent, orient)
    , mMainAction(0)
{
    mCustomPosition = true;
    setFocusPolicy(Qt::StrongFocus); // in order to detect focus loss.
}

UBAbstractSubPalette::UBAbstractSubPalette(Qt::Orientation orient, QWidget *parent) :
    UBActionPalette(orient, parent)
    , mMainAction(0)
{
    mCustomPosition = true;
}

void UBAbstractSubPalette::togglePalette()
{
    show();
    setFocus(); // explicitely give focus to the subPalette, to detect when it loses focus, and then hide it .
}

void UBAbstractSubPalette::triggerAction(QAction *action)
{
    Q_UNUSED(action)
    // NOOP
}

void UBAbstractSubPalette::focusOutEvent(QFocusEvent *)
{
    hide(); // hide subPalette when it loses focus (for exemple : clic anywhere else)
}

void UBAbstractSubPalette::mouseMoveEvent(QMouseEvent *)
{
    // User can't move subPalette with the mouse.
}

UBColorPickerButton::UBColorPickerButton(QWidget *parent)
    :QToolButton(parent)
{
}

void UBColorPickerButton::paintEvent(QPaintEvent * pe)
{
    Q_UNUSED(pe);
    QPainter painter(this);
    painter.setBrush(color());    
    painter.drawRect(margin_left, margin_top, width, height);

    // If transparent color, draw a cross in the rect :
    if (color() == Qt::transparent)
    {
        painter.drawLine(margin_left,margin_top, width+margin_left,height+margin_top);
        painter.drawLine(width+margin_left,margin_top, margin_left,height+margin_top);
    }
}
