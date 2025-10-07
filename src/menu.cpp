#pragma once
#include "menu.hpp"
#include "raygui.h"
#include "raylib.h"
#include "scene.hpp"
#include "state.hpp"
#include "web.h"
#include <cstring>
#include <memory>
#include <sys/types.h>
#if defined(PLATFORM_WEB)
#include <emscripten/html5.h>
#endif

Menu::Menu(const char *title)
    : m_title(title), m_font(App::getInstance().getDefaultFont()),
      cells({false}) {}

void Menu::init() {
  // set labels, other text, maybe theme
  //

  GuiSetFont(App::getInstance().getDefaultFont());
  GuiSetStyle(DEFAULT, TEXT_SIZE, 30);
  initCells(cells);
// GuiSetState(STATE_FOCUSED);
#if defined(PLATFORM_WEB)
  toggle_console_wrapper();

#endif
}

void Menu::draw(IVector2 *resolution) {
  // Draw sprites
#if defined(PLATFORM_WEB)
  emscripten_get_canvas_element_size("#canvas", &resolution->x, &resolution->y);
#elif defined(PLATFORM_DESKTOP)
  resolution->x = GetScreenWidth();
  resolution->y = GetScreenHeight();
#endif

  BeginDrawing();
  drawCells(resolution);

  ClearBackground({41, 41, 41, 100});
  Vector2 textSize = MeasureTextEx(m_font, m_title, 80, 2);
  DrawTextEx(m_font, m_title, {(resolution->x - textSize.x) / 2, 200.0f}, 80.0f,
             2.0f, LIGHTGRAY);

  Menu::DrawUI(*resolution);
  EndDrawing();
}

void Menu::DrawUI(IVector2 resolution) {
  // Draw raylib menus, buttons, labels
  if (GuiButton((Rectangle){static_cast<float>((resolution.x - 320) / 2),
                            static_cast<float>(resolution.y - 500), 320, 130},
                "Start"))
    startScene = true;
  // #ifndef PLATFORM_WEB
  if (GuiButton((Rectangle){static_cast<float>((resolution.x - 320) / 2),
                            static_cast<float>(resolution.y - 300), 320, 130},
                "Quit")) {
    App::getInstance().setState(AV::QUIT);
  }
  // #endif
}

void Menu::update() {
  // handle window resize, reposition elements
  // and handle updates from the UI
  static double lastUpdateTime = 0;
  double currentTime = GetTime();

  if (currentTime - lastUpdateTime >= 0.1) { // 0.1 seconds = 10 FPS
    calcGOL(nullptr);
    lastUpdateTime = currentTime;
  }

  if (IsKeyPressed(KEY_SPACE)) {
    calcGOL(nullptr);
  }
  if (startScene) {
    App::getInstance().setState(std::make_unique<AV::Scene>(&m_font));
  }
}

void Menu::input() {
#if defined(PLATFORM_WEB)
  if (IsKeyPressed(KEY_C)) {
    toggle_console_wrapper();
  }
#endif
}

void Menu::drawCells(IVector2 *resolution) {

  float cellWidth = (float)resolution->x / C_CELLS;
  float cellHeight = (float)resolution->y / R_CELLS;

  for (int r = 0; r < R_CELLS; r++) {
    for (int c = 0; c < C_CELLS; c++) {
      if (cells[r * C_CELLS + c]) {
        DrawRectangleV({c * cellWidth, r * cellHeight}, {cellWidth, cellHeight},
                       ORANGE);
      }
    }
  }
}

void Menu::calcGOL(IVector2 *resolution) {
  bool nextGen[R_CELLS * C_CELLS] = {false};

  for (int r = 0; r < R_CELLS; r++) {
    for (int c = 0; c < C_CELLS; c++) {
      int liveNeighbors = 0;

      // Check all 8 neighbors with toroidal wrapping (edges wrap around)
      for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
          if (dr == 0 && dc == 0)
            continue;

          int nr = (r + dr + R_CELLS) % R_CELLS;
          int nc = (c + dc + C_CELLS) % C_CELLS;

          if (cells[nr * C_CELLS + nc]) {
            liveNeighbors++;
          }
        }
      }

      int index = r * C_CELLS + c;
      bool currentState = cells[index];

      if (currentState) {
        nextGen[index] = (liveNeighbors == 2 || liveNeighbors == 3);
      } else {
        nextGen[index] = (liveNeighbors == 3);
      }
    }
  }

  memcpy(cells, nextGen, sizeof(cells));
}
