#include "scene.hpp"
#include "app.hpp"
#include "raygui.h"
#include "raylib.h"
#include "raymath.h"
#include "state.hpp"
#include "utils.h"
#include "web.h"
#include <string>

#if defined(PLATFORM_WEB)
#include <emscripten/html5.h>

#endif

AV::Scene::Scene(Font *font)
    : scene_gui_state(InitBaseGui()), g_camera({0}),
      m_input_mode(InteractionMode::None), m_font(font),
      algorithm_state(AV::Idle) {}

void AV::Scene::init() {

  GuiSetFont(*m_font);
  GuiSetStyle(DEFAULT, TEXT_SIZE, 25);
  lastKey = "";

  IVector2 *resolution = App::getInstance().getResolution();

  Node newNode;
  newNode.radius = 15;

  newNode.pos = {(float)(resolution->x / 2), (float)(resolution->y / 2)};
  newNode.collider = {newNode.pos.x - newNode.radius,
                      newNode.pos.y - newNode.radius, (float)newNode.radius * 2,
                      (float)newNode.radius * 2};
  newNode.data = 0;

  nodes.push_back(newNode);
  selected_node = nodes.end();
  selected_edge_origin = nodes.end();
  root = &nodes[0];
  g_camera.zoom = 1.0f;
}
void AV::Scene::draw(IVector2 *resolution) {
#if defined(PLATFORM_WEB)
  emscripten_get_canvas_element_size("#canvas", &resolution->x, &resolution->y);
#elif defined(PLATFORM_DESKTOP)
  resolution->x = GetScreenWidth();
  resolution->y = GetScreenHeight();
#endif

  BeginDrawing();
  ClearBackground({41, 41, 41, 100});

  BeginMode2D(g_camera);
  for (size_t i = 0; i < edges.size(); i++) {
    Color col = (i == hoveredEdgeIdx) ? RED : DARKGRAY;
    DrawLineEx(nodes[edges[i].from].pos, nodes[edges[i].to].pos, 3, col);
  }
  if (selected_edge_origin != nodes.end()) {
    DrawLineEx(selected_edge_origin->pos,
               GetScreenToWorld2D(GetMousePosition(), g_camera), 3, RED);
  }
  // if (selectedNodeOrigin != nullptr) {
  //   DrawLineEx(selectedNodeOrigin->pos,
  //              GetScreenToWorld2D(GetMousePosition(), g_camera), 3, RED);
  // }
  for (size_t i = 0; i < nodes.size(); i++) {
    DrawCircleV(nodes[i].pos, nodes[i].radius,
                hoveredNodeIdx == i                    ? GREEN
                : (selected_node == nodes.begin() + i) ? BLUE
                : (root == &nodes[i])                  ? GOLD
                                                       : RED);

    char idText[10];
    sprintf(idText, "%d", (int)i);
    Vector2 textSize = MeasureTextEx(*m_font, idText, 20, 1);
    // DrawText(idText, nodes[i].pos.x - textSize.x / 2,
    //        nodes[i].pos.y - textSize.y / 2, 20, WHITE);
    DrawTextEx(*m_font, idText, nodes[i].pos - (textSize / 2), 20.0f, 1.0f,
               WHITE);
  }

  EndMode2D();
  AV::Scene::drawUI(*resolution);

  EndDrawing();
}
void AV::Scene::drawUI(IVector2 resolution) {

  int prev_main_mode = main_mode;
  Vector2 mousePos = GetMousePosition();
  char posStr[64];
  snprintf(posStr, sizeof(posStr), "(%.0f, %.0f)", mousePos.x, mousePos.y);
  // Draw controls
  if (scene_gui_state.DropdownBox007EditMode)
    GuiLock();

  if (scene_gui_state.main_window_active) {
    scene_gui_state.main_window_active =
        !GuiWindowBox(scene_gui_state.layoutRecs[0], "Algorithm Visualizer");

    GuiToggleGroup(scene_gui_state.layoutRecs[1], "FREE;NODE;EDGE", &main_mode);
    if (prev_main_mode != main_mode) {
      sub_mode = 0; // Reset to SELECT when switching modes
    }

    // Second toggle group - only show for NODE or EDGE modes
    if (main_mode == 1 || main_mode == 2) {
      GuiToggleGroup(scene_gui_state.layoutRecs[16], "SELECT;CREATE;EDIT",
                     &sub_mode);
    }

    // Update the actual enum
    update_input_mode();
    GuiStatusBar(scene_gui_state.layoutRecs[2], posStr);
    GuiStatusBar(scene_gui_state.layoutRecs[3], getKeyName());
    GuiStatusBar(scene_gui_state.layoutRecs[4], "MODE");
    GuiLabel(scene_gui_state.layoutRecs[5], "MODE");

    if (GuiButton(scene_gui_state.layoutRecs[8], "Delete"))
      Button008();
    if (GuiButton(scene_gui_state.layoutRecs[10], "Step"))
      Button010();

    if (GuiButton(scene_gui_state.layoutRecs[6], "Reset"))
      Button006();
    if (GuiButton(scene_gui_state.layoutRecs[15], "Toggle Info Tab"))
      Button015();

    if (GuiButton(scene_gui_state.layoutRecs[9], "Start"))
      Button009();
    if (GuiDropdownBox(scene_gui_state.layoutRecs[7],
                       "Adjacency Matrix; Adjacency List;",
                       &scene_gui_state.DropdownBox007Active,
                       scene_gui_state.DropdownBox007EditMode))
      scene_gui_state.DropdownBox007EditMode =
          !scene_gui_state.DropdownBox007EditMode;
  }
  if (scene_gui_state.showAdjacencyPanel) {
    Rectangle adjPanelRect = {
        scene_gui_state.layoutRecs[7].x,
        scene_gui_state.layoutRecs[7].y + scene_gui_state.layoutRecs[7].height +
            10,
        scene_gui_state.layoutRecs[7].width,
        200 // Fixed height, you can make this dynamic
    };

    // Draw the panel background
    GuiPanel(adjPanelRect, "Adjacency Representation");

    // Update adjacency layouts based on current selection
    updateAdjacencyLayouts(adjPanelRect);

    // Draw adjacency list or matrix
    drawAdjacencyRepresentation(adjPanelRect);
  }

  if (scene_gui_state.WindowBox012Active) {
    scene_gui_state.WindowBox012Active =
        !GuiWindowBox(scene_gui_state.layoutRecs[12], "Info");
    GuiLine(scene_gui_state.layoutRecs[13], NULL);
    GuiGroupBox(scene_gui_state.layoutRecs[11], "Stack");
    GuiGroupBox(scene_gui_state.layoutRecs[14], "code");
  }

  GuiUnlock();
}
void AV::Scene::update_input_mode() {
  if (main_mode == 0) { // FREE
    m_input_mode = InteractionMode::None;
  } else if (main_mode == 1) { // NODE
    switch (sub_mode) {
    case 0:
      m_input_mode = InteractionMode::NodeSelect;
      break;
    case 1:
      m_input_mode = InteractionMode::NodeCreate;
      break;
    case 2:
      m_input_mode = InteractionMode::NodeEdit;
      break;
    }
  } else if (main_mode == 2) { // EDGE
    switch (sub_mode) {
    case 0:
      m_input_mode = InteractionMode::EdgeSelect;
      break;
    case 1:
      m_input_mode = InteractionMode::EdgeCreate;
      break;
    case 2:
      m_input_mode = InteractionMode::EdgeEdit;
      break;
    }
  }
}

