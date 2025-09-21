#include "utils.h"
#define RAYGUI_IMPLEMENTATION

#include "app.hpp"
int main() {

  App *instance = App::createInstance(1920, 1080);
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
