#pragma once
#include "state.hpp"

class Menu : public AV::State {
  const char *m_title;
  bool startScene = false;

public:
  Menu(const char *title) : m_title(title) {}
  void Init() override;
  void Draw(IVector2 *) override;
  void Update() override;
  void Input() override;
  void DrawUI(IVector2);
};