void AV::Scene::input() {
  mouse_world_pos = GetScreenToWorld2D(GetMousePosition(), g_camera);

  switch (m_input_mode) {
  case InteractionMode::NodeSelect:
    // Handle node selection

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      bool clickedOnNode = false;

      for (size_t i = 0; i < nodes.size(); i++) {
        if (CheckCollisionPointRec(mouse_world_pos, nodes[i].collider)) {

          clickedOnNode = true;

          if (selected_node == nodes.end()) {
            // selectedNode = &nodes[i];
            selected_node = nodes.begin() + i;
            // nodes[i].oldPos = nodes[i].pos;
            selected_node->oldPos = selected_node->pos;
            break;
          } else if (selected_node != nodes.begin() + i) {
            break;

          } else if (selected_node == nodes.begin() + i) {
            // Deselect if clicking on the same node
            clickedOnNode = false;
            // selectedNode = nullptr;
          }
        }
      }

      // Deselect if clicking on empty space
      if (!clickedOnNode && selected_node != nodes.end()) {
        //        selectedNode = nullptr;
        selected_node = nodes.end();
      }
    }
    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_DELETE)) {
      if (selected_node != nodes.end() && nodes.begin() != nodes.end()) {
        // nodes.begin();
        //  nodes.erase(nodes[0] +selectedNode);
        for (std::vector<Edge>::iterator edge = edges.begin();
             edge < edges.end(); edge++) {
          if (edge->from == (size_t)(selected_node - nodes.begin()) ||
              edge->to == (size_t)(selected_node - nodes.begin())) {
            edges.erase(edge);
          }
          if (edge->to > (size_t)(selected_node - nodes.begin())) {
            edge->to -= 1;
          }
          if (edge->from > (size_t)(selected_node - nodes.begin())) {
            edge->from -= 1;
          }
        }
        nodes.erase(selected_node);
        selected_node = nodes.end();
        selected_edge_origin = nodes.end();
      }
    }
    break;
  case InteractionMode::NodeEdit:
    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_DELETE)) {
      if (selected_node != nodes.end() && nodes.begin() != nodes.end()) {
        // nodes.begin();
        //  nodes.erase(nodes[0] +selectedNode);
        //
        for (std::vector<Edge>::iterator edge = edges.begin();
             edge < edges.end(); edge++) {

          if (edge->from == (size_t)(selected_node - nodes.begin()) ||
              edge->to == (size_t)(selected_node - nodes.begin())) {
            edges.erase(edge);
          }
          if (edge->to > (size_t)(selected_node - nodes.begin())) {
            edge->to -= 1;
          }
          if (edge->from > (size_t)(selected_node - nodes.begin())) {
            edge->from -= 1;
          }
        }

        nodes.erase(selected_node);
        selected_node = nodes.end();
        selected_edge_origin = nodes.end();
      }
    }

    break;

  case InteractionMode::EdgeCreate:
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {

      for (size_t i = 0; i < nodes.size(); i++) {
        if (CheckCollisionPointRec(mouse_world_pos, nodes[i].collider)) {

          if (selected_edge_origin == nodes.end()) {
            m_input_mode = InteractionMode::EdgeCreate;
            selected_edge_origin = nodes.begin() + i;
          } else if (selected_edge_origin != nodes.end() &&
                     selected_edge_origin != nodes.begin() + i) {

            Edge newEdge;
            newEdge.from = selected_edge_origin - nodes.begin();
            newEdge.to = i;

            bool edgeExists = false;
            for (const auto &edge : edges) {
              if ((edge.from == newEdge.from && edge.to == newEdge.to) ||
                  (edge.from == newEdge.to && edge.to == newEdge.from)) {
                edgeExists = true;
                break;
              }
            }
            if (!edgeExists) {
              edges.push_back(newEdge);

              // Also add to node's edge list
              nodes[newEdge.from].edges.push_back(newEdge.to);
              nodes[newEdge.to].edges.push_back(newEdge.from);
            }

            selected_edge_origin = nodes.end();
          } else {
            selected_edge_origin = nodes.end();
          }
          return;
        }
      }
      // selectedNodeOrigin = nullptr;
      selected_edge_origin = nodes.end();
    }
    break;

  case InteractionMode::EdgeEdit:
    // Maybe highlight edge, allow delete
    for (size_t i = 0; i < edges.size(); i++) {
      if (IsMouseHoveringEdge(mouse_world_pos, nodes[edges[i].from].pos,
                              nodes[edges[i].to].pos)) {
        m_input_mode = InteractionMode::EdgeEdit;
        hoveredEdgeIdx = i;
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) &&
            selected_edge_origin == nodes.end()) {
          // selectedNodeOrigin = &nodes[edges[i].from];
          selected_edge_origin = nodes.begin() + (edges.begin() + i)->from;
          std::vector<Edge>::iterator it = edges.begin();
          it += i;
          edges.erase(it);
          hoveredEdgeIdx = SIZE_MAX;
          return;
        }
      }
    }
    if (selected_edge_origin != nodes.end() &&
        IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
      for (size_t i = 0; i < nodes.size(); i++) {
        if (CheckCollisionPointRec(mouse_world_pos, nodes[i].collider)) {

          if (selected_edge_origin != nodes.begin() + i) {
            // create new edge
            Edge newEdge;
            int originNodeIdx = (int)(selected_edge_origin - nodes.begin());
            newEdge.from = originNodeIdx;
            newEdge.to = i;
            edges.push_back(newEdge);

            nodes[newEdge.from].edges.push_back(newEdge.to);
            nodes[newEdge.to].edges.push_back(newEdge.from);
            // selectedNodeOrigin = nullptr;
            selected_edge_origin = nodes.end();
          }
        }
      }
    }

    break;

  case InteractionMode::None:
  default:
    // Camera panning, idle state

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      for (size_t i = 0; i < nodes.size(); i++) {
        if (CheckCollisionPointRec(mouse_world_pos, nodes[i].collider)) {

          nodes[i].oldPos = nodes[i].pos;
          selected_node = nodes.begin() + i;
          // selectedNode = &nodes[i];
          m_input_mode = InteractionMode::NodeSelect;
          main_mode = 1;
          sub_mode = 0;

          return;
        }
      }
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {

      for (size_t i = 0; i < nodes.size(); i++) {
        if (CheckCollisionPointRec(mouse_world_pos, nodes[i].collider)) {
          m_input_mode = InteractionMode::EdgeCreate;
          main_mode = 2;
          sub_mode = 1;

          // selectedNodeOrigin = &nodes[i];
          selected_edge_origin = nodes.begin() + i;
          return;
        }
      }
    }
    for (size_t i = 0; i < edges.size(); i++) {
      if (IsMouseHoveringEdge(mouse_world_pos, nodes[edges[i].from].pos,
                              nodes[edges[i].to].pos)) {
        hoveredEdgeIdx = i;
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) &&
            selected_edge_origin == nodes.end()) {
          m_input_mode = InteractionMode::EdgeEdit;
          main_mode = 2;
          sub_mode = 2;

          selected_edge_origin = nodes.begin() + (edges.begin() + i)->from;
          // selectedNodeOrigin = &nodes[edges[i].from];
          std::vector<Edge>::iterator it = edges.begin();
          it += i;
          edges.erase(it);
          hoveredEdgeIdx = SIZE_MAX;
        }

        return;
      }
    }

    break;
  }

  // camera controls
  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    Vector2 delta = GetMouseDelta();
    delta = Vector2Scale(delta, -1.0f / g_camera.zoom);
    g_camera.target = Vector2Add(g_camera.target, delta);
  }

  float wheel = GetMouseWheelMove();
  if (wheel != 0) {
    // Get the world point that is under the mouse

    g_camera.offset = GetMousePosition();

    g_camera.target = mouse_world_pos;

    // Uses log scaling to provide consistent zoom speed
    float scale = 0.2f * wheel;

    g_camera.zoom = Clamp(expf(logf(g_camera.zoom) + scale), 0.125f, 64.0f);
  }

  // reset
  if (IsKeyPressed(KEY_ESCAPE)) {
    m_input_mode = InteractionMode::None;
    main_mode = 0;
    sub_mode = 0;

    if (selected_node != nodes.end()) {
      selected_node->pos = selected_node->oldPos;
      selected_node->collider = {selected_node->pos.x - selected_node->radius,
                                 selected_node->pos.y - selected_node->radius,
                                 (float)selected_node->radius * 2,
                                 (float)selected_node->radius * 2};
    }

    selected_node = nodes.end();
    selected_edge_origin = nodes.end();
    // selectedNodeOrigin = nullptr;
  }
  if (algorithm_state == AV::Idle) {
    if (IsKeyPressed(KEY_A)) {
      m_input_mode = InteractionMode::NodeSelect;
      main_mode = 1;
      sub_mode = 0;
      if (selected_node == nodes.end() && selected_edge_origin == nodes.end()) {
        Node newNode;
        Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), g_camera);
        newNode.pos = mouseWorld;
        newNode.oldPos = mouseWorld;
        newNode.radius = 15;
        newNode.collider = {
            newNode.pos.x - newNode.radius, newNode.pos.y - newNode.radius,
            (float)newNode.radius * 2, (float)newNode.radius * 2};
        newNode.data = nodes.size();

        nodes.push_back(newNode);
        // selectedNode = &nodes.back();
        selected_node = nodes.end() - 1;
        selected_edge_origin = nodes.end();
      }
    }

    if (selected_edge_origin == nodes.end() && selected_node == nodes.end()) {
      if (IsKeyPressed(KEY_N)) {

        m_input_mode = InteractionMode::NodeSelect;

        main_mode = 1;
        sub_mode = 0;
      } else if (IsKeyReleased(KEY_E)) {
        if (main_mode == 0) {
          m_input_mode = InteractionMode::EdgeSelect;

          main_mode = 2;
          sub_mode = 0;
        } else if (main_mode == 1) {
          sub_mode = 2;
          m_input_mode = InteractionMode::NodeEdit;
        } else if (main_mode == 2) {
          sub_mode = 2;
          m_input_mode = InteractionMode::EdgeEdit;
        }
      }
      if (IsKeyPressed(KEY_S)) {
        sub_mode = 0;

        if (main_mode == 1) {
          m_input_mode = InteractionMode::NodeSelect;
        } else if (main_mode == 2) {
          m_input_mode = InteractionMode::EdgeSelect;
        }
      }
      if (IsKeyPressed(KEY_C)) {
        sub_mode = 1;
        if (main_mode == 1) {
          m_input_mode = InteractionMode::NodeCreate;
        } else if (main_mode == 2) {
          m_input_mode = InteractionMode::EdgeCreate;
        }
      }
    }
  }
}

