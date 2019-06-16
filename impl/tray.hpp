#pragma once

#include "notifier.hpp"
#include "types.hpp"

namespace mic_tools {

class Tray final
    : public Message_handler
    , Notifier {
public:
    Tray();
    ~Tray();

protected:
    void init();
    void shutdown();
    void update_tray_icon(BOOL muted);
    bool on_message(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) override;

private:
    HMENU          m_context_menu;
    HICON          m_icon_off;
    HICON          m_icon_on;
    NOTIFYICONDATA m_tray_icon_info;
    TCHAR          m_tooltip_muted[16];
    TCHAR          m_tooltip_unmuted[16];
};

} // namespace mic_tools
