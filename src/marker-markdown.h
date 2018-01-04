#ifndef __MARKER_MARKDOWN_H__
#define __MARKER_MARKDOWN_H__

#include <stddef.h>

char*
marker_markdown_to_html(const char* markdown,
                        size_t      size,
                        const char* stylesheet_location);
                     
char*   
marker_markdown_to_html_with_css_inline(const char* markdown,
                                        size_t      size,
                                        const char* stylesheet_location);

void
marker_markdown_to_html_file(const char* markdown,
                             size_t      size,
                             const char* stylesheet_location,
                             const char* filepath);

void
marker_markdown_to_html_file_with_css_inline(const char* markdown,
                                             size_t      size,
                                             const char* stylesheet_location,
                                             const char* filepath);

#endif
