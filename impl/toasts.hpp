#pragma once

#include "types.hpp"

namespace mic_tools {

class Toasts : public Message_handler {
public:
    Toasts();
    ~Toasts() override;

protected:
    void init();
    void shutdown();
    void notify(const char* title, const char* message);
    bool on_message(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) override;

private:
    bool  m_initialized;
    BOOL  m_muted;
    INT64 m_current_toast;
};

} // namespace mic_tools
