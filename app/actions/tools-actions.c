/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <string.h>

#include <gegl.h>
#include <gtk/gtk.h>

#include "libgimpwidgets/gimpwidgets.h"

#include "actions-types.h"

#include "core/gimp.h"
#include "core/gimpcontainer.h"
#include "core/gimpcontext.h"
#include "core/gimptoolinfo.h"

#include "widgets/gimpactiongroup.h"
#include "widgets/gimphelp-ids.h"

#include "actions.h"
#include "tools-actions.h"
#include "tools-commands.h"

#include "gimp-intl.h"


static const GimpActionEntry tools_actions[] =
{
  { "tools-menu",           NULL, NC_("tools-action", "_Tools")           },
  { "tools-select-menu",    NULL, NC_("tools-action", "_Selection Tools") },
  { "tools-paint-menu",     NULL, NC_("tools-action", "_Paint Tools")     },
  { "tools-transform-menu", NULL, NC_("tools-action", "_Transform Tools") },
  { "tools-color-menu",     NULL, NC_("tools-action", "_Color Tools")     },
};

static const GimpStringActionEntry tools_alternative_actions[] =
{
  { "tools-by-color-select-short", GIMP_STOCK_TOOL_BY_COLOR_SELECT,
    NC_("tools-action", "_By Color"), NULL,
    NC_("tools-action", "Select regions with similar colors"),
    "gimp-by-color-select-tool",
    GIMP_HELP_TOOL_BY_COLOR_SELECT },

  { "tools-rotate-arbitrary", GIMP_STOCK_TOOL_ROTATE,
    NC_("tools-action", "_Arbitrary Rotation..."), "",
    NC_("tools-action", "Rotate by an arbitrary angle"),
    "gimp-rotate-layer",
    GIMP_HELP_TOOL_ROTATE }
};

static const GimpEnumActionEntry tools_color_average_radius_actions[] =
{
  { "tools-color-average-radius-set", GIMP_STOCK_TOOL_COLOR_PICKER,
    "Set Color Picker Radius", NULL, NULL,
    GIMP_ACTION_SELECT_SET, TRUE,
    NULL },
  { "tools-color-average-set-to-default", GIMP_STOCK_TOOL_COLOR_PICKER,
    "Set Color Picker Radius To Default Value", NULL, NULL,
    GIMP_ACTION_SELECT_SET_TO_DEFAULT, FALSE,
    NULL },
  { "tools-color-average-radius-minimum", GIMP_STOCK_TOOL_COLOR_PICKER,
    "Minimize Color Picker Radius", NULL, NULL,
    GIMP_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "tools-color-average-radius-maximum", GIMP_STOCK_TOOL_COLOR_PICKER,
    "Maximize Color Picker Radius", NULL, NULL,
    GIMP_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "tools-color-average-radius-decrease", GIMP_STOCK_TOOL_COLOR_PICKER,
    "Decrease Color Picker Radius", NULL, NULL,
    GIMP_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "tools-color-average-radius-increase", GIMP_STOCK_TOOL_COLOR_PICKER,
    "Increase Color Picker Radius", NULL, NULL,
    GIMP_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "tools-color-average-radius-decrease-skip",
    GIMP_STOCK_TOOL_COLOR_PICKER,
    "Decrease Color Picker Radius More", NULL, NULL,
    GIMP_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "tools-color-average-radius-increase-skip",
    GIMP_STOCK_TOOL_COLOR_PICKER,
    "Increase Color Picker Radius More", NULL, NULL,
    GIMP_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL },
};

