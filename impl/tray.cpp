// MIT License
//
// Copyright (c) 2019 Kerim Merdenoglu
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "tray.hpp"

#include "app.hpp"
#include "resources.hpp"
#include <shellapi.h>

namespace mic_tools {

Tray::Tray()
    : m_context_menu{}, m_icon_off{}, m_icon_on{}, m_tray_icon_info{},
      m_tooltip_muted{TEXT("Muted")}, m_tooltip_unmuted{TEXT("Unmuted")}
{
}

Tray::~Tray()
{
    Shell_NotifyIcon(NIM_DELETE, &m_tray_icon_info);
}

void Tray::init()
{
    m_context_menu = CreatePopupMenu();
    AppendMenu(m_context_menu, MF_STRING, ID_TRAY_SET_HOTKEY, TEXT("Set Hotkey"));
    AppendMenu(m_context_menu, MF_STRING, ID_TRAY_EXIT, TEXT("Exit MicTools"));

    auto instance = GetModuleHandle(NULL);
    m_icon_off    = (HICON)LoadIcon(instance, MAKEINTRESOURCE(MIC_OFF));
    m_icon_on     = (HICON)LoadIcon(instance, MAKEINTRESOURCE(MIC_ON));

    memset(&m_tray_icon_info, 0, sizeof(NOTIFYICONDATA));
    m_tray_icon_info.cbSize           = sizeof(NOTIFYICONDATA);
    m_tray_icon_info.uID              = ID_TRAY_APP_ICON;
    m_tray_icon_info.uFlags           = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    m_tray_icon_info.uCallbackMessage = WM_MT_SYSICON;
    m_tray_icon_info.hIcon            = m_icon_off;
    m_tray_icon_info.hWnd             = App::hwnd();
    strncpy(m_tray_icon_info.szTip, m_tooltip_muted, sizeof(m_tooltip_muted));

    Shell_NotifyIcon(NIM_ADD, &m_tray_icon_info);
}

void Tray::shutdown()
{
    if (m_context_menu) {
        (void)DestroyMenu(m_context_menu);
        m_context_menu = nullptr;
    }
}

void Tray::update_tray_icon(BOOL muted)
{
    if (muted) {
        m_tray_icon_info.hIcon = m_icon_off;
        strncpy(m_tray_icon_info.szTip, m_tooltip_muted, sizeof(m_tooltip_muted));
    }
    else {
        m_tray_icon_info.hIcon = m_icon_on;
        strncpy(m_tray_icon_info.szTip, m_tooltip_unmuted, sizeof(m_tooltip_unmuted));
    }

    Shell_NotifyIcon(NIM_MODIFY, &m_tray_icon_info);
}

bool Tray::on_message(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_MT_AFTER_CREATE:
        if (hwnd == App::hwnd()) {
            init();
        }
        break;

    case WM_DESTROY:
        if (hwnd == App::hwnd()) {
            shutdown();
        }
        break;

    case WM_MT_MUTE_STATE:
        update_tray_icon((BOOL)wParam);
        break;

    case WM_MT_SYSICON:
        if (wParam == ID_TRAY_APP_ICON) {
            SetForegroundWindow(App::hwnd());
        }

        if (lParam == WM_LBUTTONUP) {
            PostMessage(App::hwnd(), WM_MT_TOGGLE_MUTE, 0, 0);
            return true;
        }

        if (lParam == WM_RBUTTONUP) {
            // get current mouse position
            POINT curPoint;
            GetCursorPos(&curPoint);
            SetForegroundWindow(App::hwnd());

            // show context menu
            auto clicked = TrackPopupMenu(m_context_menu,
                                          TPM_RETURNCMD | TPM_NONOTIFY,
                                          curPoint.x,
                                          curPoint.y,
                                          0,
                                          App::hwnd(),
                                          NULL);

            // send benign message to window to make sure the menu goes away.
            SendMessage(App::hwnd(), WM_NULL, 0, 0);

            // quit the application.
            switch (clicked) {
            case ID_TRAY_SET_HOTKEY:
                PostMessage(App::hwnd(), WM_MT_SET_HOTKEY, 0, 0);
                return true;

            case ID_TRAY_EXIT:
                PostQuitMessage(0);
                return true;
            }
        }
        break;
    }

    return false;
}

} // namespace mic_tools
