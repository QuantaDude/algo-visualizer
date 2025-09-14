#include <emscripten/console.h>
#include <emscripten/em_js.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#ifdef __cplusplus
extern "C" {
#endif
extern void toggle_console();
extern float canvas_set_size();
extern void print_float(float string);
extern void print(const char *string);

#ifdef __cplusplus
}
#endif
