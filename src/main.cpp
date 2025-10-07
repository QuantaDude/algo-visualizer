#include "utils.h"
#if defined(PLATFORM_WEB)
#include <emscripten/html5.h>
#endif
#define RAYGUI_IMPLEMENTATION

#include "app.hpp"
int main() {
  int width{1920}, height{1080};

#if defined(PLATFORM_WEB)
  set_canvas_size_wrapper(&width, &height);
  emscripten_get_canvas_element_size("#canvas", &width, &height);
#endif

  App *instance = App::createInstance(width, height);
  if (!instance) {
    TRACELOG(LOG_FATAL, "Failed to create window. Exiting");
    return -1;
  }
#if defined(PLATFORM_WEB)
  instance->initWeb();
#else
  SetTargetFPS(60);

  // Main game loop
  while (!WindowShouldClose()) // Detect window close button or ESC key
  {
    instance->run();
  }
#endif
  return 0;
}
