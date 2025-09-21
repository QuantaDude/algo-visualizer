#include "raylib.h"
#include "state.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>
namespace AV {
class Scene : public State {
  Camera2D g_camera;

  size_t hoveredEdgeIdx = SIZE_MAX;
  size_t hoveredNodeIdx = SIZE_MAX;

  enum class InteractionMode {
    None = 0, // can hover and delete
    NodeSelect = 1,
    NodeCreate = 2,
    EdgeCreate = 3,
    EdgeEdit = 4,
  };

  struct Edge {
    size_t from;
    size_t to;
  };

  struct Node {
    Vector2 pos;
    Vector2 oldPos;
    uint16_t radius;
    Rectangle collider;

    int64_t data;
    std::vector<int> edges;
  };

  // allocate array of nodes
  std::vector<Node> nodes;
  std::vector<Edge> edges;
};

}; // namespace AV
