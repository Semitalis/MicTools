#pragma once

#include "mic_controls.hpp"
#include "notifier.hpp"
#include "toasts.hpp"
#include "tray.hpp"
#include "types.hpp"
#include <vector>

namespace mic_tools {

class App final : Notifier {
public:
    App();
    ~App();

    static HWND hwnd();

    void    run();
    LRESULT on_message(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT on_dialog(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

protected:
    void init();
    void shutdown();
    void load_settings();
    void save_settings();
    void show_hotkey_dialog();
    void hide_hotkey_dialog();

private:
    HWND                          m_hwnd;
    HWND                          m_hwnd_hk;
    UINT                          m_hotkey_vk;
    UINT                          m_hotkey_modifiers;
    Mic_controls                  m_mic_controls;
    Tray                          m_tray;
    Toasts                        m_toasts;
    std::vector<Message_handler*> m_message_handlers;
};

} // namespace mic_tools