static const GimpEnumActionEntry tools_paint_brush_size_actions[] =
{
  { "tools-paint-brush-size-set", GIMP_STOCK_TOOL_PAINTBRUSH,
    "Set Brush Size", NULL, NULL,
    GIMP_ACTION_SELECT_SET, TRUE,
    NULL },
  { "tools-paint-brush-size-set-to-default", GIMP_STOCK_TOOL_PAINTBRUSH,
    "Set Brush Size To Default Value", NULL, NULL,
    GIMP_ACTION_SELECT_SET_TO_DEFAULT, FALSE,
    NULL },
  { "tools-paint-brush-size-minimum", GIMP_STOCK_TOOL_PAINTBRUSH,
    "Minimize Brush Size", NULL, NULL,
    GIMP_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "tools-paint-brush-size-maximum", GIMP_STOCK_TOOL_PAINTBRUSH,
    "Maximize Brush Size", NULL, NULL,
    GIMP_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "tools-paint-brush-size-decrease", GIMP_STOCK_TOOL_PAINTBRUSH,
    "Decrease Brush Size", NULL, NULL,
    GIMP_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "tools-paint-brush-size-increase", GIMP_STOCK_TOOL_PAINTBRUSH,
    "Increase Brush Size", NULL, NULL,
    GIMP_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "tools-paint-brush-size-decrease-skip", GIMP_STOCK_TOOL_PAINTBRUSH,
    "Decrease Brush Size More", NULL, NULL,
    GIMP_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "tools-paint-brush-size-increase-skip", GIMP_STOCK_TOOL_PAINTBRUSH,
    "Increase Brush Size More", NULL, NULL,
    GIMP_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL },
  { "tools-paint-brush-size-decrease-percent", GIMP_STOCK_TOOL_PAINTBRUSH,
    "Decrease Brush Size Relative", NULL, NULL,
    GIMP_ACTION_SELECT_PERCENT_PREVIOUS, FALSE,
    NULL },
  { "tools-paint-brush-size-increase-percent", GIMP_STOCK_TOOL_PAINTBRUSH,
    "Increase Brush Size Relative", NULL, NULL,
    GIMP_ACTION_SELECT_PERCENT_NEXT, FALSE,
    NULL },
};

static const GimpEnumActionEntry tools_paint_brush_angle_actions[] =
{
  { "tools-paint-brush-angle-set", GIMP_STOCK_TOOL_PAINTBRUSH,
    "Set Brush Angle", NULL, NULL,
    GIMP_ACTION_SELECT_SET, TRUE,
    NULL },
  { "tools-paint-brush-angle-set-to-default", GIMP_STOCK_TOOL_PAINTBRUSH,
    "Set Brush Angle To Default Value", NULL, NULL,
    GIMP_ACTION_SELECT_SET_TO_DEFAULT, FALSE,
    NULL },
  { "tools-paint-brush-angle-minimum", GIMP_STOCK_TOOL_PAINTBRUSH,
    "Minimize Brush Angle", NULL, NULL,
    GIMP_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "tools-paint-brush-angle-maximum", GIMP_STOCK_TOOL_PAINTBRUSH,
    "Maximize Brush Angle", NULL, NULL,
    GIMP_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "tools-paint-brush-angle-decrease", GIMP_STOCK_TOOL_PAINTBRUSH,
    "Decrease Brush Angle", NULL, NULL,
    GIMP_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "tools-paint-brush-angle-increase", GIMP_STOCK_TOOL_PAINTBRUSH,
    "Increase Brush Angle", NULL, NULL,
    GIMP_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "tools-paint-brush-angle-decrease-skip", GIMP_STOCK_TOOL_PAINTBRUSH,
    "Decrease Brush Angle More", NULL, NULL,
    GIMP_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "tools-paint-brush-angle-increase-skip", GIMP_STOCK_TOOL_PAINTBRUSH,
    "Increase Brush Angle More", NULL, NULL,
    GIMP_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL },
};

static const GimpEnumActionEntry tools_paint_brush_aspect_ratio_actions[] =
{
  { "tools-paint-brush-aspect-ratio-set", GIMP_STOCK_TOOL_PAINTBRUSH,
    "Set Brush Aspect Ratio", NULL, NULL,
    GIMP_ACTION_SELECT_SET, TRUE,
    NULL },
  { "tools-paint-brush-aspect-ratio-set-to-default", GIMP_STOCK_TOOL_PAINTBRUSH,
    "Set Brush Aspect Ratio To Default Value", NULL, NULL,
    GIMP_ACTION_SELECT_SET_TO_DEFAULT, FALSE,
    NULL },
  { "tools-paint-brush-aspect-ratio-minimum", GIMP_STOCK_TOOL_PAINTBRUSH,
    "Minimize Brush Aspect Ratio", NULL, NULL,
    GIMP_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "tools-paint-brush-aspect-ratio-maximum", GIMP_STOCK_TOOL_PAINTBRUSH,
    "Maximize Brush Aspect Ratio", NULL, NULL,
    GIMP_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "tools-paint-brush-aspect-ratio-decrease", GIMP_STOCK_TOOL_PAINTBRUSH,
    "Decrease Brush Aspect Ratio", NULL, NULL,
    GIMP_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "tools-paint-brush-aspect-ratio-increase", GIMP_STOCK_TOOL_PAINTBRUSH,
    "Increase Brush Aspect Ratio", NULL, NULL,
    GIMP_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "tools-paint-brush-aspect-ratio-decrease-skip", GIMP_STOCK_TOOL_PAINTBRUSH,
    "Decrease Brush Aspect Ratio More", NULL, NULL,
    GIMP_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "tools-paint-brush-aspect-ratio-increase-skip", GIMP_STOCK_TOOL_PAINTBRUSH,
    "Increase Brush Aspect Ratio More", NULL, NULL,
    GIMP_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL },
};

