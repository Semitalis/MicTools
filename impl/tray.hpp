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
