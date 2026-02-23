#pragma once

#include <memory>

#include "../platform/platform.h"
#include "../keyboard/keyboard.h"

#include "./widgets/widget.h"
#include "./widgets/panel.h"
#include "./widgets/list.h"
#include "./widgets/text_input.h"
#include "./widgets/scrollbar.h"
#include "./widgets/context_menu.h"

class UIRoot
{
public:
    UIRoot(float screenW, float screenH, Font& codeFont, Font& uiFont);
    void layout() const;
    void update(double dt);
    void routeEvent(const Input::InputEvent& e);
    void draw() const;

    Input::PointerState pointer = {};
    bool quit = false, showLeft = true, showBottom = true;

    std::unique_ptr<Panel> root = std::make_unique<Panel>();
    Widget *captureWidget = nullptr, *hoverWidget = nullptr, *focusedWidget = nullptr;
    Panel *left = nullptr, *center = nullptr, *bottom = nullptr;

    List* fileList = nullptr;
    ScrollView *fileListScroll = nullptr, *editorScroll = nullptr;
    TextInput* editor = nullptr;
    Keyboard* keyboard = nullptr;
    ContextMenu* contextMenu = nullptr;

private:
    void setFocus(Widget* w, bool show);
    void rebuildFocusList();
    [[nodiscard]] Widget* findNextFocusable(int dirX, int dirY) const;

    std::vector<Widget*> focusableWidgets;
    float screenW = 0.0f, screenH = 0.0f;
};