static const GimpEnumActionEntry tools_ink_blob_size_actions[] =
{
  { "tools-ink-blob-size-set", GIMP_STOCK_TOOL_INK,
    "Set Ink Blob Size", NULL, NULL,
    GIMP_ACTION_SELECT_SET, TRUE,
    NULL },
  { "tools-ink-blob-size-minimum", GIMP_STOCK_TOOL_INK,
    "Minimize Ink Blob Size", NULL, NULL,
    GIMP_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "tools-ink-blob-size-maximum", GIMP_STOCK_TOOL_INK,
    "Maximize Ink Blob Size", NULL, NULL,
    GIMP_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "tools-ink-blob-size-decrease", GIMP_STOCK_TOOL_INK,
    "Decrease Ink Blob Size", NULL, NULL,
    GIMP_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "tools-ink-blob-size-increase", GIMP_STOCK_TOOL_INK,
    "Increase Ink Blob Size", NULL, NULL,
    GIMP_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "tools-ink-blob-size-decrease-skip", GIMP_STOCK_TOOL_INK,
    "Decrease Ink Blob Size More", NULL, NULL,
    GIMP_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "tools-ink-blob-size-increase-skip", GIMP_STOCK_TOOL_INK,
    "Increase Ink Blob Size More", NULL, NULL,
    GIMP_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL },
  { "tools-ink-blob-size-decrease-percent", GIMP_STOCK_TOOL_INK,
    "Decrease Ink Blob Size Relative", NULL, NULL,
    GIMP_ACTION_SELECT_PERCENT_PREVIOUS, FALSE,
    NULL },
  { "tools-ink-blob-size-increase-percent", GIMP_STOCK_TOOL_INK,
    "Increase Ink Blob Size Relative", NULL, NULL,
    GIMP_ACTION_SELECT_PERCENT_NEXT, FALSE,
    NULL },
};

static const GimpEnumActionEntry tools_ink_blob_aspect_actions[] =
{
  { "tools-ink-blob-aspect-set", GIMP_STOCK_TOOL_INK,
    "Set Ink Blob Aspect", NULL, NULL,
    GIMP_ACTION_SELECT_SET, TRUE,
    NULL },
  { "tools-ink-blob-aspect-set-to-default", GIMP_STOCK_TOOL_INK,
    "Set Ink Blob Aspect To Default Value", NULL, NULL,
    GIMP_ACTION_SELECT_SET_TO_DEFAULT, FALSE,
    NULL },
  { "tools-ink-blob-aspect-minimum", GIMP_STOCK_TOOL_INK,
    "Minimize Ink Blob Aspect", NULL, NULL,
    GIMP_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "tools-ink-blob-aspect-maximum", GIMP_STOCK_TOOL_INK,
    "Maximize Ink Blob Aspect", NULL, NULL,
    GIMP_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "tools-ink-blob-aspect-decrease", GIMP_STOCK_TOOL_INK,
    "Decrease Ink Blob Aspect", NULL, NULL,
    GIMP_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "tools-ink-blob-aspect-increase", GIMP_STOCK_TOOL_INK,
    "Increase Ink Blob Aspect", NULL, NULL,
    GIMP_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "tools-ink-blob-aspect-decrease-skip", GIMP_STOCK_TOOL_INK,
    "Decrease Ink Blob Aspect More", NULL, NULL,
    GIMP_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "tools-ink-blob-aspect-increase-skip", GIMP_STOCK_TOOL_INK,
    "Increase Ink Blob Aspect More", NULL, NULL,
    GIMP_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL },
};

