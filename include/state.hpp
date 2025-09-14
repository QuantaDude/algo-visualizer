#pragma once
#include "utils.hpp"
namespace AV {
enum class AppState { MENU = 0, SCENE };
enum class AlgorithmState { Idle, Stepping, Running, Done };

class State {
  AppState state;

public:
  virtual ~State() {}
  virtual void Init() = 0;
  virtual void Draw(IVector2 *) = 0;
  virtual void Update() = 0;
  virtual void Input() {};
  void update(void);
};
} // namespace AV
