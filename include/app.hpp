#pragma once
#include "menu.hpp"
#include "raygui.h"
#include "raylib.h"
#include "state.hpp"
#include <memory>
#if defined(PLATFORM_WEB)
#include "web.h"
#endif

class App {
  static App *instance;
  IVector2 resolution;
  Font g_font;
  static AV::AppState g_app_state;
  std::unique_ptr<AV::State> current_state;
  App() {}
  App(int width, int height, const char *font = "");

  App(const App &) = delete;
  App &operator=(const App &) = delete;

public:
  static App *createInstance(int width, int height);
  static App &getInstance();

  void shutdown();
  Font &getDefaultFont();

#if defined(PLATFORM_WEB)
  void initWeb();
#endif

  void setState(std::unique_ptr<AV::State> state);
  void setState(AV::AppState);
  void run(void);
  static void runWrapper();
};
