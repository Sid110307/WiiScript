#pragma once

#include <memory>

#include "../core/input_events.h"

#include "./widgets/widget.h"
#include "./widgets/panel.h"
#include "./widgets/button.h"
#include "./widgets/list.h"

class UIRoot
{
public:
    std::unique_ptr<Panel> root = std::make_unique<Panel>();
    PointerState pointer = {};
    Widget* capture = nullptr;

    Panel *left = nullptr, *center = nullptr, *bottom = nullptr;
    Button *btnRun = nullptr, *btnStop = nullptr, *btnSave = nullptr;
    List* fileList = nullptr;
    bool showLeft = true, showBottom = true;

    void init();
    void layout(float screenW, float screenH) const;
    void update(double dt) const;
    void routeEvent(const InputEvent& e);
    void draw() const;
};
