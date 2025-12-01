/**
 * @file colors.h
 * @brief Theme Color Definitions
 * @layer Application/GUI/Theme
 * 
 * Defines the ThemeColors structure containing all color values
 * used throughout the application UI.
 */

#ifndef COLORS_H
#define COLORS_H

/* ============================================================================
 * Qt Framework Includes
 * ============================================================================ */
#include <QColor>

/**
 * @struct ThemeColors
 * @brief Complete color palette for a theme
 * 
 * Groups colors by purpose:
 * - Brand colors (greens, accent)
 * - Background colors (primary, secondary, tertiary)
 * - Text colors (primary, secondary, muted)
 * - Border colors
 * - Status indicator colors
 */
struct ThemeColors {
    /* ------------------------------------------------------------------------
     * Brand Colors
     * ------------------------------------------------------------------------ */
    QColor primary_green;    ///< Main brand color (buttons, highlights)
    QColor secondary_green;  ///< Secondary accent green
    QColor accent_orange;    ///< Accent color for warnings/highlights
    QColor alert_red;        ///< Alert/error color

    /* ------------------------------------------------------------------------
     * Background Colors
     * ------------------------------------------------------------------------ */
    QColor bg_primary;       ///< Main window background
    QColor bg_secondary;     ///< Card/panel background
    QColor bg_tertiary;      ///< Hover/selected state background

    /* ------------------------------------------------------------------------
     * Text Colors
     * ------------------------------------------------------------------------ */
    QColor text_primary;     ///< Main text color
    QColor text_secondary;   ///< Secondary/label text
    QColor text_muted;       ///< Disabled/hint text

    /* ------------------------------------------------------------------------
     * Border Colors
     * ------------------------------------------------------------------------ */
    QColor border_light;     ///< Light border (separators)
    QColor border_dark;      ///< Dark border (containers)

    /* ------------------------------------------------------------------------
     * Status Indicator Colors
     * ------------------------------------------------------------------------ */
    QColor status_healthy;   ///< Green - all parameters normal
    QColor status_warning;   ///< Orange - approaching limits
    QColor status_critical;  ///< Red - out of range
};

#endif // COLORS_H
