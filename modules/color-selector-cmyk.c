/* GIMP CMYK ColorSelector using littleCMS
 * Copyright (C) 2006  Sven Neumann <sven@gimp.org>
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

#include <lcms2.h>

#include <gegl.h>
#include <gtk/gtk.h>

#include "libgimpcolor/gimpcolor.h"
#include "libgimpconfig/gimpconfig.h"
#include "libgimpmodule/gimpmodule.h"
#include "libgimpwidgets/gimpwidgets.h"

#include "libgimp/libgimp-intl.h"


/* definitions and variables */

#define COLORSEL_TYPE_CMYK            (colorsel_cmyk_get_type ())
#define COLORSEL_CMYK(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), COLORSEL_TYPE_CMYK, ColorselCmyk))
#define COLORSEL_CMYK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), COLORSEL_TYPE_CMYK, ColorselCmykClass))
#define COLORSEL_IS_CMYK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), COLORSEL_TYPE_CMYK))
#define COLORSEL_IS_CMYK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), COLORSEL_TYPE_CMYK))


typedef struct _ColorselCmyk      ColorselCmyk;
typedef struct _ColorselCmykClass ColorselCmykClass;

struct _ColorselCmyk
{
  GimpColorSelector  parent_instance;

  GimpColorConfig   *config;
  cmsHTRANSFORM      rgb2cmyk;
  cmsHTRANSFORM      cmyk2rgb;

  GimpCMYK           cmyk;
  GtkAdjustment     *adj[4];
  GtkWidget         *name_label;

  gboolean           in_destruction;
};

struct _ColorselCmykClass
{
  GimpColorSelectorClass  parent_class;
};


static GType  colorsel_cmyk_get_type       (void);

static void   colorsel_cmyk_dispose        (GObject           *object);

static void   colorsel_cmyk_set_color      (GimpColorSelector *selector,
                                            const GimpRGB     *rgb,
                                            const GimpHSV     *hsv);
static void   colorsel_cmyk_set_config     (GimpColorSelector *selector,
                                            GimpColorConfig   *config);

static void   colorsel_cmyk_adj_update     (GtkAdjustment     *adj,
                                            ColorselCmyk      *module);
static void   colorsel_cmyk_config_changed (ColorselCmyk      *module);


static const GimpModuleInfo colorsel_cmyk_info =
{
  GIMP_MODULE_ABI_VERSION,
  N_("CMYK color selector (using color profile)"),
  "Sven Neumann <sven@gimp.org>",
  "v0.1",
  "(c) 2006, released under the GPL",
  "September 2006"
};


G_DEFINE_DYNAMIC_TYPE (ColorselCmyk, colorsel_cmyk,
                       GIMP_TYPE_COLOR_SELECTOR)


G_MODULE_EXPORT const GimpModuleInfo *
gimp_module_query (GTypeModule *module)
{
  return &colorsel_cmyk_info;
}

G_MODULE_EXPORT gboolean
gimp_module_register (GTypeModule *module)
{
  colorsel_cmyk_register_type (module);

  return TRUE;
}

static void
colorsel_cmyk_class_init (ColorselCmykClass *klass)
{
  GObjectClass           *object_class   = G_OBJECT_CLASS (klass);
  GimpColorSelectorClass *selector_class = GIMP_COLOR_SELECTOR_CLASS (klass);

  object_class->dispose      = colorsel_cmyk_dispose;

  selector_class->name       = _("CMYK");
  selector_class->help_id    = "gimp-colorselector-cmyk";
  selector_class->stock_id   = GTK_STOCK_PRINT;  /* FIXME */
  selector_class->set_color  = colorsel_cmyk_set_color;
  selector_class->set_config = colorsel_cmyk_set_config;
}

static void
colorsel_cmyk_class_finalize (ColorselCmykClass *klass)
{
}

