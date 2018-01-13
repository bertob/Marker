/*
 * marker-preview.c
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

#include <string.h>
#include <stdlib.h>

#include <glib.h>
#include <time.h>

#include "marker-markdown.h"
#include "marker-prefs.h"

#include "marker-preview.h"
#include "marker.h"

#define MAX_ZOOM  4.0
#define MIN_ZOOM  0.1

#define min(a, b) ((a < b) ? a : b)
#define max(a, b) ((a < b) ? b : a)

struct _MarkerPreview
{
  WebKitWebView parent_instance;
};

G_DEFINE_TYPE(MarkerPreview, marker_preview, WEBKIT_TYPE_WEB_VIEW)

static void
open_uri (WebKitResponsePolicyDecision *decision) {
  const gchar * uri = webkit_uri_request_get_uri(webkit_response_policy_decision_get_request(decision));
  GtkApplication * app = marker_get_app();
  GList* windows = gtk_application_get_windows(app);
  time_t now = time(0);
  gtk_show_uri_on_window (windows->data, uri, now, NULL);
}

static gboolean
decide_policy_cb (WebKitWebView *web_view,
                  WebKitPolicyDecision *decision,
                  WebKitPolicyDecisionType type)
{
    switch (type) {
    case WEBKIT_POLICY_DECISION_TYPE_RESPONSE:
        /* ignore default policy and open uri in default browser*/
        open_uri((WebKitResponsePolicyDecision*)decision);
        webkit_policy_decision_ignore(decision);
        break;
    default:
        /* Making no decision results in webkit_policy_decision_use(). */
        return FALSE;
    }
    return TRUE;
}


static gboolean
context_menu_cb  (WebKitWebView       *web_view,
                  WebKitContextMenu   *context_menu,
                  GdkEvent            *event,
                  WebKitHitTestResult *hit_test_result,
                  gpointer             user_data)
{
  return TRUE;
}

static void
initialize_web_extensions_cb (WebKitWebContext *context,
                              gpointer          user_data)
{
  /* Web Extensions get a different ID for each Web Process */
  static guint32 unique_id = 0;

  webkit_web_context_set_web_extensions_directory (
     context, WEB_EXTENSIONS_DIRECTORY);
  webkit_web_context_set_web_extensions_initialization_user_data (
     context, g_variant_new_uint32 (unique_id++));
}

gboolean
key_press_event_cb (GtkWidget *widget,
                    GdkEvent  *event,
                    gpointer   user_data)
{
  g_return_val_if_fail (MARKER_IS_PREVIEW (widget), FALSE);
  MarkerPreview *preview = MARKER_PREVIEW (widget);

  GdkEventKey *key_event = (GdkEventKey *) event;

  if ((key_event->state & GDK_CONTROL_MASK) != 0)
  {
    switch (key_event->keyval)
    {
      case GDK_KEY_plus:
        marker_preview_zoom_in (preview);
        break;
      
      case GDK_KEY_minus:
        marker_preview_zoom_out (preview);
        break;
      
      case GDK_KEY_0:
        marker_preview_zoom_original (preview);
        break;
    }
  }
  
  return FALSE;
}

gboolean
scroll_event_cb (GtkWidget *widget,
                 GdkEvent  *event,
                 gpointer   user_data)
{
  g_return_val_if_fail (MARKER_IS_PREVIEW (widget), FALSE);
  MarkerPreview *preview = MARKER_PREVIEW (widget);
  
  GdkEventScroll *scroll_event = (GdkEventScroll *) event;
  
  guint state = scroll_event->state;
  if ((state & GDK_CONTROL_MASK) != 0)
  {
    gdouble delta_y = scroll_event->delta_y;
    
    if (delta_y > 0)
    {
      marker_preview_zoom_out (preview);
    }
    else if (delta_y < 0)
    {
      marker_preview_zoom_in (preview);
    }
  }
  
  return FALSE;
}

static void
load_changed_cb (WebKitWebView   *preview,
                 WebKitLoadEvent  event)
{
  switch (event)
  {
    case WEBKIT_LOAD_STARTED:
      break;
      
    case WEBKIT_LOAD_REDIRECTED:
      break;
      
    case WEBKIT_LOAD_COMMITTED:
      break;
      
    case WEBKIT_LOAD_FINISHED:
      break;
  }
}

static void
pdf_print_failed_cb (WebKitPrintOperation* print_op,
                     GError*               err,
                     gpointer              user_data)
{
  g_printerr("print failed with error: %s\n", err->message);
}

void
marker_preview_set_zoom_level (MarkerPreview *preview,
                               gdouble        zoom_level)
{
  g_return_if_fail (MARKER_IS_PREVIEW (preview));
  webkit_web_view_set_zoom_level (WEBKIT_WEB_VIEW (preview), zoom_level);
  g_signal_emit_by_name (preview, "zoom-changed");
}

static void
marker_preview_init (MarkerPreview *preview)
{
  g_signal_connect (webkit_web_context_get_default (),
                    "initialize-web-extensions",
                    G_CALLBACK (initialize_web_extensions_cb),
                    NULL);

  g_signal_connect (preview, "scroll-event", G_CALLBACK (scroll_event_cb), NULL);
  g_signal_connect (preview, "key-press-event", G_CALLBACK (key_press_event_cb), NULL);
}

