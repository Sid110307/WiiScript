#pragma once

#include "./widget.h"
#include "./panel.h"
#include "./button.h"
#include "./label.h"
#include "../theme.h"

#include <cstring>

class Modal : public Widget
{
public:
    enum class Kind: uint8_t { None, Input, Message, Confirm };

    Kind kind = Kind::None;
    bool open = false;
    std::string title, message, inputText;
    size_t caret = 0;

    std::function<void(const std::string&)> onOkInput;
    std::function<void()> onOk, onCancel;
    float w = 420.0f, h = 170.0f;

    Modal()
    {
        visible = false;
        enabled = true;
        focusable = true;

        panel = addChild<Panel>(true);
        panel->radiusX = panel->radiusY = 10.0f;

        titleLabel = panel->addChild<Label>("");
        messageLabel = panel->addChild<Label>("");
        okBtn = panel->addChild<Button>("OK");
        cancelBtn = panel->addChild<Button>("Cancel");

        okBtn->onClick = [this]
        {
            if (!open) return;

            if (kind == Kind::Input && onOkInput) onOkInput(inputText);
            else if (onOk) onOk();

            close();
        };

        cancelBtn->onClick = [this]
        {
            if (!open) return;
            if (onCancel) onCancel();

            close();
        };
    }

    void showInput(std::string t, std::string msg, std::string initial, std::function<void(const std::string&)> ok,
                   std::function<void()> cancel = nullptr)
    {
        kind = Kind::Input;
        title = std::move(t);
        message = std::move(msg);
        inputText = std::move(initial);
        caret = inputText.size();

        onOkInput = std::move(ok);
        onCancel = std::move(cancel);
        onOk = nullptr;

        open = true;
        visible = true;
        focused = true;
    }

    void showMessage(std::string t, std::string msg, std::function<void()> ok = nullptr)
    {
        kind = Kind::Message;
        title = std::move(t);
        message = std::move(msg);
        onOk = std::move(ok);
        onCancel = nullptr;
        onOkInput = nullptr;

        open = true;
        visible = true;
        focused = true;
    }

    void showConfirm(std::string t, std::string msg, std::function<void()> ok, std::function<void()> cancel = nullptr)
    {
        kind = Kind::Confirm;
        title = std::move(t);
        message = std::move(msg);
        onOk = std::move(ok);
        onCancel = std::move(cancel);
        onOkInput = nullptr;

        open = true;
        visible = true;
        focused = true;
    }

    void close()
    {
        open = false;
        visible = false;
        focused = false;
        kind = Kind::None;

        onOk = nullptr;
        onOkInput = nullptr;
        onCancel = nullptr;

        title.clear();
        message.clear();
        inputText.clear();
        caret = 0;
    }

    [[nodiscard]] bool isOpen() const { return open && visible; }

    void onKey(const char* key, const KeyAction action)
    {
        if (!isOpen()) return;

        if (kind != Kind::Input)
        {
            if (action == KeyAction::Enter && okBtn && okBtn->onClick) okBtn->onClick();
            return;
        }

        switch (action)
        {
        case KeyAction::Backspace:
            if (caret > 0 && caret <= inputText.size())
            {
                inputText.erase(caret - 1, 1);
                caret--;
            }
            break;
        case KeyAction::Tab:
            break;
        case KeyAction::Enter:
            if (okBtn && okBtn->onClick) okBtn->onClick();
            break;
        case KeyAction::Text:
            if (key && key[0] != '\0')
            {
                inputText.insert(caret, key);
                caret += std::strlen(key);
            }
            break;
        default:
            break;
        }
    }

    bool onEvent(const Input::InputEvent& e) override
    {
        if (!isOpen()) return Widget::onEvent(e);

        if (e.type == Input::InputEvent::Type::KeyDown)
        {
            if (e.key == Input::Key::A && e.pointer.valid)
            {
                if (const Rect p = panel ? panel->worldBounds() : Rect::empty(); !p.contains(e.pointer.x, e.pointer.y))
                {
                    if (onCancel) onCancel();
                    close();

                    return true;
                }
            }

            if (e.key == Input::Key::B)
            {
                if (onCancel) onCancel();
                close();

                return true;
            }
        }

        return Widget::onEvent(e);
    }

protected:
    void onUpdate(double) override
    {
        if (!isOpen() || !panel) return;

        const Rect host = parent ? parent->bounds : Rect({0, 0, 640, 480});
        bounds = host;

        const float ww = std::min(w, host.w - 40.0f);
        const float hh = std::min(h, host.h - 40.0f);

        panel->bounds = {(host.w - ww) / 2.0f, (host.h - hh) / 2.0f, ww, hh};
        titleLabel->text = title;
        messageLabel->text = message;

        constexpr float pad = 12.0f;
        titleLabel->bounds = {pad, pad, ww - pad * 2, 18.0f};
        messageLabel->bounds = {pad, pad + 22.0f, ww - pad * 2, 40.0f};
        inputRect = {pad, pad + 70.0f, ww - pad * 2, 26.0f};

        constexpr float btnW = 90.0f, btnH = 28.0f, gap = 10.0f;
        cancelBtn->bounds = {ww - pad - btnW, hh - pad - btnH, btnW, btnH};
        okBtn->bounds = {ww - pad - btnW * 2 - gap, hh - pad - btnH, btnW, btnH};

        cancelBtn->visible = kind != Kind::Message;
        okBtn->visible = true;
    }

    void onDraw() const override
    {
        if (!isOpen()) return;

        const Rect r = worldBounds();
        GRRLIB_Rectangle(r.x, r.y, r.w, r.h, theme().modalBackdrop, true);

        if (panel && kind == Kind::Input)
        {
            const Rect p = panel->worldBounds();
            const Rect ir = {p.x + inputRect.x, p.y + inputRect.y, inputRect.w, inputRect.h};

            roundedRectangle(ir.x, ir.y, ir.w, ir.h, 6, 6, theme().bg, true);
            roundedRectangle(ir.x, ir.y, ir.w, ir.h, 6, 6, theme().panelBorder, false);

            if (const Font* f = getFont())
            {
                const std::string displayText = inputText.empty() ? " " : inputText;
                f->drawText(displayText, ir.x + 6.0f, ir.y + (ir.h - f->textHeight()) / 2.0f, theme().text);

                if (caret <= inputText.size())
                {
                    const float caretX = ir.x + 6.0f + f->textWidth(inputText.substr(0, caret));
                    GRRLIB_Line(caretX, ir.y + 4.0f, caretX, ir.y + ir.h - 4.0f, theme().accent);
                }
            }
        }
    }

private:
    Panel* panel = nullptr;
    Label *titleLabel = nullptr, *messageLabel = nullptr;
    Button *okBtn = nullptr, *cancelBtn = nullptr;

    Rect inputRect;
};