bool AV::Scene::IsMouseHoveringEdge(const Vector2 &mouse, const Vector2 &p1,
                                    const Vector2 &p2, float thickness) {
  // Vector from p1 to p2
  Vector2 edge = Vector2Subtract(p2, p1);
  Vector2 mouseVec = Vector2Subtract(mouse, p1);

  float edgeLenSq = Vector2LengthSqr(edge);
  if (edgeLenSq == 0.0f)
    return false;

  // Project mouseVec onto edge (clamped to [0,1])
  float t = Vector2DotProduct(mouseVec, edge) / edgeLenSq;
  t = Clamp(t, 0.0f, 1.0f);

  // Closest point on the edge
  Vector2 closest = Vector2Add(p1, Vector2Scale(edge, t));

  // Distance from mouse to closest point
  float dist = Vector2Distance(mouse, closest);
  return dist <= thickness;
}

void AV::Scene::update() {
  UpdateGuiLayout();

  // if (selectedNode != nullptr) {
  //   selectedNode->pos = GetMousePosition();
  //   selectedNode->collider.x = GetMousePosition().x;
  //   selectedNode->collider.y = GetMousePosition().y;
  // }
  if (selected_node != nodes.end()) {
    Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), g_camera);
    selected_node->pos = mouseWorld;
    selected_node->collider.x =
        mouseWorld.x - selected_node->collider.width / 2;
    selected_node->collider.y =
        mouseWorld.y - selected_node->collider.height / 2;
  }
}
void AV::Scene::Button006() {} // Button: Button006 logic
void AV::Scene::Button008() {} // Button: Button008 logic
void AV::Scene::Button009() {} // Button: Button009 logic
void AV::Scene::Button010() {} // Button: Button010 logic
void AV::Scene::Button015() {} // Button: Button015 logic