static void
marker_preview_class_init (MarkerPreviewClass *class)
{
  g_signal_newv ("zoom-changed",
                 G_TYPE_FROM_CLASS (class),
                 G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
                 NULL, NULL, NULL, NULL,
                 G_TYPE_NONE, 0, NULL);
                 

  WEBKIT_WEB_VIEW_CLASS(class)->load_changed = load_changed_cb;
}

MarkerPreview*
marker_preview_new(void)
{
  MarkerPreview * obj =  g_object_new(MARKER_TYPE_PREVIEW, NULL); 
  
  webkit_web_view_set_zoom_level (WEBKIT_WEB_VIEW (obj), makrer_prefs_get_zoom_level ());
  
  return obj;
}

void
marker_preview_zoom_out (MarkerPreview *preview)
{
  g_return_if_fail (WEBKIT_IS_WEB_VIEW (preview));
  WebKitWebView *view = WEBKIT_WEB_VIEW (preview);
  
  gdouble val = webkit_web_view_get_zoom_level (view) - 0.1;
  val = max (val, MIN_ZOOM);
  
  marker_prefs_set_zoom_level(val);
  webkit_web_view_set_zoom_level(view, val);
  
  g_signal_emit_by_name (preview, "zoom-changed");
}

void
marker_preview_zoom_original (MarkerPreview *preview)
{
  g_return_if_fail (WEBKIT_IS_WEB_VIEW (preview));
  WebKitWebView *view = WEBKIT_WEB_VIEW (preview);
  
  gdouble zoom = 1.0;
  
  marker_prefs_set_zoom_level (zoom);
  webkit_web_view_set_zoom_level (view, zoom);
  
  g_signal_emit_by_name (preview, "zoom-changed");
}

void
marker_preview_zoom_in (MarkerPreview *preview)
{
  g_return_if_fail (WEBKIT_IS_WEB_VIEW (preview));
  WebKitWebView *view = WEBKIT_WEB_VIEW (preview);
  
  gdouble val = webkit_web_view_get_zoom_level (view) + 0.1;
  val = min (val, MAX_ZOOM);
  
  marker_prefs_set_zoom_level(val);
  webkit_web_view_set_zoom_level(view, val);
  
  g_signal_emit_by_name (preview, "zoom-changed");
}

void
marker_preview_render_markdown(MarkerPreview* preview,
                               const char*    markdown,
                               const char*    css_theme,
                               const char*    base_uri)
{
  MarkerKaTeXMode katex_mode = KATEX_OFF;
  if (marker_prefs_get_use_katex()) {
    katex_mode = KATEX_LOCAL;
  }
  MarkerHighlightMode highlight_mode = HIGHLIGHT_OFF;
  if (marker_prefs_get_use_highlight()){
    highlight_mode = HIGHLIGHT_LOCAL;
  }
  MarkerMermaidMode mermaid_mode = MERMAID_OFF;
  if (marker_prefs_get_use_mermaid())
  {
    mermaid_mode = MERMAID_LOCAL;
  }


  char* html = marker_markdown_to_html(markdown, 
                                       strlen(markdown), 
                                       katex_mode, 
                                       highlight_mode,
                                       mermaid_mode,
                                       css_theme);
                                       
  const char* uri = (base_uri) ? base_uri : "file://unnamed.md";
  WebKitWebView* web_view = WEBKIT_WEB_VIEW(preview);

  g_signal_connect(web_view,
                   "decide-policy",
                   G_CALLBACK(decide_policy_cb),
                   NULL);
  g_signal_connect(web_view,
                   "context-menu",
                   G_CALLBACK(context_menu_cb),
                   NULL);
  webkit_web_view_load_html(web_view, html, uri);
  free(html);
}

WebKitPrintOperationResponse
marker_preview_run_print_dialog(MarkerPreview* preview,
                                GtkWindow*     parent)
{
  WebKitPrintOperation* print_op =
    webkit_print_operation_new(WEBKIT_WEB_VIEW(preview));
  
  g_signal_connect(print_op, "failed", G_CALLBACK(pdf_print_failed_cb), NULL);
  
  return webkit_print_operation_run_dialog(print_op, parent);
}

void
marker_preview_print_pdf(MarkerPreview* preview,
                         const char*    outfile)

{
    WebKitPrintOperation* print_op = NULL;
    GtkPrintSettings* print_s = NULL;
    char* uri = g_strdup_printf("file://%s", outfile);
    
    print_op = webkit_print_operation_new(WEBKIT_WEB_VIEW(preview));
    g_signal_connect(print_op, "failed", G_CALLBACK(pdf_print_failed_cb), NULL);
    
    print_s = gtk_print_settings_new();
    gtk_print_settings_set(print_s, GTK_PRINT_SETTINGS_OUTPUT_FILE_FORMAT, "pdf");
    gtk_print_settings_set(print_s, GTK_PRINT_SETTINGS_OUTPUT_URI, uri);
    gtk_print_settings_set(print_s, GTK_PRINT_SETTINGS_PRINTER, "Print to File");
    webkit_print_operation_set_print_settings(print_op, print_s);
    
    webkit_print_operation_print(print_op);

    g_free(uri);
}