static void
colorsel_cmyk_init (ColorselCmyk *module)
{
  GtkWidget *table;
  GtkObject *adj;
  gint       i;

  static const gchar * const cmyk_labels[] =
  {
    /* Cyan        */
    N_("_C"),
    /* Magenta     */
    N_("_M"),
    /* Yellow      */
    N_("_Y"),
    /* Key (Black) */
    N_("_K")
  };
  static const gchar * const cmyk_tips[] =
  {
    N_("Cyan"),
    N_("Magenta"),
    N_("Yellow"),
    N_("Black")
  };

  module->config   = NULL;
  module->rgb2cmyk = NULL;
  module->cmyk2rgb = NULL;

  gtk_box_set_spacing (GTK_BOX (module), 6);

  table = gtk_table_new (4, 4, FALSE);

  gtk_table_set_row_spacings (GTK_TABLE (table), 1);
  gtk_table_set_col_spacings (GTK_TABLE (table), 2);
  gtk_table_set_col_spacing (GTK_TABLE (table), 0, 0);

  gtk_box_pack_start (GTK_BOX (module), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  for (i = 0; i < 4; i++)
    {
      adj = gimp_scale_entry_new (GTK_TABLE (table), 1, i,
                                  gettext (cmyk_labels[i]),
                                  -1, -1,
                                  0.0,
                                  0.0, 100.0,
                                  1.0, 10.0,
                                  0,
                                  TRUE, 0.0, 0.0,
                                  gettext (cmyk_tips[i]),
                                  NULL);

      g_signal_connect (adj, "value-changed",
                        G_CALLBACK (colorsel_cmyk_adj_update),
                        module);

      module->adj[i] = GTK_ADJUSTMENT (adj);
    }

  module->name_label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (module->name_label), 0.0, 0.5);
  gtk_label_set_ellipsize (GTK_LABEL (module->name_label), PANGO_ELLIPSIZE_END);
  gimp_label_set_attributes (GTK_LABEL (module->name_label),
                             PANGO_ATTR_SCALE, PANGO_SCALE_SMALL,
                             -1);
  gtk_box_pack_start (GTK_BOX (module), module->name_label, FALSE, FALSE, 0);
  gtk_widget_show (module->name_label);
}

static void
colorsel_cmyk_dispose (GObject *object)
{
  ColorselCmyk *module = COLORSEL_CMYK (object);

  module->in_destruction = TRUE;

  colorsel_cmyk_set_config (GIMP_COLOR_SELECTOR (object), NULL);

  G_OBJECT_CLASS (colorsel_cmyk_parent_class)->dispose (object);
}

static void
colorsel_cmyk_set_color (GimpColorSelector *selector,
                         const GimpRGB     *rgb,
                         const GimpHSV     *hsv)
{
  ColorselCmyk *module = COLORSEL_CMYK (selector);
  gdouble       values[4];
  gint          i;

  if (module->rgb2cmyk)
    {
      gdouble rgb_values[3];
      gdouble cmyk_values[4];

      rgb_values[0] = rgb->r;
      rgb_values[1] = rgb->g;
      rgb_values[2] = rgb->b;

      cmsDoTransform (module->rgb2cmyk, rgb_values, cmyk_values, 1);

      module->cmyk.c = cmyk_values[0] / 100.0;
      module->cmyk.m = cmyk_values[1] / 100.0;
      module->cmyk.y = cmyk_values[2] / 100.0;
      module->cmyk.k = cmyk_values[3] / 100.0;
    }
  else
    {
      gimp_rgb_to_cmyk (rgb, 1.0, &module->cmyk);
    }

  values[0] = module->cmyk.c * 100.0;
  values[1] = module->cmyk.m * 100.0;
  values[2] = module->cmyk.y * 100.0;
  values[3] = module->cmyk.k * 100.0;

  for (i = 0; i < 4; i++)
    {
      g_signal_handlers_block_by_func (module->adj[i],
                                       colorsel_cmyk_adj_update,
                                       module);

      gtk_adjustment_set_value (module->adj[i], values[i]);

      g_signal_handlers_unblock_by_func (module->adj[i],
                                         colorsel_cmyk_adj_update,
                                         module);
    }
}

static void
colorsel_cmyk_set_config (GimpColorSelector *selector,
                          GimpColorConfig   *config)
{
  ColorselCmyk *module = COLORSEL_CMYK (selector);

  if (config == module->config)
    return;

  if (module->config)
    {
      g_signal_handlers_disconnect_by_func (module->config,
                                            G_CALLBACK (colorsel_cmyk_config_changed),
                                            module);
      g_object_unref (module->config);
    }

  module->config = config;

  if (module->config)
    {
      g_object_ref (module->config);
      g_signal_connect_swapped (module->config, "notify",
                                G_CALLBACK (colorsel_cmyk_config_changed),
                                module);
    }

  colorsel_cmyk_config_changed (module);
}

