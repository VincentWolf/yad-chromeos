/*
 * This file is part of YAD.
 *
 * YAD is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * YAD is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with YAD. If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2008-2019, Victor Ananjevsky <ananasik@gmail.com>
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include "yad.h"

YadSettings settings;

void
read_settings (void)
{
  GKeyFile *kf;
  gchar *filename;

  /* set defaults */
  settings.width = settings.height = -1;
  settings.timeout = 0;
  settings.to_indicator = "none";
  settings.show_remain = FALSE;
  settings.combo_always_editable = FALSE;
  settings.term = "xterm -e '%s'";
  settings.open_cmd = "xdg-open '%s'";
  settings.date_format = "%x";
  settings.ignore_unknown = TRUE;
  settings.max_tab = 100;
  settings.debug = FALSE;
  settings.large_preview = FALSE;

  settings.icon_theme = gtk_icon_theme_get_default ();

  filename = g_build_filename (g_get_user_config_dir (), YAD_SETTINGS_FILE, NULL);

  if (g_file_test (filename, G_FILE_TEST_EXISTS))
    {
      kf = g_key_file_new ();

      if (g_key_file_load_from_file (kf, filename, G_KEY_FILE_NONE, NULL))
        {
          if (g_key_file_has_key (kf, "General", "width", NULL))
            settings.width = g_key_file_get_integer (kf, "General", "width", NULL);
          if (g_key_file_has_key (kf, "General", "height", NULL))
            settings.height = g_key_file_get_integer (kf, "General", "height", NULL);
          if (g_key_file_has_key (kf, "General", "timeout", NULL))
            settings.timeout = g_key_file_get_integer (kf, "General", "timeout", NULL);
          if (g_key_file_has_key (kf, "General", "timeout_indicator", NULL))
            settings.to_indicator = g_key_file_get_string (kf, "General", "timeout_indicator", NULL);
          if (g_key_file_has_key (kf, "General", "show_remain", NULL))
            settings.show_remain = g_key_file_get_boolean (kf, "General", "show_remain", NULL);
          if (g_key_file_has_key (kf, "General", "combo_always_editable", NULL))
            settings.combo_always_editable = g_key_file_get_boolean (kf, "General", "combo_always_editable", NULL);
          if (g_key_file_has_key (kf, "General", "terminal", NULL))
            settings.term = g_key_file_get_string (kf, "General", "terminal", NULL);
          if (g_key_file_has_key (kf, "General", "open_command", NULL))
            settings.open_cmd = g_key_file_get_string (kf, "General", "open_command", NULL);
          if (g_key_file_has_key (kf, "General", "date_format", NULL))
            settings.date_format = g_key_file_get_string (kf, "General", "date_format", NULL);
          if (g_key_file_has_key (kf, "General", "ignore_unknown_options", NULL))
            settings.ignore_unknown = g_key_file_get_boolean (kf, "General", "ignore_unknown_options", NULL);
          if (g_key_file_has_key (kf, "General", "max_tab", NULL))
            settings.max_tab = g_key_file_get_integer (kf, "General", "max_tab", NULL);
          if (g_key_file_has_key (kf, "General", "debug", NULL))
            settings.debug = g_key_file_get_boolean (kf, "General", "debug", NULL);
          if (g_key_file_has_key (kf, "General", "large_preview", NULL))
            settings.large_preview = g_key_file_get_boolean (kf, "General", "large_preview", NULL);
        }

      g_key_file_free (kf);
    }
  else
    write_settings ();

  g_free (filename);
}

static void
set_comment (GKeyFile *kf, gchar *key, gchar *s)
{
  gchar *comment;
  comment = g_strconcat(" ", s, NULL);
  g_key_file_set_comment (kf, "General", key, comment, NULL);
  g_free (comment);
}

