#pragma once
#include "raygui.h"
#include "raylib.h"
#include "state.hpp"
#include <memory>

class App {
  static App *instance;
  IVector2 resolution;
  Font g_font;
  AV::AppState appState;
  std::unique_ptr<AV::State> current_state;
  App() {}
  App(int width, int height, const char *font = "");

  App(const App &) = delete;
  App &operator=(const App &) = delete;

public:
  static App &createInstance(int width, int height);
  static App &getInstance();

#if defined(PLATFORM_WEB)
  void initWeb();
#endif
  void setState(std::unique_ptr<AV::State> state);
  void run(void);
  static void runWrapper();
};