static void
colorsel_cmyk_adj_update (GtkAdjustment *adj,
                          ColorselCmyk  *module)
{
  GimpColorSelector *selector = GIMP_COLOR_SELECTOR (module);
  gint               i;
  gdouble            value;

  for (i = 0; i < 4; i++)
    if (module->adj[i] == adj)
      break;

  value = gtk_adjustment_get_value (adj) / 100.0;

  switch (i)
    {
    case 0:
      module->cmyk.c = value;
      break;
    case 1:
      module->cmyk.m = value;
      break;
    case 2:
      module->cmyk.y = value;
      break;
    case 3:
      module->cmyk.k = value;
      break;
    default:
      return;
    }

  if (module->cmyk2rgb)
    {
      gdouble cmyk_values[4];
      gdouble rgb_values[3];

      cmyk_values[0] = module->cmyk.c * 100.0;
      cmyk_values[1] = module->cmyk.m * 100.0;
      cmyk_values[2] = module->cmyk.y * 100.0;
      cmyk_values[3] = module->cmyk.k * 100.0;

      cmsDoTransform (module->cmyk2rgb, cmyk_values, rgb_values, 1);

      selector->rgb.r = rgb_values[0];
      selector->rgb.g = rgb_values[1];
      selector->rgb.b = rgb_values[2];
    }
  else
    {
      gimp_cmyk_to_rgb (&module->cmyk, &selector->rgb);
    }

  gimp_rgb_to_hsv (&selector->rgb, &selector->hsv);

  gimp_color_selector_color_changed (selector);
}

static cmsHPROFILE
color_config_get_rgb_profile (GimpColorConfig *config)
{
  cmsHPROFILE profile = NULL;

  if (config->rgb_profile)
    profile = cmsOpenProfileFromFile (config->rgb_profile, "r");

  return profile ? profile : gimp_lcms_create_srgb_profile ();
}

static cmsHPROFILE
color_config_get_cmyk_profile (GimpColorConfig *config)
{
  cmsHPROFILE profile = NULL;

  if (config->cmyk_profile)
    profile = cmsOpenProfileFromFile (config->cmyk_profile, "r");

  return profile;
}

static void
colorsel_cmyk_config_changed (ColorselCmyk *module)
{
  GimpColorSelector *selector = GIMP_COLOR_SELECTOR (module);
  GimpColorConfig   *config   = module->config;
  cmsUInt32Number    flags    = 0;
  cmsHPROFILE        rgb_profile;
  cmsHPROFILE        cmyk_profile;
  gchar             *label;
  gchar             *summary;
  gchar             *text;

  if (module->rgb2cmyk)
    {
      cmsDeleteTransform (module->rgb2cmyk);
      module->rgb2cmyk = NULL;
    }

  if (module->cmyk2rgb)
    {
      cmsDeleteTransform (module->cmyk2rgb);
      module->cmyk2rgb = NULL;
    }

  gtk_label_set_text (GTK_LABEL (module->name_label), _("Profile: (none)"));
  gimp_help_set_help_data (module->name_label, NULL, NULL);

  if (! config)
    goto out;

  rgb_profile  = color_config_get_rgb_profile (config);
  cmyk_profile = color_config_get_cmyk_profile (config);

  if (! cmyk_profile)
    goto out;

  label   = gimp_lcms_profile_get_label (cmyk_profile);
  summary = gimp_lcms_profile_get_summary (cmyk_profile);

  text = g_strdup_printf (_("Profile: %s"), label);
  gtk_label_set_text (GTK_LABEL (module->name_label), text);
  gimp_help_set_help_data (module->name_label, summary, NULL);

  g_free (text);
  g_free (label);
  g_free (summary);

  if (config->display_intent ==
      GIMP_COLOR_RENDERING_INTENT_RELATIVE_COLORIMETRIC)
    {
      flags |= cmsFLAGS_BLACKPOINTCOMPENSATION;
    }

  module->rgb2cmyk = cmsCreateTransform (rgb_profile,  TYPE_RGB_DBL,
                                         cmyk_profile, TYPE_CMYK_DBL,
                                         config->display_intent,
                                         flags);
  module->cmyk2rgb = cmsCreateTransform (cmyk_profile, TYPE_CMYK_DBL,
                                         rgb_profile,  TYPE_RGB_DBL,
                                         config->display_intent,
                                         flags);

  cmsCloseProfile (rgb_profile);
  cmsCloseProfile (cmyk_profile);

 out:
  if (! module->in_destruction)
    colorsel_cmyk_set_color (selector, &selector->rgb, &selector->hsv);
}