void
write_settings (void)
{
  GKeyFile *kf;
  gchar *context;

  kf = g_key_file_new ();

  g_key_file_set_integer (kf, "General", "width", settings.width);
  set_comment(kf, "width", _("Default dialog width"));
  g_key_file_set_integer (kf, "General", "height", settings.height);
  set_comment(kf, "height", _("Default dialog height"));
  g_key_file_set_integer (kf, "General", "timeout", settings.timeout);
  set_comment(kf, "timeout", _("Default timeout (0 for no timeout)"));
  g_key_file_set_string (kf, "General", "timeout_indicator", settings.to_indicator);
  set_comment(kf, "timeout_indicator",
      _(" Position of timeout indicator (top, bottom, left, right, none)"));
  g_key_file_set_boolean (kf, "General", "show_remain", settings.show_remain);
  set_comment(kf, "show_remain", _("Show remaining seconds in timeout indicator"));
  g_key_file_set_boolean (kf, "General", "combo_always_editable", settings.combo_always_editable);
  set_comment(kf, "combo_always_editable", _("Combo-box in entry dialog is always editable"));
  g_key_file_set_string (kf, "General", "terminal", settings.term);
  set_comment(kf, "terminal", _("Default terminal command (use %s for arguments placeholder)"));
  g_key_file_set_string (kf, "General", "open_command", settings.open_cmd);
  set_comment(kf, "open_command", _("Default open command (use %s for arguments placeholder)"));
  g_key_file_set_string (kf, "General", "date_format", settings.date_format);
  set_comment(kf, "date_format", _("Default date format (see strftime(3) for details)"));
  g_key_file_set_boolean (kf, "General", "ignore_unknown_options", settings.ignore_unknown);
  set_comment(kf, "ignore_unknown_options", _("Ignore unknown command-line options (false if debug)"));
  g_key_file_set_integer (kf, "General", "max_tab", settings.max_tab);
  set_comment(kf, "max_tab", _("Maximum number of notebook tabs"));
  g_key_file_set_boolean (kf, "General", "debug", settings.debug);
  set_comment(kf, "debug", _("Enable debug mode and warn about deprecated features"));
  g_key_file_set_boolean (kf, "General", "large_preview", settings.large_preview);
  set_comment(kf, "large_preview", _("Use large previews in file selection dialogs"));

  context = g_key_file_to_data (kf, NULL, NULL);

  g_key_file_free (kf);

  if (g_mkdir_with_parents (g_get_user_config_dir (), 0755) != -1)
    {
      gchar *filename = g_build_filename (g_get_user_config_dir (), YAD_SETTINGS_FILE, NULL);
      g_file_set_contents (filename, context, -1, NULL);
      g_free (filename);
    }
  else
    g_printerr (_("cannot write settings file: %s\n"), strerror (errno));

  g_free (context);
}

GdkPixbuf *
get_pixbuf (gchar *name, YadIconSize size, gboolean force)
{
  gint w, h;
  GdkPixbuf *pb = NULL;
  GError *err = NULL;

  if (size == YAD_BIG_ICON)
    gtk_icon_size_lookup (GTK_ICON_SIZE_DIALOG, &w, &h);
  else
    gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &w, &h);

  if (g_file_test (name, G_FILE_TEST_EXISTS))
    {
      pb = gdk_pixbuf_new_from_file (name, &err);
      if (!pb)
        {
          g_printerr ("yad: get_pixbuf(): %s\n", err->message);
          g_error_free (err);
        }
    }
  else
    pb = gtk_icon_theme_load_icon (settings.icon_theme, name, MIN (w, h), GTK_ICON_LOOKUP_GENERIC_FALLBACK, NULL);

  if (!pb)
    {
      if (size == YAD_BIG_ICON)
        pb = g_object_ref (settings.big_fallback_image);
      else
        pb = g_object_ref (settings.small_fallback_image);
    }

  /* force scaling image to specific size */
  if (!options.data.keep_icon_size && force && pb)
    {
      gint iw = gdk_pixbuf_get_width (pb);
      gint ih = gdk_pixbuf_get_height (pb);

      if (w != iw || h != ih)
        {
          GdkPixbuf *spb;
          spb = gdk_pixbuf_scale_simple (pb, w, h, GDK_INTERP_BILINEAR);
          g_object_unref (pb);
          pb = spb;
        }
    }

  return pb;
}

