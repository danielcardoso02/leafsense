#ifndef COLORS_H
#define COLORS_H

#include <QColor>

struct ThemeColors {
    QColor primary_green;
    QColor secondary_green;
    QColor accent_orange;
    QColor alert_red;

    QColor bg_primary;
    QColor bg_secondary;
    QColor bg_tertiary;

    QColor text_primary;
    QColor text_secondary;
    QColor text_muted;

    QColor border_light;
    QColor border_dark;

    QColor status_healthy;
    QColor status_warning;
    QColor status_critical;
};

#endif // COLORS_H