static const GimpEnumActionEntry tools_ink_blob_angle_actions[] =
{
  { "tools-ink-blob-angle-set", GIMP_STOCK_TOOL_INK,
    "Set Ink Blob Angle", NULL, NULL,
    GIMP_ACTION_SELECT_SET, TRUE,
    NULL },
  { "tools-ink-blob-angle-minimum", GIMP_STOCK_TOOL_INK,
    "Minimize Ink Blob Angle", NULL, NULL,
    GIMP_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "tools-ink-blob-angle-maximum", GIMP_STOCK_TOOL_INK,
    "Maximize Ink Blob Angle", NULL, NULL,
    GIMP_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "tools-ink-blob-angle-decrease", GIMP_STOCK_TOOL_INK,
    "Decrease Ink Blob Angle", NULL, NULL,
    GIMP_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "tools-ink-blob-angle-increase", GIMP_STOCK_TOOL_INK,
    "Increase Ink Blob Angle", NULL, NULL,
    GIMP_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "tools-ink-blob-angle-decrease-skip", GIMP_STOCK_TOOL_INK,
    "Decrease Ink Blob Angle More", NULL, NULL,
    GIMP_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "tools-ink-blob-angle-increase-skip", GIMP_STOCK_TOOL_INK,
    "Increase Ink Blob Angle More", NULL, NULL,
    GIMP_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL },
};

static const GimpEnumActionEntry tools_foreground_select_brush_size_actions[] =
{
  { "tools-foreground-select-brush-size-set",
    GIMP_STOCK_TOOL_FOREGROUND_SELECT,
    "Set Foreground Select Brush Size", NULL, NULL,
    GIMP_ACTION_SELECT_SET, TRUE,
    NULL },
  { "tools-foreground-select-brush-size-set-to-default",
    GIMP_STOCK_TOOL_FOREGROUND_SELECT,
    "Set Foreground Select Brush Size to Default Value", NULL, NULL,
    GIMP_ACTION_SELECT_SET_TO_DEFAULT, FALSE,
    NULL },
  { "tools-foreground-select-brush-size-minimum",
    GIMP_STOCK_TOOL_FOREGROUND_SELECT,
    "Minimize Foreground Select Brush Size", NULL, NULL,
    GIMP_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "tools-foreground-select-brush-size-maximum",
    GIMP_STOCK_TOOL_FOREGROUND_SELECT,
    "Maximize Foreground Select Brush Size", NULL, NULL,
    GIMP_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "tools-foreground-select-brush-size-decrease",
    GIMP_STOCK_TOOL_FOREGROUND_SELECT,
    "Decrease Foreground Select Brush Size", NULL, NULL,
    GIMP_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "tools-foreground-select-brush-size-increase",
    GIMP_STOCK_TOOL_FOREGROUND_SELECT,
    "Increase Foreground Select Brush Size", NULL, NULL,
    GIMP_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "tools-foreground-select-brush-size-decrease-skip",
    GIMP_STOCK_TOOL_FOREGROUND_SELECT,
    "Decrease Foreground Select Brush Size More", NULL, NULL,
    GIMP_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "tools-foreground-select-brush-size-increase-skip",
    GIMP_STOCK_TOOL_FOREGROUND_SELECT,
    "Increase Foreground Select Brush Size More", NULL, NULL,
    GIMP_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL },
  { "tools-foreground-select-brush-size-decrease-percent",
    GIMP_STOCK_TOOL_FOREGROUND_SELECT,
    "Decrease Foreground Select Brush Size Relative", NULL, NULL,
    GIMP_ACTION_SELECT_PERCENT_PREVIOUS, FALSE,
    NULL },
  { "tools-foreground-select-brush-size-increase-percent",
    GIMP_STOCK_TOOL_FOREGROUND_SELECT,
    "Increase Foreground Select Brush Size", NULL, NULL,
    GIMP_ACTION_SELECT_PERCENT_NEXT, FALSE,
    NULL },
};

