#include "raylib.h"
#include "state.hpp"
#include <cstddef>
#include <cstdint>
#include <stack>
#include <unordered_map>
#include <vector>
namespace AV {

typedef struct {
  // Define anchors
  Vector2 main_window_anchor; // ANCHOR ID:1
  Vector2 anchor02;           // ANCHOR ID:2
  Vector2 anchor03;           // ANCHOR ID:3
  Vector2 anchor04;           // ANCHOR ID:4

  // Define controls variables
  bool main_window_active;      // WindowBox: WindowBox000
  int input_mode_toggle_active; // ToggleGroup: ToggleGroup001
  bool DropdownBox007EditMode;
  int DropdownBox007Active; // DropdownBox: DropdownBox007
  bool WindowBox012Active;  // WindowBox: WindowBox012

  // Define rectangles
  Rectangle layoutRecs[17];

  // Custom state variables (depend on development software)
  // NOTE: This variables should be added manually if required
  std::vector<Rectangle> adjacencyLayouts;
  bool showAdjacencyPanel = true;
  int hoveredAdjacencyElement = -1;
  bool adjacencyIsNode = false;

  Vector2 stackScroll = {0, 0};
  Vector2 adjacencyScroll = {0, 0};
  Rectangle adjacencyContent = {0};

} BaseGuiState;

class Scene : public State {
  enum class DFSPhase { ENTER, EXIT };

  struct DFSFrame {
    int node;
    DFSPhase phase;
  };
  AlgorithmState algorithm_state;
  Camera2D g_camera;
  Vector2 mouse_world_pos;
  Font *m_font;
  BaseGuiState scene_gui_state;

  BaseGuiState InitBaseGui(void);
  void UpdateGuiLayout(void);
  // void GuiAlgoViz(BaseGuiState *state);

  static void Button006();
  static void Button008();
  static void StartButton(Scene *);
  static void StepButton(Scene *);
  static void Button015();
  enum class InteractionMode {
    None = 0, // can hover and delete
    NodeSelect = 1,
    EdgeSelect = 2,

    NodeCreate = 3,
    NodeEdit = 4,

    EdgeCreate = 5,
    EdgeEdit = 6,
  } m_input_mode;
  int main_mode = 0; // 0=FREE, 1=NODE, 2=EDGE
  int sub_mode = 0;  // 0=SELECT, 1=CREATE, 2=EDIT
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

  Node *root;
  // allocate array of nodes
  std::vector<Node> nodes;
  std::vector<Edge> edges;
  std::stack<DFSFrame> dfs_stack;
  std::vector<bool> visited;

  std::vector<Node>::iterator selected_node;
  std::vector<Node>::iterator selected_edge_origin;
  size_t hoveredEdgeIdx = SIZE_MAX;
  size_t hoveredNodeIdx = SIZE_MAX;

public:
  Scene(Font *);
  void init() override;
  void input() override;
  void update_input_mode();
  void draw(IVector2 *) override;
  void drawUI(IVector2);
  void update() override;
  bool IsMouseHoveringEdge(const Vector2 &, const Vector2 &, const Vector2 &,
                           float thickness = 5.0f);
  void updateAdjacencyLayouts(const Rectangle &);

  void drawAdjacencyRepresentation(const Rectangle &);
  void drawAdjacencyMatrix(float, float, float, float, float);
  void drawAdjacencyList(float, float, float, float, float);

  void dfs();
  void createStack();
  void drawDFSStack(Rectangle);
  void traverse();
};

}; // namespace AV
