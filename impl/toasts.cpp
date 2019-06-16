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

#include "toasts.hpp"

#include "app.hpp"
#include "resources.hpp"
#include "wintoastlib.h"
#include <algorithm>
#include <memory>

using namespace WinToastLib;

namespace mic_tools {

class WinToastHandler : public IWinToastHandler {
public:
    void toastActivated() const override {}

    void toastActivated(int /*actionIndex*/) const override {}

    void toastDismissed(WinToastDismissalReason /*state*/) const override {}

    void toastFailed() const override {}
};

static std::wstring g_icon_paths[2]{};

Toasts::Toasts() : m_initialized{}, m_muted{}, m_current_toast{}
{
}

Toasts::~Toasts()
{
}

void Toasts::init()
{
    // initialize WinToast framework
    if (!WinToast::isCompatible()) {
        return;
    }

    WinToast::instance()->setAppName(L"MicTools");
    const auto aumi = WinToast::configureAUMI(L"Semitalis", L"MicTools", L"product", L"version");
    WinToast::instance()->setAppUserModelId(aumi);

    if (!WinToast::instance()->initialize()) {
        notify("Error", "could not initialize the WinToast lib!");
        return;
    }

    m_initialized = true;

    // save images to AppData
    auto dir = get_app_data_dir() + "\\resources";
    auto hr  = CreateDirectory(dir.c_str(), NULL);
    if (FAILED(hr)) {
        return;
    }

    // write images to disk
    auto save_image = [](auto idx, auto id, auto path) {
        auto src      = FindResource(NULL, MAKEINTRESOURCE(id), RT_GROUP_ICON);
        auto res      = LoadResource(NULL, src);
        auto icon_dir = (GRPICONDIR*)LockResource(res);
        if (icon_dir) {
            for (size_t i = 0; i < icon_dir->idCount; ++i) {
                auto icon = icon_dir->idEntries[i];

                auto src  = FindResource(NULL, MAKEINTRESOURCE(icon.nID), RT_ICON);
                auto size = SizeofResource(NULL, src);
                auto res  = LoadResource(NULL, src);
                auto data = (BYTE*)LockResource(res);
                if (size && data) {
                    HANDLE file = CreateFile(path.c_str(),
                                             GENERIC_WRITE,
                                             FILE_SHARE_READ,
                                             NULL,
                                             CREATE_ALWAYS,
                                             FILE_ATTRIBUTE_NORMAL,
                                             NULL);

                    DWORD written{};
                    (void)WriteFile(file, data, size, &written, NULL);
                    (void)CloseHandle(file);

                    g_icon_paths[idx] = s_to_wstring(path.c_str());
                }
            }
        }
    };

    save_image(0, MIC_OFF, dir + "\\icon_off.ico");
    save_image(1, MIC_ON, dir + "\\icon_on.ico");
}

void Toasts::shutdown()
{
    m_initialized = false;

    if (m_current_toast) {
        WinToast::instance()->hideToast(m_current_toast);
        m_current_toast = 0;
    }
}

void Toasts::notify(const char* title, const char* message)
{
    if (m_initialized) {
        auto*            handler = new WinToastHandler;
        WinToastTemplate templ{WinToastTemplate::ImageAndText02};
        if (m_muted) {
            templ.setImagePath(g_icon_paths[0]);
        }
        else {
            templ.setImagePath(g_icon_paths[1]);
        }
        templ.setTextField(s_to_wstring(title), WinToastTemplate::FirstLine);
        templ.setTextField(s_to_wstring(message), WinToastTemplate::SecondLine);

        if (m_current_toast) {
            WinToast::instance()->hideToast(m_current_toast);
            m_current_toast = 0;
        }
        m_current_toast = WinToast::instance()->showToast(templ, handler);
        if (m_current_toast) {
            return;
        }
        delete handler;
    }

    MessageBoxA(NULL, message, title, MB_OK);
}

bool Toasts::on_message(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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
        m_muted = (BOOL)wParam;
        if (m_muted) {
            notify("Microphone", "The microphone is now MUTED");
        }
        else {
            notify("Microphone", "The microphone is now RECORDING");
        }
        break;

    case WM_MT_MESSAGE:
        notify(std::shared_ptr<std::string>(reinterpret_cast<std::string*>(wParam))->c_str(),
               std::shared_ptr<std::string>(reinterpret_cast<std::string*>(lParam))->c_str());
        break;
    }

    return false;
}

} // namespace mic_tools
