
#include "web.h"
#include <emscripten.h> // Required for EM_JS macros
#include <emscripten/console.h>
#include <emscripten/em_js.h>
#include <emscripten/em_macros.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

// Toggle console visibility
EM_JS(void, toggle_console, (void), {
  var output = document.getElementById("output");
  output.hidden = !output.hidden;
});

// Set canvas size to full window and return area
EM_JS(int, canvas_set_size, (void), {
  var canvas = document.getElementById("canvas");
  canvas.width = window.innerWidth;
  canvas.height = window.innerHeight;
  return window.innerWidth * window.innerHeight;
});

// Print a float value
EM_JS(void, print_float, (float val), { Module.print(val); });

// Print a UTF-8 string
EM_JS(void, print, (const char *str), { Module.print(UTF8ToString(str)); });

void call_canvas() { canvas_set_size(); }

void print_console(const char *str) { print(str); };
