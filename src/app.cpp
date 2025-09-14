#pragma once
#include "app.hpp"
#include "menu.hpp"
#include "state.hpp"
#include <memory>
#include <utility>

App *App::instance = nullptr;
App &App::createInstance(int width, int height) {
  if (instance == nullptr) {
    instance = new App(width, height,
                       RESOURCES_PATH "./resources/font/alpha_beta.png");
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    instance->setState(std::make_unique<Menu>("MENU"));
#if defined(PLATFORM_WEB)
    canvas_set_size();

    emscripten_get_canvas_element_size("#canvas", &width, &height);
#endif

    InitWindow(width, height, "Algorithm Visualizer");
    GuiLoadStyle(RESOURCES_PATH "style_amber.rgs");
  }
  return *instance;
}

App::App(int width, int height, const char *font)
    : resolution({.x = width, .y = height}), g_font(LoadFont(font)),
      appState(AV::AppState::MENU) {
  // createInstance(width, height);
}

App &App::getInstance() {
  if (instance == nullptr) {
    instance = new App(1280, 720);
  }
  return *instance;
}

void App::setState(std::unique_ptr<AV::State> state) {
  current_state = std::move(state);
  current_state->Init();
}

void App::run(void) {

  if (current_state) {
    current_state->Update();
    current_state->Draw(&resolution);
  }
}
void App::runWrapper() { getInstance().run(); }

#if defined(PLATFORM_WEB)
void App::initWeb() {
  emscripten_get_canvas_element_size("#canvas", &resolution.x, &resolution.y);

  emscripten_set_main_loop(runWrapper, 0, 1);
}
#endif