AV::BaseGuiState AV::Scene::InitBaseGui(void) {
  IVector2 resolution = *App::getInstance().getResolution();
  Vector2 scale{static_cast<float>(resolution.x) / 1920 * 100,
                static_cast<float>(resolution.y) / 1080 * 100};
  BaseGuiState state = {0};

  // Init anchors
  state.main_window_anchor = (Vector2){0, 0}; // ANCHOR ID:1
  state.anchor02 =
      (Vector2){static_cast<float>(resolution.x - 320), 32}; // ANCHOR ID:2
  state.anchor03 =
      (Vector2){static_cast<float>(resolution.x - 350), 0}; // ANCHOR ID:3
  state.anchor04 =
      (Vector2){static_cast<float>(resolution.x - 320), 200}; // ANCHOR ID:4

  // Initilize controls variables
  state.main_window_active = true;    // WindowBox: WindowBox000
  state.input_mode_toggle_active = 0; // ToggleGroup: ToggleGroup001
  state.DropdownBox007EditMode = false;
  state.DropdownBox007Active = 0;  // DropdownBox: DropdownBox007
  state.WindowBox012Active = true; // WindowBox: WindowBox012

  // Init controls rectangles
  state.layoutRecs[0] =
      (Rectangle){state.main_window_anchor.x + 0,
                  state.main_window_anchor.y + 0, 360 + scale.x,
                  static_cast<float>(resolution.y)}; // WindowBox: WindowBox000
  state.layoutRecs[1] = (Rectangle){
      state.main_window_anchor.x + 16, state.main_window_anchor.y + 48,
      104 + (scale.x / 3), 24 + scale.y}; // ToggleGroup: ToggleGroup001
  state.layoutRecs[2] = (Rectangle){
      state.main_window_anchor.x + 120, state.main_window_anchor.y + 936,
      120 + scale.x, 24 + scale.y}; // StatusBar: StatusBar003
  state.layoutRecs[3] = (Rectangle){
      state.main_window_anchor.x + 240, state.main_window_anchor.y + 936,
      120 + scale.x, 24 + scale.y}; // StatusBar: StatusBar004
  state.layoutRecs[4] = (Rectangle){
      state.main_window_anchor.x + 0, state.main_window_anchor.y + 936,
      120 + scale.x, 24 + scale.y}; // StatusBar: StatusBar004
  state.layoutRecs[5] = (Rectangle){
      state.main_window_anchor.x + 16, state.main_window_anchor.y + 24,
      120 + scale.x, 24 + scale.y}; // Label: Label005
  state.layoutRecs[6] = (Rectangle){state.main_window_anchor.x + 120,
                                    state.main_window_anchor.y + 872,
                                    120 + scale.x, 24}; // Button: Button006
  state.layoutRecs[7] = (Rectangle){
      state.main_window_anchor.x + 96, state.main_window_anchor.y + 272,
      152 + scale.x, 32 + scale.y}; // DropdownBox: DropdownBox007
  state.layoutRecs[8] = (Rectangle){
      state.main_window_anchor.x + 120, state.main_window_anchor.y + 208,
      120 + scale.x, 24 + scale.y}; // Button: Button008
  state.layoutRecs[9] = (Rectangle){
      state.main_window_anchor.x + 120, state.main_window_anchor.y + 144,
      120 + scale.x, 24 + scale.y}; // Button: Button009
  state.layoutRecs[10] = (Rectangle){
      state.main_window_anchor.x + 120, state.main_window_anchor.y + 176,
      120 + scale.x, 24 + scale.y}; // Button: Button010
  state.layoutRecs[11] =
      (Rectangle){state.anchor02.x + 0, state.anchor02.y + 8, 192 + scale.x,
                  128 + scale.y}; // GroupBox: GroupBox011
  state.layoutRecs[12] =
      (Rectangle){state.anchor03.x + 0, state.anchor03.y + 0, 208 + scale.x,
                  368 + scale.y}; // WindowBox: WindowBox012
  state.layoutRecs[13] =
      (Rectangle){state.anchor03.x + 40, state.anchor03.y + 184, 120 + scale.x,
                  16 + scale.y}; // Line: Line013
  state.layoutRecs[14] =
      (Rectangle){state.anchor04.x + 0, state.anchor04.y + 8, 192 + scale.x,
                  144 + scale.y}; // GroupBox: GroupBox014
  state.layoutRecs[15] = (Rectangle){state.main_window_anchor.x + 120,
                                     state.main_window_anchor.y + 832,
                                     120 + scale.x, 24}; // Button: Button015
  state.layoutRecs[16] = (Rectangle){
      state.main_window_anchor.x + 16, state.main_window_anchor.y + 72,
      104 + (scale.x / 3), 24}; // ToggleGroup: ToggleGroup002

  // Custom variables initialization

  return state;
}