static const GimpEnumActionEntry tools_transform_preview_opacity_actions[] =
{
  { "tools-transform-preview-opacity-set", GIMP_STOCK_TOOL_PERSPECTIVE,
    "Set Transform Tool Preview Opacity", NULL, NULL,
    GIMP_ACTION_SELECT_SET, TRUE,
    NULL },
  { "tools-transform-preview-opacity-minimum", GIMP_STOCK_TOOL_PERSPECTIVE,
    "Minimize Transform Tool Preview Opacity", NULL, NULL,
    GIMP_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "tools-transform-preview-opacity-maximum", GIMP_STOCK_TOOL_PERSPECTIVE,
    "Maximize Transform Tool Preview Opacity", NULL, NULL,
    GIMP_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "tools-transform-preview-opacity-decrease", GIMP_STOCK_TOOL_PERSPECTIVE,
    "Decrease Transform Tool Preview Opacity", NULL, NULL,
    GIMP_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "tools-transform-preview-opacity-increase", GIMP_STOCK_TOOL_PERSPECTIVE,
    "Increase Transform Tool Preview Opacity", NULL, NULL,
    GIMP_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "tools-transform-preview-opacity-decrease-skip", GIMP_STOCK_TOOL_PERSPECTIVE,
    "Decrease Transform Tool Preview Opacity More", NULL, NULL,
    GIMP_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "tools-transform-preview-opacity-increase-skip", GIMP_STOCK_TOOL_PERSPECTIVE,
    "Increase Transform Tool Preview Opacity More", NULL, NULL,
    GIMP_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL },
};

static const GimpEnumActionEntry tools_opacity_actions[] =
{
  { "tools-opacity-set", GIMP_STOCK_TOOL_OPTIONS,
    "Set Opacity", NULL, NULL,
    GIMP_ACTION_SELECT_SET, TRUE,
    NULL },
  { "tools-opacity-set-to-default", GIMP_STOCK_TOOL_OPTIONS,
    "Set Opacity To Default Value", NULL, NULL,
    GIMP_ACTION_SELECT_SET_TO_DEFAULT, FALSE,
    NULL },
  { "tools-opacity-minimum", GIMP_STOCK_TOOL_OPTIONS,
    "Minimize Opacity", NULL, NULL,
    GIMP_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "tools-opacity-maximum", GIMP_STOCK_TOOL_OPTIONS,
    "Maximize Opacity", NULL, NULL,
    GIMP_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "tools-opacity-decrease", GIMP_STOCK_TOOL_OPTIONS,
    "Decrease Opacity", "less", NULL,
    GIMP_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "tools-opacity-increase", GIMP_STOCK_TOOL_OPTIONS,
    "Increase Opacity", "greater", NULL,
    GIMP_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "tools-opacity-decrease-skip", GIMP_STOCK_TOOL_OPTIONS,
    "Decrease Opacity More", "<primary>less", NULL,
    GIMP_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "tools-opacity-increase-skip", GIMP_STOCK_TOOL_OPTIONS,
    "Increase Opacity More", "<primary>greater", NULL,
    GIMP_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL },
  { "tools-opacity-decrease-percent", GIMP_STOCK_TOOL_OPTIONS,
    "Decrease Opacity Relative", NULL, NULL,
    GIMP_ACTION_SELECT_PERCENT_PREVIOUS, FALSE,
    NULL },
  { "tools-opacity-increase-percent", GIMP_STOCK_TOOL_OPTIONS,
    "Increase Opacity Relative", NULL, NULL,
    GIMP_ACTION_SELECT_PERCENT_NEXT, FALSE,
    NULL },
};

