/*
 * marker-exporter.h
 *
 * Copyright (C) 2017 - 2018 Fabio Colacio
 *
 * Marker is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * Marker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __MARKER_EXPORTER_H__
#define __MARKER_EXPORTER_H__

#include <gtk/gtk.h>

typedef enum
{
  HTML, PDF, RTF, ODT, DOCX, LATEX
} MarkerExportFormat;

void
marker_exporter_show_export_dialog(MarkerEditorWindow* window);

void
marker_exporter_export(const char*        markdown,
                       const char*        stylesheet,
                       const char*        outfile,
                       MarkerExportFormat format);

#endif