void AV::Scene::UpdateGuiLayout() {
  IVector2 resolution = *App::getInstance().getResolution();
  Vector2 scale{(static_cast<float>(resolution.x) / 1920) * 100,
                (static_cast<float>(resolution.y) / 1080) * 100};
  // Update anchors based on current resolution
  scene_gui_state.main_window_anchor = (Vector2){0, 0}; // Top-left
  scene_gui_state.anchor02 = (Vector2){static_cast<float>(resolution.x - 320),
                                       32}; // Right side, 32px from top
  scene_gui_state.anchor03 =
      (Vector2){static_cast<float>(resolution.x - 208 - scale.x),
                0}; // Right side, top (using window width 208)
  scene_gui_state.anchor04 = (Vector2){static_cast<float>(resolution.x - 320),
                                       200}; // Right side, 200px from top

  // Update all rectangles based on current anchors and resolution
  scene_gui_state.layoutRecs[0] =
      (Rectangle){scene_gui_state.main_window_anchor.x + 0,
                  scene_gui_state.main_window_anchor.y + 0, 360 + scale.x,
                  static_cast<float>(resolution.y)}; // Full height window

  scene_gui_state.layoutRecs[1] = (Rectangle){
      scene_gui_state.main_window_anchor.x + 16,
      scene_gui_state.main_window_anchor.y + 48, 104 + (scale.x / 3), 24};

  // Status bars at the bottom - stick to bottom
  float statusBarY = static_cast<float>(resolution.y - 24); // 24px from bottom
  scene_gui_state.layoutRecs[2] =
      (Rectangle){scene_gui_state.main_window_anchor.x + 120 + scale.x / 3,
                  statusBarY, 120 + scale.x / 3, 24}; // StatusBar003
  scene_gui_state.layoutRecs[3] =
      (Rectangle){scene_gui_state.main_window_anchor.x + 240 + scale.x / 3 * 2,
                  statusBarY, 120 + scale.x / 3, 24}; // StatusBar004
  scene_gui_state.layoutRecs[4] =
      (Rectangle){scene_gui_state.main_window_anchor.x + 0, statusBarY,
                  120 + scale.x / 3, 24}; // StatusBar004

  scene_gui_state.layoutRecs[5] =
      (Rectangle){scene_gui_state.main_window_anchor.x + 16,
                  scene_gui_state.main_window_anchor.y + 24, 120 + scale.x, 24};
  scene_gui_state.layoutRecs[6] = (Rectangle){
      scene_gui_state.main_window_anchor.x + 120,
      scene_gui_state.main_window_anchor.y + 872, 120 + scale.x, 24};
  scene_gui_state.layoutRecs[7] = (Rectangle){
      scene_gui_state.main_window_anchor.x + 96,
      scene_gui_state.main_window_anchor.y + 272, 152 + scale.x, 32};
  scene_gui_state.layoutRecs[8] = (Rectangle){
      scene_gui_state.main_window_anchor.x + 120,
      scene_gui_state.main_window_anchor.y + 208, 120 + scale.x, 24};
  scene_gui_state.layoutRecs[9] = (Rectangle){
      scene_gui_state.main_window_anchor.x + 120,
      scene_gui_state.main_window_anchor.y + 144, 120 + scale.x, 24};
  scene_gui_state.layoutRecs[10] = (Rectangle){
      scene_gui_state.main_window_anchor.x + 120,
      scene_gui_state.main_window_anchor.y + 176, 120 + scale.x, 24};
  scene_gui_state.layoutRecs[11] =
      (Rectangle){scene_gui_state.anchor02.x + 0,
                  scene_gui_state.anchor02.y + 8, 192 + scale.x, 128};

  // Second window sticks to right edge
  scene_gui_state.layoutRecs[12] = (Rectangle){
      scene_gui_state.anchor03.x + 0, scene_gui_state.anchor03.y + 0,
      208 + scale.x, 368 + scale.y}; // WindowBox012

  scene_gui_state.layoutRecs[13] = (Rectangle){scene_gui_state.anchor03.x + 40,
                                               scene_gui_state.anchor03.y + 184,
                                               120 + scale.x, 16 + scale.y};
  scene_gui_state.layoutRecs[14] =
      (Rectangle){scene_gui_state.anchor04.x + 0,
                  scene_gui_state.anchor04.y + 8, 192 + scale.x, 144 + scale.y};
  scene_gui_state.layoutRecs[15] = (Rectangle){
      scene_gui_state.main_window_anchor.x + 120,
      scene_gui_state.main_window_anchor.y + 832, 120 + scale.x, 24};

  scene_gui_state.layoutRecs[16] =
      (Rectangle){scene_gui_state.main_window_anchor.x + 16,
                  scene_gui_state.main_window_anchor.y + 72,
                  104 + (scale.x / 3), 24}; // ToggleGroup: ToggleGroup002
}