static const GimpEnumActionEntry tools_size_actions[] =
{
  { "tools-size-set", GIMP_STOCK_TOOL_OPTIONS,
    "Set Size", NULL, NULL,
    GIMP_ACTION_SELECT_SET, TRUE,
    NULL },
  { "tools-size-set-to-default", GIMP_STOCK_TOOL_OPTIONS,
    "Set Size To Default Value", "backslash", NULL,
    GIMP_ACTION_SELECT_SET_TO_DEFAULT, FALSE,
    NULL },
  { "tools-size-minimum", GIMP_STOCK_TOOL_OPTIONS,
    "Minimize Size", NULL, NULL,
    GIMP_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "tools-size-maximum", GIMP_STOCK_TOOL_OPTIONS,
    "Maximize Size", NULL, NULL,
    GIMP_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "tools-size-decrease", GIMP_STOCK_TOOL_OPTIONS,
    "Decrease Size", "bracketleft", NULL,
    GIMP_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "tools-size-increase", GIMP_STOCK_TOOL_OPTIONS,
    "Increase Size", "bracketright", NULL,
    GIMP_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "tools-size-decrease-skip", GIMP_STOCK_TOOL_OPTIONS,
    "Decrease Size More", "<shift>bracketleft", NULL,
    GIMP_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "tools-size-increase-skip", GIMP_STOCK_TOOL_OPTIONS,
    "Increase Size More", "<shift>bracketright", NULL,
    GIMP_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL },
  { "tools-size-decrease-percent", GIMP_STOCK_TOOL_OPTIONS,
    "Decrease Size Relative", NULL, NULL,
    GIMP_ACTION_SELECT_PERCENT_PREVIOUS, FALSE,
    NULL },
  { "tools-size-increase-percent", GIMP_STOCK_TOOL_OPTIONS,
    "Increase Size Relative", NULL, NULL,
    GIMP_ACTION_SELECT_PERCENT_NEXT, FALSE,
    NULL },
};

static const GimpEnumActionEntry tools_aspect_actions[] =
{
  { "tools-aspect-set", GIMP_STOCK_TOOL_OPTIONS,
    "Set Aspect Ratio", NULL, NULL,
    GIMP_ACTION_SELECT_SET, TRUE,
    NULL },
  { "tools-aspect-set-to-default", GIMP_STOCK_TOOL_OPTIONS,
    "Set Aspect Ratio To Default Value", NULL, NULL,
    GIMP_ACTION_SELECT_SET_TO_DEFAULT, FALSE,
    NULL },
  { "tools-aspect-minimum", GIMP_STOCK_TOOL_OPTIONS,
    "Minimize Aspect Ratio", NULL, NULL,
    GIMP_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "tools-aspect-maximum", GIMP_STOCK_TOOL_OPTIONS,
    "Maximize Aspect Ratio", NULL, NULL,
    GIMP_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "tools-aspect-decrease", GIMP_STOCK_TOOL_OPTIONS,
    "Decrease Aspect Ratio", NULL, NULL,
    GIMP_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "tools-aspect-increase", GIMP_STOCK_TOOL_OPTIONS,
    "Increase Aspect Ratio", NULL, NULL,
    GIMP_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "tools-aspect-decrease-skip", GIMP_STOCK_TOOL_OPTIONS,
    "Decrease Aspect Ratio More", NULL, NULL,
    GIMP_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "tools-aspect-increase-skip", GIMP_STOCK_TOOL_OPTIONS,
    "Increase Aspect Ratio More", NULL, NULL,
    GIMP_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL },
  { "tools-aspect-decrease-percent", GIMP_STOCK_TOOL_OPTIONS,
    "Decrease Aspect Ratio Relative", NULL, NULL,
    GIMP_ACTION_SELECT_PERCENT_PREVIOUS, FALSE,
    NULL },
  { "tools-aspect-increase-percent", GIMP_STOCK_TOOL_OPTIONS,
    "Increase Aspect Ratio Relative", NULL, NULL,
    GIMP_ACTION_SELECT_PERCENT_NEXT, FALSE,
    NULL },
};

static const GimpEnumActionEntry tools_angle_actions[] =
{
  { "tools-angle-set", GIMP_STOCK_TOOL_OPTIONS,
    "Set Angle", NULL, NULL,
    GIMP_ACTION_SELECT_SET, TRUE,
    NULL },
  { "tools-angle-set-to-default", GIMP_STOCK_TOOL_OPTIONS,
    "Set Angle To Default Value", NULL, NULL,
    GIMP_ACTION_SELECT_SET_TO_DEFAULT, FALSE,
    NULL },
  { "tools-angle-minimum", GIMP_STOCK_TOOL_OPTIONS,
    "Minimize Angle", NULL, NULL,
    GIMP_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "tools-angle-maximum", GIMP_STOCK_TOOL_OPTIONS,
    "Maximize Angle", NULL, NULL,
    GIMP_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "tools-angle-decrease", GIMP_STOCK_TOOL_OPTIONS,
    "Decrease Angle", NULL, NULL,
    GIMP_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "tools-angle-increase", GIMP_STOCK_TOOL_OPTIONS,
    "Increase Angle", NULL, NULL,
    GIMP_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "tools-angle-decrease-skip", GIMP_STOCK_TOOL_OPTIONS,
    "Decrease Angle More", NULL, NULL,
    GIMP_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "tools-angle-increase-skip", GIMP_STOCK_TOOL_OPTIONS,
    "Increase Angle More", NULL, NULL,
    GIMP_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL },
  { "tools-angle-decrease-percent", GIMP_STOCK_TOOL_OPTIONS,
    "Decrease Angle Relative", NULL, NULL,
    GIMP_ACTION_SELECT_PERCENT_PREVIOUS, FALSE,
    NULL },
  { "tools-angle-increase-percent", GIMP_STOCK_TOOL_OPTIONS,
    "Increase Angle Relative", NULL, NULL,
    GIMP_ACTION_SELECT_PERCENT_NEXT, FALSE,
    NULL },
};

