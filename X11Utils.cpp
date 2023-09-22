#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <array>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

struct MouseState {
    bool pressed = false;
    std::chrono::steady_clock::time_point lastChanged = {};
    int keepState = 0;
};

static std::array<MouseState, 12> MouseStates { };

class X11Utils
{
private:
    Display *m_display = XOpenDisplay(NULL);

    int getEventState() {
        int state = 0;
        for (int i = 1; i<= 5; ++i) {
            if (MouseStates[i].pressed) {
                state |= 1 << (7+i);
            }
        }
        return state;
    }

public:
    bool isKeyDown(int keyCode)
    {
        char keys_return[32];
        XQueryKeymap(m_display, keys_return);
        KeyCode kc2 = XKeysymToKeycode(m_display, keyCode);
        bool buttonDown = !!(keys_return[kc2 >> 3] & (1 << (kc2 & 7)));
        return buttonDown;
    }

    bool isMouseDown(int button)
    {
        MouseState& state = MouseStates.at(button);
        return state.pressed;
    }

    void mouseEvent(int type, int button, int keepState = 0, bool force = false)
    {
        MouseState& state = MouseStates.at(button);
        if (type == ButtonPress) {
            if (state.pressed) {
                state.keepState = std::max(state.keepState, keepState);
                return;
            }
        } else if (type == ButtonRelease) {
            if (!state.pressed) {
                state.keepState = std::max(state.keepState, keepState);
                return;
            }
        } else {
            fprintf(stderr, "unknow mouse event type %d\n", type);
            return;
        }
        auto now = std::chrono::steady_clock::now();
        auto changedSince = now - state.lastChanged;
        if (!force && changedSince < std::chrono::milliseconds(state.keepState)) {
            return;
        }
        if (!force && changedSince < std::chrono::milliseconds(20)) {
            utils::randomSleep(20, 30);
            now = std::chrono::steady_clock::now();
        }
        state.pressed = (type == ButtonPress);
        state.lastChanged = now;
        state.keepState = keepState;
        // printf("mouse %s\n", state.pressed ? "pressed" : "released");
        XEvent event;
        memset(&event, 0x00, sizeof(event));
        event.type = type;
        event.xbutton.button = button;
        event.xbutton.state = getEventState();
        event.xbutton.same_screen = True;
        XQueryPointer(m_display, RootWindow(m_display, DefaultScreen(m_display)), &event.xbutton.root, &event.xbutton.window, &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
        event.xbutton.subwindow = event.xbutton.window;
        while (event.xbutton.subwindow)
        {
            event.xbutton.window = event.xbutton.subwindow;
            XQueryPointer(m_display, event.xbutton.window, &event.xbutton.root, &event.xbutton.subwindow, &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
        }
        if (XSendEvent(m_display, PointerWindow, True, 0xfff, &event) == 0)
            fprintf(stderr, "XSendEvent error\n");
        XFlush(m_display);
    }
};