void AV::Scene::updateAdjacencyLayouts(const Rectangle &panelRect) {
  scene_gui_state.adjacencyLayouts.clear();

  float startX = panelRect.x + 10;
  float startY = panelRect.y + 30;
  float elementWidth = 40;
  float elementHeight = 25;
  float spacing = 5;

  if (scene_gui_state.DropdownBox007Active == 0) { // Adjacency Matrix
    // Create matrix layout
    int n = nodes.size();
    if (n == 0)
      return;

    // Column headers (node indices)
    for (int i = 0; i <= n; i++) {
      for (int j = 0; j <= n; j++) {
        float x = startX + j * (elementWidth + spacing);
        float y = startY + i * (elementHeight + spacing);

        if (i == 0 && j == 0) {
          // Top-left corner (empty)
          scene_gui_state.adjacencyLayouts.push_back(
              {x, y, elementWidth, elementHeight});
        } else if (i == 0) {
          // Column header (node indices)
          scene_gui_state.adjacencyLayouts.push_back(
              {x, y, elementWidth, elementHeight});
        } else if (j == 0) {
          // Row header (node indices)
          scene_gui_state.adjacencyLayouts.push_back(
              {x, y, elementWidth, elementHeight});
        } else {
          // Matrix cell
          scene_gui_state.adjacencyLayouts.push_back(
              {x, y, elementWidth, elementHeight});
        }
      }
    }
  } else { // Adjacency List
    // Create list layout
    for (size_t i = 0; i < nodes.size(); i++) {
      // Node label
      float x = startX;
      float y = startY + i * (elementHeight + spacing);
      scene_gui_state.adjacencyLayouts.push_back(
          {x, y, elementWidth, elementHeight});

      // Adjacent nodes
      for (size_t j = 0; j < nodes[i].edges.size(); j++) {
        x = startX + (j + 1) * (elementWidth + spacing);
        scene_gui_state.adjacencyLayouts.push_back(
            {x, y, elementWidth, elementHeight});
      }
    }
  }
}

