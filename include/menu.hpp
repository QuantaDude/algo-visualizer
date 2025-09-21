#pragma once
#include "app.hpp"
#include "raylib.h"
#include "state.hpp"

class Menu : public AV::State {
  const char *m_title;
  bool startScene = false;
  Font &m_font;

public:
  Menu(const char *);
  void Init() override;
  void Draw(IVector2 *) override;
  void Update() override;
  void Input() override;
  void DrawUI(IVector2);
};
