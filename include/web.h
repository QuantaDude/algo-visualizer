#ifndef WEB_H
#define WEB_H

// extern "C" {
//}
#if defined(PLATFORM_WEB)
#include <emscripten/em_types.h>
#ifdef __cplusplus
extern "C" {
#endif
void toggle_console(void);
void canvas_set_size(int *width, int *height);
void close_window(void);
void print_float(float string);
void print(const char *);
char *list_all_files(const char *);

void toggle_console_wrapper(void);
void set_canvas_size_wrapper(int *, int *);
void print_console(const char *);
void print_console_float(float);
void close_window_wrapper();
em_str_callback_func onPreloadSuccess(const char *);
em_str_callback_func onPreloadError(const char *);

void list_files();

#ifdef __cplusplus
}
#endif

#endif
#endif
