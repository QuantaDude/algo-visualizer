#pragma once
#include "utils.hpp"
namespace AV {
typedef enum { MENU = 0, SCENE, QUIT } AppState;
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
