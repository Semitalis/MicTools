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

#include "mic_controls.hpp"

#include "app.hpp"
#include "resources.hpp"
#include "types.hpp"
#include <mmdeviceapi.h>

namespace mic_tools {

Mic_controls::Mic_controls() : m_ref_count{2}, m_muted{}, m_audio_endpoint{}
{
}

Mic_controls::~Mic_controls()
{
    shutdown();
}

void Mic_controls::init()
{
    Raii_ptr<IMMDeviceEnumerator> pEnumerator{};
    Raii_ptr<IMMDevice>           pDevice{};

    // Get enumerator for audio endpoint devices.
    auto hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
                               NULL,
                               CLSCTX_INPROC_SERVER,
                               __uuidof(IMMDeviceEnumerator),
                               (void**)&pEnumerator);
    if (FAILED(hr)) {
        notify("Error", "Could not create audio endpoint enumerator instance!");
        return;
    }

    // Get default audio-rendering device.
    hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &(*pDevice));
    if (FAILED(hr)) {
        notify("Error", "Could not retrieve default recording audio endpoint!");
        return;
    }

    // activate device
    hr = pDevice->Activate(
        __uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void**)&m_audio_endpoint);
    if (FAILED(hr)) {
        notify("Error", "Could not activate recording audio endpoint!");
        return;
    }

    // setup listener
    hr = m_audio_endpoint->RegisterControlChangeNotify(this);
    if (FAILED(hr)) {
        notify("Error", "Could not register listener to recording audio endpoint!");
        return;
    }

    // retrieve initial mute status
    hr = m_audio_endpoint->GetMute(&m_muted);
    if (FAILED(hr)) {
        notify("Error", "Could not retrieve mute flag from recording audio endpoint!");
        return;
    }
}

void Mic_controls::shutdown()
{
    if (m_audio_endpoint) {
        m_audio_endpoint->UnregisterControlChangeNotify(this);
        m_audio_endpoint->Release();
        m_audio_endpoint = nullptr;
    }
}

void Mic_controls::toggle_mute()
{
    mute(!m_muted);
}

void Mic_controls::mute(BOOL muted)
{
    if (m_muted == muted) {
        return;
    }

    // toggle mute state on audio endpoint
    auto hr = m_audio_endpoint->SetMute(muted, &g_guid);
    if (FAILED(hr)) {
        notify("Error", "Could not update audio endpoint mute status!");
        return;
    }

    // adjust stored flag
    m_muted = muted;

    // notify tray icon
    PostMessage(App::hwnd(), WM_MT_MUTE_STATE, m_muted, 0);
}

bool Mic_controls::on_message(HWND hwnd, UINT message, WPARAM wParam, LPARAM /*lParam*/)
{
    switch (message) {
    case WM_MT_AFTER_CREATE:
        if (hwnd == App::hwnd()) {
            init();
            PostMessage(App::hwnd(), WM_MT_MUTE_STATE, m_muted, 0);
        }
        break;

    case WM_DESTROY:
        if (hwnd == App::hwnd()) {
            shutdown();
        }
        break;

    case WM_MT_CHECK_MUTE:
        mute((BOOL)wParam);
        break;

    case WM_MT_TOGGLE_MUTE:
        toggle_mute();
        break;
    }

    return false;
}

ULONG STDMETHODCALLTYPE Mic_controls::AddRef()
{
    return InterlockedIncrement(&m_ref_count);
}

ULONG STDMETHODCALLTYPE Mic_controls::Release()
{
    ULONG ulRef = InterlockedDecrement(&m_ref_count);
    if (0 == ulRef) {
        delete this;
    }
    return ulRef;
}

HRESULT STDMETHODCALLTYPE Mic_controls::QueryInterface(REFIID riid, VOID** ppvInterface)
{
    if (IID_IUnknown == riid) {
        AddRef();
        *ppvInterface = (IUnknown*)this;
    }
    else if (__uuidof(IAudioEndpointVolumeCallback) == riid) {
        AddRef();
        *ppvInterface = (IAudioEndpointVolumeCallback*)this;
    }
    else {
        *ppvInterface = NULL;
        return E_NOINTERFACE;
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE Mic_controls::OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify)
{
    if (!pNotify) {
        return E_INVALIDARG;
    }
    if (pNotify->guidEventContext != g_guid) {
        PostMessage(App::hwnd(), WM_MT_CHECK_MUTE, pNotify->bMuted, 0);
    }
    return S_OK;
}

} // namespace mic_tools