#if !GTK_CHECK_VERSION(3,0,0)
gchar *
get_color (GdkColor *c, guint64 alpha)
{
  gchar *res = NULL;

  /*
   * As observed GTK-2.24.32's color picker (GtkColorButton and gtk_color_button_get_color()):
   * - returns individual HEX channel values as 0xABAB where AB is the color value
   *   but picking from the wheel returns 0xABCD where AB is the color value and CD is garbage
   * - sometimes selecting from the wheel returns red's AB off by 1; work-around: pick from the wheel using the eyedropper
   */

  switch (options.color_data.mode)
    {
    case YAD_COLOR_HEX:
      if (options.color_data.alpha)
        res = g_strdup_printf ("#%02X%02X%02X%02X", (c->red >> 8u) & 0xFF, (c->green >> 8u) & 0xFF, (c->blue >> 8u) & 0xFF, (guint) ((alpha / 256) & 0xFF));
      else
        res = g_strdup_printf ("#%02X%02X%02X", (c->red >> 8u) & 0xFF, (c->green >> 8u) & 0xFF, (c->blue >> 8u) & 0xFF);
      break;
    case YAD_COLOR_RGB:
      if (options.color_data.alpha)
        res = g_strdup_printf ("rgba(%.1f, %.1f, %.1f, %.1f)", (double) c->red / 255.0, (double) c->green / 255.0,
                               (double) c->blue / 255.0, (double) alpha / 255 / 255);
      else
        res = g_strdup_printf ("rgb(%.1f, %.1f, %.1f)", (double) c->red / 255.0, (double) c->green / 255.0,
                               (double) c->blue / 255.0);
      break;
    }

  return res;
}
#else
gchar *
get_color (GdkRGBA *c)
{
  gchar *res = NULL;

  switch (options.color_data.mode)
    {
    case YAD_COLOR_HEX:
      if (options.color_data.alpha)
        res = g_strdup_printf ("#%02X%02X%02X%02X", (int) (c->red * 255), (int) (c->green * 255), (int) (c->blue * 255), (int) (c->alpha * 255));
      else
        res = g_strdup_printf ("#%02X%02X%02X", (int) (c->red * 255), (int) (c->green * 255), (int) (c->blue * 255));
      break;
    case YAD_COLOR_RGB:
      res = gdk_rgba_to_string (c);
      break;
    }
  return res;
}
#endif

void
update_preview (GtkFileChooser * chooser, GtkWidget *p)
{
  gchar *uri;
  static gchar *normal_path = NULL;
  static gchar *large_path = NULL;

  /* init thumbnails path */
  if (!normal_path)
    normal_path = g_build_filename (g_get_user_cache_dir (), "thumbnails", "normal", NULL);
  if (!large_path)
    large_path = g_build_filename (g_get_user_cache_dir (), "thumbnails", "large", NULL);

  /* load preview */
  uri = gtk_file_chooser_get_preview_uri (chooser);
  if (uri)
    {
      gchar *file;
      GChecksum *chs;
      GdkPixbuf *pb = NULL;

      chs = g_checksum_new (G_CHECKSUM_MD5);
      g_checksum_update (chs, (const guchar *) uri, -1);
      /* first try to get preview from large thumbnail */
      file = g_strdup_printf ("%s/%s.png", large_path, g_checksum_get_string (chs));
      if (options.common_data.large_preview && g_file_test (file, G_FILE_TEST_EXISTS))
        pb = gdk_pixbuf_new_from_file (file, NULL);
      else
        {
          /* try to get preview from normal thumbnail */
          g_free (file);
          file = g_strdup_printf ("%s/%s.png", normal_path, g_checksum_get_string (chs));
          if (!options.common_data.large_preview && g_file_test (file, G_FILE_TEST_EXISTS))
            pb = gdk_pixbuf_new_from_file (file, NULL);
          else
            {
              /* try to create it */
              g_free (file);
              file = g_filename_from_uri (uri, NULL, NULL);
              if (options.common_data.large_preview)
                pb = gdk_pixbuf_new_from_file_at_scale (file, 256, 256, TRUE, NULL);
              else
                pb = gdk_pixbuf_new_from_file_at_scale (file, 128, 128, TRUE, NULL);
              if (pb)
                {
                  struct stat st;
                  gchar *smtime;

                  stat (file, &st);
                  smtime = g_strdup_printf ("%d", st.st_mtime);
                  g_free (file);

                  /* save thumbnail */
                  if (options.common_data.large_preview)
                    {
                      g_mkdir_with_parents (large_path, 0755);
                      file = g_strdup_printf ("%s/%s.png", large_path, g_checksum_get_string (chs));
                    }
                  else
                    {
                      g_mkdir_with_parents (normal_path, 0755);
                      file = g_strdup_printf ("%s/%s.png", normal_path, g_checksum_get_string (chs));
                    }
                  gdk_pixbuf_save (pb, file, "png", NULL,
                                   "tEXt::Thumb::URI", uri,
                                   "tEXt::Thumb::MTime", smtime,
                                   NULL);
                  g_free (smtime);
                }
            }
        }
      g_checksum_free (chs);
      g_free (file);

      if (pb)
        {
          gtk_image_set_from_pixbuf (GTK_IMAGE (p), pb);
          g_object_unref (pb);
          gtk_file_chooser_set_preview_widget_active (chooser, TRUE);
        }
      else
        gtk_file_chooser_set_preview_widget_active (chooser, FALSE);

      g_free (uri);
    }
  else
    gtk_file_chooser_set_preview_widget_active (chooser, FALSE);
}

