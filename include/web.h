#ifndef WEB_H
#define WEB_H

// extern "C" {
//}
#include <emscripten/em_types.h>
#ifdef __cplusplus
extern "C" {
#endif
void toggle_console(void);
int canvas_set_size(void);
void close_window(void);
void print_float(float string);
void print(const char *);
char *list_all_files(const char *);

void call_canvas(void);
void print_console(const char *);
void close_window_wrapper();
em_str_callback_func onPreloadSuccess(const char *);
em_str_callback_func onPreloadError(const char *);

void list_files();

#ifdef __cplusplus
}
#endif

#endif
