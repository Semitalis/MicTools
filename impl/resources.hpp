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

#include <afxwin.h>

#define IDOK 1
#define IDCANCEL 2

#define MIC_ON 101
#define MIC_OFF 102

#define IDC_HOTKEY 1000

#define ID_TRAY_APP_ICON 1001
#define ID_TRAY_SET_HOTKEY 1002
#define ID_TRAY_EXIT 1003

#define IDD_HK_DIALOG 2000

#define ID_MT_HK_TOGGLE_MUTE 1

#define WM_MT_AFTER_CREATE 0x0500
#define WM_MT_SYSICON 0x0501
#define WM_MT_NOTIFY 0x0502
#define WM_MT_MESSAGE 0x0503
#define WM_MT_SET_HOTKEY 0x0504

#define WM_MT_CHECK_MUTE 0x0600
#define WM_MT_TOGGLE_MUTE 0x0602
#define WM_MT_MUTE_STATE 0x0603