gchar **
split_arg (const gchar *str)
{
  gchar **res;
  gchar *p_col;

  res = g_new0 (gchar *, 3);

  p_col = g_strrstr (str, ":");
  if (p_col && p_col[1])
    {
      res[0] = g_strndup (str, p_col - str);
      res[1] = g_strdup (p_col + 1);
    }
  else
    res[0] = g_strdup (str);

  return res;
}

YadNTabs *
get_tabs (key_t key, gboolean create)
{
  YadNTabs *t = NULL;
  int shmid, i, max_tab;

  /* get shared memory */
  max_tab = settings.max_tab + 1;
  if (create)
    {
      if ((shmid = shmget (key, max_tab * sizeof (YadNTabs), IPC_CREAT | IPC_EXCL | 0644)) == -1)
        {
          g_printerr ("yad: cannot create shared memory for key %d: %s\n", key, strerror (errno));
          return NULL;
        }
    }
  else
    {
      if ((shmid = shmget (key, max_tab * sizeof (YadNTabs), 0)) == -1)
        {
          if (errno != ENOENT)
            g_printerr ("yad: cannot get shared memory for key %d: %s\n", key, strerror (errno));
          return NULL;
        }
    }

  /* attach shared memory */
  if ((t = shmat (shmid, NULL, 0)) == (YadNTabs *) -1)
    {
      g_printerr ("yad: cannot attach shared memory for key %d: %s\n", key, strerror (errno));
      return NULL;
    }

  /* initialize memory */
  if (create)
    {
      for (i = 1; i < max_tab; i++)
        {
          t[i].pid = -1;
          t[i].xid = 0;
        }
      t[0].pid = shmid;
      /* lastly, allow plugs to write shmem */
      t[0].xid = 1;
    }

  return t;
}

GtkWidget *
get_label (gchar *str, guint border, GtkWidget *w)
{
  GtkWidget *a, *t, *i, *l;
  GtkStockItem it;
  gchar **vals;
  gboolean b;

  if (!str || !*str)
    return gtk_label_new (NULL);

  l = i = NULL;

  SETUNDEPR (a, gtk_alignment_new, 0.5, 0.5, 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (a), border);

#if !GTK_CHECK_VERSION(3,0,0)
  t = gtk_hbox_new (FALSE, 0);
#else
  t = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
#endif
  gtk_container_add (GTK_CONTAINER (a), t);

  vals = g_strsplit_set (str, options.common_data.item_separator, 3);
  SETUNDEPR (b, gtk_stock_lookup, vals[0], &it);
  if (b)
    {
      l = gtk_label_new_with_mnemonic (it.label);
      SETUNDEPR (i, gtk_image_new_from_stock, it.stock_id, GTK_ICON_SIZE_BUTTON);
    }
  else
    {
      if (vals[0] && *vals[0])
        {
          l = gtk_label_new (NULL);
          if (!options.data.no_markup)
            gtk_label_set_markup_with_mnemonic (GTK_LABEL (l), vals[0]);
          else
            gtk_label_set_text_with_mnemonic (GTK_LABEL (l), vals[0]);
        }

      if (vals[1] && *vals[1])
        {
          SETUNDEPR (b, gtk_stock_lookup, vals[1], &it);
          if (b)
            {
              if (vals[0] && *vals[0])
                l = gtk_label_new_with_mnemonic (vals[0]);
              else if (!vals[0])
                l = gtk_label_new_with_mnemonic (it.label);
              SETUNDEPR (i, gtk_image_new_from_stock, it.stock_id, GTK_ICON_SIZE_BUTTON);
            }
          else
            i = gtk_image_new_from_pixbuf (get_pixbuf (vals[1], YAD_SMALL_ICON, TRUE));
        }
    }

  if (i)
    gtk_box_pack_start (GTK_BOX (t), i, FALSE, FALSE, 1);
  if (l)
    {
      if (w)
        gtk_label_set_mnemonic_widget (GTK_LABEL (l), w);
#if !GTK_CHECK_VERSION(3,0,0)
      gtk_misc_set_alignment (GTK_MISC (l), 0.0, 0.5);
#else
      gtk_label_set_xalign (GTK_LABEL (l), 0.0);
#endif
      gtk_box_pack_start (GTK_BOX (t), l, FALSE, FALSE, 1);
    }

  /* !!! must check both 1 and 2 values for !NULL */
  if (vals[1] && vals[2] && *vals[2])
    {
      if (!options.data.no_markup)
        gtk_widget_set_tooltip_markup (t, vals[2]);
      else
        gtk_widget_set_tooltip_text (t, vals[2]);
    }

  g_strfreev (vals);

  gtk_widget_show_all (a);

  return a;
}

