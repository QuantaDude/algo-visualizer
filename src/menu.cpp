#pragma once
#include "menu.hpp"
#include "raygui.h"
#include "raylib.h"
#if defined(PLATFORM_WEB)
#include <emscripten/html5.h>
#endif

Menu::Menu(const char *title)
    : m_title(title), m_font(App::getInstance().getDefaultFont()) {}

void Menu::Init() {
  // set labels, other text, maybe theme

  GuiSetFont(App::getInstance().getDefaultFont());
  GuiSetStyle(DEFAULT, TEXT_SIZE, 30);
  // GuiSetState(STATE_FOCUSED);
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

  ClearBackground({41, 41, 41, 100});
  Vector2 textSize = MeasureTextEx(m_font, m_title, 30, 2);
  DrawTextEx(
      m_font, m_title,
      {resolution->x / 2.0f - (textSize.x / 2), 20.0f - (textSize.y / 2)},
      30.0f, 2.0f, LIGHTGRAY);

  Menu::DrawUI(*resolution);
  EndDrawing();
}

void Menu::DrawUI(IVector2 resolution) {
  // Draw raylib menus, buttons, labels
  if (GuiButton((Rectangle){static_cast<float>((resolution.x / 2) - 60),
                            static_cast<float>(resolution.y - 500), 320, 130},
                "Start"))
    startScene = true;
  // #ifndef PLATFORM_WEB
  if (GuiButton((Rectangle){static_cast<float>((resolution.x / 2) - 60),
                            static_cast<float>(resolution.y - 300), 320, 130},
                "Quit")) {
    App::getInstance().setState(AV::QUIT);
  }
  // #endif
}

void Menu::Update() {
  // handle window resize, reposition elements
  // and handle updates from the UI
  if (startScene) {
  }
}

void Menu::Input() {}
