#pragma once

#include "notifier.hpp"
#include "types.hpp"
#include <endpointvolume.h>

namespace mic_tools {

class Mic_controls final
    : public Message_handler
    , Notifier
    , IAudioEndpointVolumeCallback {
public:
    Mic_controls();
    ~Mic_controls() override;

protected:
    void init();
    void shutdown();
    void toggle_mute();
    void mute(BOOL muted);
    bool on_message(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) override;

    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID** ppvInterface);
    HRESULT STDMETHODCALLTYPE OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify);

private:
    LONG                  m_ref_count;
    BOOL                  m_muted;
    IAudioEndpointVolume* m_audio_endpoint;
};

} // namespace mic_tools