gchar *
escape_str (gchar *str)
{
  gchar *res, *buf = str;
  guint i = 0, len;

  if (!str)
    return NULL;

  len = strlen (str);
  res = (gchar *) calloc (len * 2 + 1, sizeof (gchar));

  while (*buf)
    {
      switch (*buf)
        {
        case '\n':
          strcpy (res + i, "\\n");
          i += 2;
          break;
        case '\t':
          strcpy (res + i, "\\t");
          i += 2;
          break;
        case '\\':
          strcpy (res + i, "\\\\");
          i += 2;
          break;
        default:
          *(res + i) = *buf;
          i++;
          break;
        }
      buf++;
    }
  res[i] = '\0';

  return res;
}

gchar *
escape_char (gchar *str, gchar ch)
{
  gchar *res, *buf = str;
  guint i = 0, len;

  if (!str)
    return NULL;

  len = strlen (str);
  res = (gchar *) calloc (len * 2 + 1, sizeof (gchar));

  while (*buf)
    {
      if (*buf == ch)
        {
          strcpy (res + i, "\\\"");
          i += 2;
        }
      else
        {
          *(res + i) = *buf;
          i++;
        }
      buf++;
    }
  res[i] = '\0';

  return res;
}

gboolean
check_complete (GtkEntryCompletion *c, const gchar *key, GtkTreeIter *iter, gpointer data)
{
  gchar *value = NULL;
  GtkTreeModel *model = gtk_entry_completion_get_model (c);
  gboolean found = FALSE;

  if (!model || !key || !key[0])
    return FALSE;

  gtk_tree_model_get (model, iter, 0, &value, -1);

  if (value)
    {
      gchar **words = NULL;
      guint i = 0;

      switch (options.common_data.complete)
        {
        case YAD_COMPLETE_ANY:
          words = g_strsplit_set (key, " \t", -1);
          while (words[i])
            {
              if (strcasestr (value, words[i]) != NULL)
                {
                  /* found one of the words */
                  found = TRUE;
                  break;
                }
              i++;
            }
          break;
        case YAD_COMPLETE_ALL:
          words = g_strsplit_set (key, " \t", -1);
          found = TRUE;
          while (words[i])
            {
              if (strcasestr (value, words[i]) == NULL)
                {
                  /* not found one of the words */
                  found = FALSE;
                  break;
                }
              i++;
            }
          break;
        case YAD_COMPLETE_REGEX:
          found = g_regex_match_simple (key, value, G_REGEX_CASELESS | G_REGEX_OPTIMIZE, G_REGEX_MATCH_NOTEMPTY);
          break;
        default: ;
        }

      if (words)
        g_strfreev (words);
    }

  return found;
}