static const GimpEnumActionEntry tools_object_1_actions[] =
{
  { "tools-object-1-set", GIMP_STOCK_TOOL_OPTIONS,
    "Select Object 1 by Index", NULL, NULL,
    GIMP_ACTION_SELECT_SET, TRUE,
    NULL },
  { "tools-object-1-first", GIMP_STOCK_TOOL_OPTIONS,
    "First Object 1", NULL, NULL,
    GIMP_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "tools-object-1-last", GIMP_STOCK_TOOL_OPTIONS,
    "Last Object 1", NULL, NULL,
    GIMP_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "tools-object-1-previous", GIMP_STOCK_TOOL_OPTIONS,
    "Previous Object 1", NULL, NULL,
    GIMP_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "tools-object-1-next", GIMP_STOCK_TOOL_OPTIONS,
    "Next Object 1", NULL, NULL,
    GIMP_ACTION_SELECT_NEXT, FALSE,
    NULL }
};

static const GimpEnumActionEntry tools_object_2_actions[] =
{
  { "tools-object-2-set", GIMP_STOCK_TOOL_OPTIONS,
    "Select Object 2 by Index", NULL, NULL,
    GIMP_ACTION_SELECT_SET, TRUE,
    NULL },
  { "tools-object-2-first", GIMP_STOCK_TOOL_OPTIONS,
    "First Object 2", NULL, NULL,
    GIMP_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "tools-object-2-last", GIMP_STOCK_TOOL_OPTIONS,
    "Last Object 2", NULL, NULL,
    GIMP_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "tools-object-2-previous", GIMP_STOCK_TOOL_OPTIONS,
    "Previous Object 2", NULL, NULL,
    GIMP_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "tools-object-2-next", GIMP_STOCK_TOOL_OPTIONS,
    "Next Object 2", NULL, NULL,
    GIMP_ACTION_SELECT_NEXT, FALSE,
    NULL }
};


