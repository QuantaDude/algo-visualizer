#pragma once
#include "menu.hpp"
#include "app.hpp"
#include "raylib.h"

void Menu::Init() {
  // set labels, other text, maybe theme
}

void Menu::Draw(IVector2 *resolution) {
  // Draw sprites
#if defined(PLATFORM_WEB)
  emscripten_get_canvas_element_size("#canvas", &resolution->x, &resolution->y);
#elif defined(PLATFORM_DESKTOP)
  resolution->x = GetScreenWidth();
  resolution->y = GetScreenHeight();
#endif

  BeginDrawing();

  ClearBackground(RAYWHITE);
  Vector2 textSize =
      MeasureTextEx(GetFontDefault(), "Algorithm Visualizer", 30, 2);
  DrawTextEx(
      GetFontDefault(), "Algorithm Visualizer",
      {resolution->x / 2.0f - (textSize.x / 2), 20.0f - (textSize.y / 2)},
      30.0f, 2.0f, LIGHTGRAY);

  Menu::DrawUI(*resolution);
  EndDrawing();
}

void Menu::DrawUI(IVector2 resolution) {
  GuiSetStyle(DEFAULT, TEXT_SIZE, 30);
  // Draw raylib menus, buttons, labels
  if (GuiButton((Rectangle){static_cast<float>((resolution.x / 2) - 60),
                            static_cast<float>(resolution.y - 500), 320, 130},
                "Start"))
    startScene = true;

  if (GuiButton((Rectangle){static_cast<float>((resolution.x / 2) - 60),
                            static_cast<float>(resolution.y - 300), 320, 130},
                "Quit")) {
  }
}

void Menu::Update() {
  // handle window resize, reposition elements
  // and handle updates from the UI
  if (startScene) {
  }
}

void Menu::Input() {}