void
parse_geometry ()
{
  gchar *geom, *ptr;
  gint w = -1, h = -1, x = 0, y = 0;
  gboolean usexy = FALSE;
  guint i = 0;

  if (!options.data.geometry)
    return;

  geom = options.data.geometry;

  if (geom[i] != '+' && geom[i] != '-')
    {
      ptr = geom + i;
      w = atoi (ptr);

      while (geom[i] && geom[i] != 'x') i++;

      if (!geom[i])
        return;

      ptr = geom + i + 1;
      h = atoi (ptr);

      while (geom[i] && geom[i] != '-' && geom[i] != '+') i++;
    }

  if (geom[i])
    {
      usexy = TRUE;

      ptr = geom + i;
      x = atoi (ptr);

      i++;
      while (geom[i] && geom[i] != '-' && geom[i] != '+') i++;

      if (!geom[i])
        return;

      ptr = geom + i;
      y = atoi (ptr);
    }

  if (w != -1)
    options.data.width = w;
  if (h != -1)
    options.data.height = h;
  options.data.posx = x;
  options.data.posy = y;
  options.data.use_posx = options.data.use_posy = usexy;
}

gboolean
get_bool_val (gchar *str)
{
  if (!str || !str[0])
    return FALSE;

  switch (str[0])
    {
    case '1':
    case 't':
    case 'T':
    case 'y':
    case 'Y':
      if (strcasecmp (str, "t") == 0 || strcasecmp (str, "true") == 0 ||
          strcasecmp (str, "y") == 0 || strcasecmp (str, "yes") == 0 ||
          strcmp (str, "1") == 0)
        return TRUE;
      break;
    case '0':
    case 'f':
    case 'F':
    case 'n':
    case 'N':
      if (strcasecmp (str, "f") == 0 || strcasecmp (str, "false") == 0 ||
          strcasecmp (str, "n") == 0 || strcasecmp (str, "no") == 0 ||
          strcmp (str, "0") == 0)
        return FALSE;
      break;
    case 'o':
    case 'O':
      if (strcasecmp (str, "on") == 0)
        return TRUE;
      else if (strcasecmp (str, "off") == 0)
        return FALSE;
      break;
    default: ;
    }

 g_printerr ("yad: wrong boolean value '%s'\n", str);
 return FALSE;
}

gchar *
print_bool_val (gboolean val)
{
  gchar *ret = "";

  switch (options.common_data.bool_fmt)
    {
    case YAD_BOOL_FMT_UT:
      ret = val ? "TRUE" : "FALSE";
      break;
    case YAD_BOOL_FMT_UY:
      ret = val ? "YES" : "NO";
      break;
    case YAD_BOOL_FMT_UO:
      ret = val ? "ON" : "OFF";
      break;
    case YAD_BOOL_FMT_LT:
      ret = val ? "true" : "false";
      break;
    case YAD_BOOL_FMT_LY:
      ret = val ? "yes" : "no";
      break;
    case YAD_BOOL_FMT_LO:
      ret = val ? "on" : "off";
      break;
    case YAD_BOOL_FMT_1:
      ret = val ? "1" : "0";
      break;
    }

  return ret;
}

typedef struct {
  gchar *cmd;
  gchar **out;
  gint ret;
  gboolean lock;
} RunData;

static void
run_thread (RunData *d)
{
  GError *err = NULL;

  if (!g_spawn_command_line_sync (d->cmd, d->out, NULL, NULL, &err))
    {
      if (options.debug)
        g_printerr (_("WARNING: Run command failed: %s\n"), err->message);
      g_error_free (err);
      d->ret = -1;
    }
  d->lock = FALSE;
}

gint
run_command_sync (gchar *cmd, gchar **out, GtkWidget *w)
{
  RunData *d;
  gint ret;

  d = g_new0 (RunData, 1);

  if (options.data.use_interp)
    {
      if (g_strstr_len (options.data.interp, -1, "%s") != NULL)
        d->cmd = g_strdup_printf (options.data.interp, cmd);
      else
        d->cmd = g_strdup_printf ("%s %s", options.data.interp, cmd);
    }
  else
    d->cmd = g_strdup (cmd);
  d->out = out;

  if (w)
    gtk_widget_set_sensitive (w, FALSE);

  d->lock = TRUE;
  g_thread_new ("run_sync", (GThreadFunc) run_thread, d);

  while (d->lock != FALSE)
    {
      gtk_main_iteration_do (FALSE);
      usleep (10000);
    }

  ret = d->ret;
  if (w)
      gtk_widget_set_sensitive (w, TRUE);

  g_free (d->cmd);
  g_free (d);

  return ret;
}

