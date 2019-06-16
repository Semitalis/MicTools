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

#include "app.hpp"

#include "types.hpp"
#include <afxcmn.h>
#include <fstream>
#include <sstream>

namespace mic_tools {

GUID        g_guid{GUID_NULL};
static App* g_instance{};

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (g_instance) {
        return g_instance->on_message(hwnd, message, wParam, lParam);
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK DialogProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (g_instance) {
        return g_instance->on_dialog(hwnd, message, wParam, lParam);
    }
    return 0;
}

App::App()
    : m_hwnd{}, m_hwnd_hk{}, m_hotkey_vk{'M'}, m_hotkey_modifiers{MOD_CONTROL},
      m_mic_controls{}, m_tray{}, m_toasts{}, m_message_handlers{}
{
    g_instance = this;
}

App::~App()
{
    shutdown();
    g_instance = nullptr;
}

HWND App::hwnd()
{
    return g_instance->m_hwnd;
}

void App::run()
{
    init();

    // main message loop
    MSG messages;
    while (GetMessage(&messages, NULL, 0, 0)) {
        if (!IsDialogMessage(m_hwnd_hk, &messages)) {
            TranslateMessage(&messages);
            DispatchMessage(&messages);
        }
    }
}

LRESULT App::on_message(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    for (auto handler : m_message_handlers) {
        if (handler->on_message(hwnd, message, wParam, lParam)) {
            return 0;
        }
    }

    switch (message) {
    case WM_SYSCOMMAND:
        switch (wParam & 0xFFF0) {
        case SC_MINIMIZE:
        case SC_CLOSE:
            return true;
        default:
            break;
        }
        break;

    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;

    case WM_MT_SET_HOTKEY:
        show_hotkey_dialog();
        return 0;

    case WM_HOTKEY:
        if (wParam == ID_MT_HK_TOGGLE_MUTE) {
            PostMessage(m_hwnd, WM_MT_TOGGLE_MUTE, 0, 0);
            return 0;
        }
        break;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT App::on_dialog(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    CHotKeyCtrl* pwnd;
    WORD         key;
    WORD         modifiers;
    switch (message) {
    case WM_INITDIALOG:
        SetForegroundWindow(App::hwnd());
        pwnd = static_cast<CHotKeyCtrl*>(CWnd::FromHandle(hwnd)->GetDlgItem(IDC_HOTKEY));
        pwnd->SetHotKey(m_hotkey_vk, m_hotkey_modifiers);
        pwnd->SetFocus();
        break;

    case WM_COMMAND:
        switch (wParam) {
        case IDOK:
            pwnd = static_cast<CHotKeyCtrl*>(CWnd::FromHandle(hwnd)->GetDlgItem(IDC_HOTKEY));
            pwnd->GetHotKey(key, modifiers);
            if (key) {
                m_hotkey_modifiers = modifiers;
                m_hotkey_vk        = key;
            }
            // fallthrough
        case IDCANCEL:
            hide_hotkey_dialog();
            return 1;
        }
        break;
    }
    return 0;
}

void App::init()
{
    HINSTANCE instance{GetModuleHandle(NULL)};
    char*     class_name{"MicTools"};

    (void)CoInitialize(NULL);

    // create GUID
    auto hr = CoCreateGuid(&g_guid);
    if (FAILED(hr)) {
        notify("Error", "Could not create a UIID!");
        return;
    }

    // window config
    WNDCLASSEX wincl;
    wincl.cbSize        = sizeof(WNDCLASSEX);
    wincl.hInstance     = instance;
    wincl.lpszClassName = class_name;
    wincl.lpfnWndProc   = WindowProcedure;
    wincl.style         = CS_DBLCLKS;
    wincl.hIcon         = NULL;
    wincl.hIconSm       = NULL;
    wincl.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wincl.lpszMenuName  = NULL;
    wincl.cbClsExtra    = 0;
    wincl.cbWndExtra    = 0;
    wincl.hbrBackground = (HBRUSH)(CreateSolidBrush(RGB(255, 255, 255)));

    // register the window class
    if (!RegisterClassEx(&wincl)) {
        notify("Error", "Could not register main window!");
        return;
    }

    // enqueue all message handlers
    m_message_handlers.push_back(&m_toasts);
    m_message_handlers.push_back(&m_tray);
    m_message_handlers.push_back(&m_mic_controls);

    // create main window
    m_hwnd = CreateWindowEx(0,                   /* Extended possibilites for variation */
                            class_name,          /* Classname */
                            class_name,          /* Title Text */
                            WS_OVERLAPPEDWINDOW, /* default window */
                            CW_USEDEFAULT,       /* Windows decides the position */
                            CW_USEDEFAULT,       /* where the window ends up on the screen */
                            400,                 /* The programs width */
                            300,                 /* and height in pixels */
                            HWND_DESKTOP,        /* The window is a child-window to desktop */
                            NULL,                /* No menu */
                            instance,            /* Program Instance handler */
                            NULL                 /* No Window Creation data */
    );

    // read out settings
    load_settings();

    // register global hotkey
    (void)RegisterHotKey(m_hwnd, ID_MT_HK_TOGGLE_MUTE, m_hotkey_modifiers, m_hotkey_vk);

    // event after application has been created
    // Note: this also includes the creation of tray icon etc
    PostMessage(m_hwnd, WM_MT_AFTER_CREATE, 0, 0);
}

void App::shutdown()
{
    if (m_hwnd_hk) {
        DestroyWindow(m_hwnd_hk);
        m_hwnd_hk = nullptr;
    }

    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }

    m_message_handlers.clear();

    CoUninitialize();
}

void App::load_settings()
{
    auto          filepath = get_app_data_dir() + "\\settings.ini";
    std::ifstream file{filepath};
    if (file.is_open()) {
        std::string dummy;
        char        c;
        file >> dummy >> c >> std::hex >> m_hotkey_vk >> dummy >> c >> m_hotkey_modifiers;
    }
}

void App::save_settings()
{
    auto          filepath = get_app_data_dir() + "\\settings.ini";
    std::ofstream file{filepath};
    if (file.is_open()) {
        file << "Hotkey.key = 0x" << std::hex << m_hotkey_vk << std::endl
             << "Hotkey.modifiers = 0x" << std::hex << m_hotkey_modifiers << std::endl;
    }
}

void App::show_hotkey_dialog()
{
    if (m_hwnd_hk) {
        return;
    }

    // unregister global hotkey
    (void)UnregisterHotKey(m_hwnd, ID_MT_HK_TOGGLE_MUTE);

    // create hotkey dialog
    m_hwnd_hk = CreateDialog(
        GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_HK_DIALOG), HWND_DESKTOP, DialogProcedure);
    ShowWindow(m_hwnd_hk, SW_SHOW);
}

void App::hide_hotkey_dialog()
{
    if (!m_hwnd_hk) {
        return;
    }

    // DestroyWindow(m_hwnd_hk);
    (void)EndDialog(m_hwnd_hk, 0);
    m_hwnd_hk = nullptr;
    SetForegroundWindow(App::hwnd());

    // register global hotkey
    (void)RegisterHotKey(m_hwnd, ID_MT_HK_TOGGLE_MUTE, m_hotkey_modifiers, m_hotkey_vk);

    // save settings
    save_settings();
}

} // namespace mic_tools