void
tools_actions_setup (GimpActionGroup *group)
{
  GtkAction *action;
  GList     *list;

  gimp_action_group_add_actions (group, "tools-action",
                                 tools_actions,
                                 G_N_ELEMENTS (tools_actions));

  gimp_action_group_add_string_actions (group, "tools-action",
                                        tools_alternative_actions,
                                        G_N_ELEMENTS (tools_alternative_actions),
                                        G_CALLBACK (tools_select_cmd_callback));

  action = gtk_action_group_get_action (GTK_ACTION_GROUP (group),
                                        "tools-by-color-select-short");
  gtk_action_set_accel_path (action, "<Actions>/tools/tools-by-color-select");

  gimp_action_group_add_enum_actions (group, NULL,
                                      tools_color_average_radius_actions,
                                      G_N_ELEMENTS (tools_color_average_radius_actions),
                                      G_CALLBACK (tools_color_average_radius_cmd_callback));

  gimp_action_group_add_enum_actions (group, NULL,
                                      tools_paint_brush_size_actions,
                                      G_N_ELEMENTS (tools_paint_brush_size_actions),
                                      G_CALLBACK (tools_paint_brush_size_cmd_callback));

  gimp_action_group_add_enum_actions (group, NULL,
                                      tools_paint_brush_angle_actions,
                                      G_N_ELEMENTS (tools_paint_brush_angle_actions),
                                      G_CALLBACK (tools_paint_brush_angle_cmd_callback));

  gimp_action_group_add_enum_actions (group, NULL,
                                      tools_paint_brush_aspect_ratio_actions,
                                      G_N_ELEMENTS (tools_paint_brush_aspect_ratio_actions),
                                      G_CALLBACK (tools_paint_brush_aspect_ratio_cmd_callback));

  gimp_action_group_add_enum_actions (group, NULL,
                                      tools_ink_blob_size_actions,
                                      G_N_ELEMENTS (tools_ink_blob_size_actions),
                                      G_CALLBACK (tools_ink_blob_size_cmd_callback));
  gimp_action_group_add_enum_actions (group, NULL,
                                      tools_ink_blob_aspect_actions,
                                      G_N_ELEMENTS (tools_ink_blob_aspect_actions),
                                      G_CALLBACK (tools_ink_blob_aspect_cmd_callback));
  gimp_action_group_add_enum_actions (group, NULL,
                                      tools_ink_blob_angle_actions,
                                      G_N_ELEMENTS (tools_ink_blob_angle_actions),
                                      G_CALLBACK (tools_ink_blob_angle_cmd_callback));

  gimp_action_group_add_enum_actions (group, NULL,
                                      tools_foreground_select_brush_size_actions,
                                      G_N_ELEMENTS (tools_foreground_select_brush_size_actions),
                                      G_CALLBACK (tools_fg_select_brush_size_cmd_callback));

  gimp_action_group_add_enum_actions (group, NULL,
                                      tools_transform_preview_opacity_actions,
                                      G_N_ELEMENTS (tools_transform_preview_opacity_actions),
                                      G_CALLBACK (tools_transform_preview_opacity_cmd_callback));

  gimp_action_group_add_enum_actions (group, NULL,
                                      tools_opacity_actions,
                                      G_N_ELEMENTS (tools_opacity_actions),
                                      G_CALLBACK (tools_opacity_cmd_callback));
  gimp_action_group_add_enum_actions (group, NULL,
                                      tools_size_actions,
                                      G_N_ELEMENTS (tools_size_actions),
                                      G_CALLBACK (tools_size_cmd_callback));
  gimp_action_group_add_enum_actions (group, NULL,
                                      tools_aspect_actions,
                                      G_N_ELEMENTS (tools_aspect_actions),
                                      G_CALLBACK (tools_aspect_cmd_callback));
  gimp_action_group_add_enum_actions (group, NULL,
                                      tools_angle_actions,
                                      G_N_ELEMENTS (tools_angle_actions),
                                      G_CALLBACK (tools_angle_cmd_callback));

  gimp_action_group_add_enum_actions (group, NULL,
                                      tools_object_1_actions,
                                      G_N_ELEMENTS (tools_object_1_actions),
                                      G_CALLBACK (tools_object_1_cmd_callback));
  gimp_action_group_add_enum_actions (group, NULL,
                                      tools_object_2_actions,
                                      G_N_ELEMENTS (tools_object_2_actions),
                                      G_CALLBACK (tools_object_2_cmd_callback));

  for (list = gimp_get_tool_info_iter (group->gimp);
       list;
       list = g_list_next (list))
    {
      GimpToolInfo *tool_info = list->data;

      if (tool_info->menu_label)
        {
          GimpStringActionEntry  entry;
          const gchar           *stock_id;
          const gchar           *identifier;
          gchar                 *tmp;
          gchar                 *name;

          stock_id   = gimp_viewable_get_stock_id (GIMP_VIEWABLE (tool_info));
          identifier = gimp_object_get_name (tool_info);

          tmp = g_strndup (identifier + strlen ("gimp-"),
                           strlen (identifier) - strlen ("gimp--tool"));
          name = g_strdup_printf ("tools-%s", tmp);
          g_free (tmp);

          entry.name        = name;
          entry.stock_id    = stock_id;
          entry.label       = tool_info->menu_label;
          entry.accelerator = tool_info->menu_accel;
          entry.tooltip     = tool_info->help;
          entry.help_id     = tool_info->help_id;
          entry.value       = identifier;

          gimp_action_group_add_string_actions (group, NULL,
                                                &entry, 1,
                                                G_CALLBACK (tools_select_cmd_callback));

          g_free (name);
        }
    }
}

void
tools_actions_update (GimpActionGroup *group,
                      gpointer         data)
{
}