void
run_command_async (gchar *cmd)
{
  gchar *full_cmd = NULL;
  GError *err = NULL;

  if (options.data.use_interp)
    {
      if (g_strstr_len (options.data.interp, -1, "%s") != NULL)
        full_cmd = g_strdup_printf (options.data.interp, cmd);
      else
        full_cmd = g_strdup_printf ("%s %s", options.data.interp, cmd);
    }
  else
    full_cmd = g_strdup (cmd);

  if (!g_spawn_command_line_async (full_cmd, &err))
    {
      if (options.debug)
        g_printerr (_("WARNING: Run command failed: %s\n"), err->message);
      g_error_free (err);
    }

  g_free (full_cmd);
}

#if GTK_CHECK_VERSION(3,0,0)
gchar *
pango_to_css (gchar *font)
{
  PangoFontDescription *desc;
  PangoFontMask mask;
  GString *str;
  gchar *res;

  str = g_string_new (NULL);

  desc = pango_font_description_from_string (font);
  mask = pango_font_description_get_set_fields (desc);

  if (mask & PANGO_FONT_MASK_STYLE)
    {
      switch (pango_font_description_get_style (desc))
        {
        case PANGO_STYLE_OBLIQUE:
          g_string_append (str, "oblique ");
          break;
        case PANGO_STYLE_ITALIC:
          g_string_append (str, "italic ");
          break;
        default: ;
        }
    }
  if (mask & PANGO_FONT_MASK_VARIANT)
    {
      if (pango_font_description_get_variant (desc) == PANGO_VARIANT_SMALL_CAPS)
        g_string_append (str, "small-caps ");
    }

  if (mask & PANGO_FONT_MASK_WEIGHT)
    {
      switch (pango_font_description_get_weight (desc))
        {
        case PANGO_WEIGHT_THIN:
          g_string_append (str, "Thin ");
          break;
        case PANGO_WEIGHT_ULTRALIGHT:
          g_string_append (str, "Ultralight ");
          break;
        case PANGO_WEIGHT_LIGHT:
          g_string_append (str, "Light ");
          break;
        case PANGO_WEIGHT_SEMILIGHT:
          g_string_append (str, "Semilight ");
          break;
        case PANGO_WEIGHT_BOOK:
          g_string_append (str, "Book ");
          break;
        case PANGO_WEIGHT_MEDIUM:
          g_string_append (str, "Medium ");
          break;
        case PANGO_WEIGHT_SEMIBOLD:
          g_string_append (str, "Semibold ");
          break;
        case PANGO_WEIGHT_BOLD:
          g_string_append (str, "Bold ");
          break;
        case PANGO_WEIGHT_ULTRABOLD:
          g_string_append (str, "Ultrabold ");
          break;
        case PANGO_WEIGHT_HEAVY:
          g_string_append (str, "Heavy ");
          break;
        case PANGO_WEIGHT_ULTRAHEAVY:
          g_string_append (str, "Ultraheavy ");
          break;
        default: ;
        }
    }

  if (mask & PANGO_FONT_MASK_SIZE)
    {
      if (pango_font_description_get_size_is_absolute (desc))
        g_string_append_printf (str, "%dpx ", pango_font_description_get_size (desc) / PANGO_SCALE);
      else
        g_string_append_printf (str, "%dpt ", pango_font_description_get_size (desc) / PANGO_SCALE);
    }

  if (mask & PANGO_FONT_MASK_FAMILY)
    g_string_append (str, pango_font_description_get_family (desc));

  if (str->str)
    res = str->str;
  else
    res = g_strdup (font);

  return res;
}
#endif

void
open_uri (const gchar *uri)
{
  gchar *cmdline;

  if (!uri || !uri[0])
    return;

  if (g_strstr_len (options.data.uri_handler, -1, "%s") != NULL)
    cmdline = g_strdup_printf (options.data.uri_handler, uri);
  else
    cmdline = g_strdup_printf ("%s '%s'", options.data.uri_handler, uri);
  run_command_async (cmdline);
  g_free (cmdline);
}