void AV::Scene::drawAdjacencyRepresentation(const Rectangle &panelRect) {
  Vector2 mousePos = GetMousePosition();
  scene_gui_state.hoveredAdjacencyElement = -1;

  // Check for hover
  for (size_t i = 0; i < scene_gui_state.adjacencyLayouts.size(); i++) {
    if (CheckCollisionPointRec(mousePos, scene_gui_state.adjacencyLayouts[i])) {
      scene_gui_state.hoveredAdjacencyElement = i;
      break;
    }
  }

  float startX = panelRect.x + 10;
  float startY = panelRect.y + 30;
  float elementWidth = 40;
  float elementHeight = 25;
  float spacing = 5;

  if (scene_gui_state.DropdownBox007Active == 0) { // Adjacency Matrix
    drawAdjacencyMatrix(startX, startY, elementWidth, elementHeight, spacing);
  } else { // Adjacency List
    drawAdjacencyList(startX, startY, elementWidth, elementHeight, spacing);
  }
}

void AV::Scene::drawAdjacencyMatrix(float startX, float startY, float width,
                                    float height, float spacing) {
  int n = nodes.size();
  if (n == 0)
    return;

  // Draw column headers
  for (int j = 0; j < n; j++) {
    float x = startX + (j + 1) * (width + spacing);
    float y = startY;
    char label[10];
    sprintf(label, "%d", j);

    bool isHovered = (scene_gui_state.hoveredAdjacencyElement == j + 1);
    Color bgColor = isHovered ? BLUE : DARKGRAY;
    if (isHovered)
      GuiSetState(STATE_FOCUSED);

    // GuiDrawRectangle({x, y, width, height}, 1, bgColor, BLANK);
    GuiLabel({x, y, width, height}, label);
    GuiSetState(STATE_NORMAL);
    if (isHovered) {
      hoveredNodeIdx = j;
      scene_gui_state.adjacencyIsNode = true;
    }
  }

  // Draw row headers and matrix
  for (int i = 0; i < n; i++) {
    // Row header
    float headerX = startX;
    float headerY = startY + (i + 1) * (height + spacing);
    char headerLabel[10];
    sprintf(headerLabel, "%d", i);

    bool headerHovered =
        (scene_gui_state.hoveredAdjacencyElement == (i + 1) * (n + 1));
    Color headerBgColor = headerHovered ? BLUE : DARKGRAY;

    // GuiDrawRectangle({headerX, headerY, width, height}, 1, headerBgColor,
    // BLANK); GuiDrawText(headerLabel, {headerX, headerY, width, height},
    // TEXT_ALIGN_CENTER,
    //  headerHovered ? WHITE : LIGHTGRAY);
    if (headerHovered)
      GuiSetState(STATE_FOCUSED);

    GuiLabel({headerX, headerY, width, height}, headerLabel);
    GuiSetState(STATE_NORMAL);

    if (headerHovered) {
      hoveredNodeIdx = i;
      scene_gui_state.adjacencyIsNode = true;
    }

    // Matrix cells
    for (int j = 0; j < n; j++) {
      float cellX = startX + (j + 1) * (width + spacing);
      float cellY = startY + (i + 1) * (height + spacing);

      bool hasEdge = false;
      for (const auto &edge : edges) {
        if ((edge.from == i && edge.to == j) ||
            (edge.from == j && edge.to == i)) {
          hasEdge = true;
          break;
        }
      }

      int cellIndex = (i + 1) * (n + 1) + (j + 1);
      bool cellHovered = (scene_gui_state.hoveredAdjacencyElement == cellIndex);

      Color cellColor;
      if (cellHovered) {
        cellColor = RED;
        // Find and highlight the corresponding edge
        for (size_t k = 0; k < edges.size(); k++) {
          if ((edges[k].from == i && edges[k].to == j) ||
              (edges[k].from == j && edges[k].to == i)) {
            hoveredEdgeIdx = k;
            scene_gui_state.adjacencyIsNode = false;
            break;
          }
        }
      } else {
        cellColor = hasEdge ? GREEN : DARKGRAY;
      }

      char cellText[2] = {hasEdge ? '1' : '0', '\0'};
      //    GuiDrawRectangle({cellX, cellY, width, height}, 1, cellColor,
      //    BLANK);
      //  GuiDrawText(cellText, {cellX, cellY, width, height},
      //  TEXT_ALIGN_CENTER, WHITE);
      if (cellHovered)
        GuiSetState(STATE_FOCUSED);

      GuiLabel({cellX, cellY, width, height}, cellText);
      GuiSetState(STATE_NORMAL);
    }
  }
}

