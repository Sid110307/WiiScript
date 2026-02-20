#pragma once

#include <memory>

#include "../platform/platform.h"
#include "../keyboard/keyboard.h"

#include "./widgets/widget.h"
#include "./widgets/panel.h"
#include "./widgets/button.h"
#include "./widgets/list.h"
#include "./widgets/text_input.h"
#include "./widgets/scrollbars.h"
#include "./widgets/layout.h"

class UIRoot
{
public:
    UIRoot(Font& codeFont, Font& uiFont);
    void layout(float screenW, float screenH) const;
    void update(double dt);
    void routeEvent(const Input::InputEvent& e);
    void draw() const;

    Input::PointerState pointer = {};
    bool quit = false, showLeft = true, showBottom = true;

    std::unique_ptr<Panel> root = std::make_unique<Panel>();
    Widget *captureWidget = nullptr, *hoverWidget = nullptr, *focusedWidget = nullptr;
    Box* toolbar = nullptr;
    Panel *left = nullptr, *center = nullptr, *bottom = nullptr;

    Button *btnRun = nullptr, *btnStop = nullptr, *btnSave = nullptr;
    List* fileList = nullptr;
    ScrollView *fileListScroll = nullptr, *editorScroll = nullptr;
    TextInput* editor = nullptr;
    Keyboard* keyboard = nullptr;

private:
    void setFocus(Widget* w, bool show);
    void rebuildFocusList();
    [[nodiscard]] Widget* findNextFocusable(int dirX, int dirY) const;

    std::vector<Widget*> focusableWidgets;
};
