#ifndef WEB_H
#define WEB_H

// extern "C" {
//}
#ifdef __cplusplus
extern "C" {
#endif
void toggle_console(void);
int canvas_set_size(void);
void print_float(float string);
void print(const char *);
void call_canvas(void);
void print_console(const char *);
#ifdef __cplusplus
}
#endif

#endif