void AV::Scene::drawAdjacencyList(float startX, float startY, float width,
                                  float height, float spacing) {
  for (size_t i = 0; i < nodes.size(); i++) {
    float x = startX;
    float y = startY + i * (height + spacing);

    // Node label
    char nodeLabel[10];
    sprintf(nodeLabel, "%zu:", i);

    bool nodeHovered = (scene_gui_state.hoveredAdjacencyElement ==
                        i * (nodes[i].edges.size() + 1));
    Color nodeColor = nodeHovered ? BLUE : DARKGRAY;

    // GuiDrawRectangle({x, y, width, height}, 1, nodeColor, BLANK);
    // GuiDrawText(nodeLabel, {x, y, width, height}, TEXT_ALIGN_CENTER,
    //          nodeHovered ? WHITE : LIGHTGRAY);
    // GuiSetStyle(LABEL, TEXT_SIZE, 25);

    if (nodeHovered)
      GuiSetState(STATE_FOCUSED);
    GuiLabel({x, y, width, height}, nodeLabel);
    GuiSetState(STATE_NORMAL);

    if (nodeHovered) {
      hoveredNodeIdx = i;
      scene_gui_state.adjacencyIsNode = true;
    }

    // Adjacent nodes
    for (size_t j = 0; j < nodes[i].edges.size(); j++) {
      x = startX + (j + 1) * (width + spacing);

      char adjLabel[10];
      sprintf(adjLabel, "%d", nodes[i].edges[j]);

      int elementIndex = i * (nodes[i].edges.size() + 1) + j + 1;
      bool adjHovered =
          (scene_gui_state.hoveredAdjacencyElement == elementIndex);
      Color adjColor = adjHovered ? GREEN : DARKGRAY;

      // GuiDrawRectangle({x, y, width, height}, 1, adjColor, BLANK);
      // GuiDrawText(adjLabel, {x, y, width, height}, TEXT_ALIGN_CENTER,
      // WHITE);
      if (adjHovered)
        GuiSetState(STATE_FOCUSED);

      GuiLabel({x, y, width, height}, adjLabel);
      GuiSetState(STATE_NORMAL);

      if (adjHovered) {
        // Find and highlight the corresponding edge
        for (size_t k = 0; k < edges.size(); k++) {
          if ((edges[k].from == i && edges[k].to == nodes[i].edges[j]) ||
              (edges[k].from == nodes[i].edges[j] && edges[k].to == i)) {
            hoveredEdgeIdx = k;
            scene_gui_state.adjacencyIsNode = false;
            break;
          }
        }
      }
    }
  }
}
