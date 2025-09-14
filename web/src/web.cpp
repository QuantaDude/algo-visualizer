#include "web.hpp"

EM_JS(void, toggle_console, (), {
  var output = document.getElementById("output");
  output.hidden = !output.hidden;
});
EM_JS(int, canvas_set_size, (), {
  var canvas = document.getElementById("canvas");
  canvas.width = window.innerWidth;
  canvas.height = window.innerHeight;
  return window.innerWidth * window.innerHeight;
});
EM_JS(void, print_float, (float string), { Module.print(string); });

EM_JS(void, print, (const char *string),
      { Module.print(UTF8ToString(string)); });
