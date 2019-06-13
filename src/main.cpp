#include <Windows.h>
#include <endpointvolume.h>
#include <mmdeviceapi.h>

#define EXIT_ON_ERROR(hr)                                                                         \
    if (FAILED(hr)) {                                                                             \
        goto Exit;                                                                                \
    }
#define ERROR_CANCEL(hr)                                                                          \
    if (FAILED(hr)) {                                                                             \
        MessageBox(hDlg, TEXT("The program will exit."), TEXT("Fatal error"), MB_OK);             \
        EndDialog(hDlg, TRUE);                                                                    \
        return TRUE;                                                                              \
    }
#define SAFE_RELEASE(punk)                                                                        \
    if ((punk) != NULL) {                                                                         \
        (punk)->Release();                                                                        \
        (punk) = NULL;                                                                            \
    }

HWND                         g_hDlg          = NULL;
GUID                         g_guidMyContext = GUID_NULL;
static IAudioEndpointVolume* g_pEndptVol     = NULL;

class CAudioEndpointVolumeCallback : public IAudioEndpointVolumeCallback {
    LONG _cRef;

public:
    CAudioEndpointVolumeCallback() : _cRef(1) {}

    ~CAudioEndpointVolumeCallback() {}

    // IUnknown methods -- AddRef, Release, and QueryInterface

    ULONG STDMETHODCALLTYPE AddRef() { return InterlockedIncrement(&_cRef); }

    ULONG STDMETHODCALLTYPE Release()
    {
        ULONG ulRef = InterlockedDecrement(&_cRef);
        if (0 == ulRef) {
            delete this;
        }
        return ulRef;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID** ppvInterface)
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

    // Callback method for endpoint-volume-change notifications.
    HRESULT STDMETHODCALLTYPE OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify)
    {
        if (pNotify == NULL) {
            return E_INVALIDARG;
        }
        if (g_hDlg != NULL && pNotify->guidEventContext != g_guidMyContext) {
            /*PostMessage(GetDlgItem(g_hDlg, IDC_CHECK_MUTE),
                        BM_SETCHECK,
                        (pNotify->bMuted) ? BST_CHECKED : BST_UNCHECKED,
                        0);

            PostMessage(GetDlgItem(g_hDlg, IDC_SLIDER_VOLUME),
                        TBM_SETPOS,
                        TRUE,
                        LPARAM((UINT32)(MAX_VOL * pNotify->fMasterVolume + 0.5)));*/
        }
        return S_OK;
    }
};

void main()
{
    HRESULT                      hr          = S_OK;
    IMMDeviceEnumerator*         pEnumerator = NULL;
    IMMDevice*                   pDevice     = NULL;
    CAudioEndpointVolumeCallback EPVolEvents;

    CoInitialize(NULL);

    hr = CoCreateGuid(&g_guidMyContext);
    EXIT_ON_ERROR(hr)

    // Get enumerator for audio endpoint devices.
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          __uuidof(IMMDeviceEnumerator),
                          (void**)&pEnumerator);
    EXIT_ON_ERROR(hr)

    // Get default audio-rendering device.
    hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pDevice);
    EXIT_ON_ERROR(hr)

    hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void**)&g_pEndptVol);
    EXIT_ON_ERROR(hr)

    hr = g_pEndptVol->RegisterControlChangeNotify((IAudioEndpointVolumeCallback*)&EPVolEvents);
    EXIT_ON_ERROR(hr)

    // InitCommonControls();
    // DialogBox(hInstance, L"VOLUMECONTROL", NULL, (DLGPROC)DlgProc);

    BOOL muted{};
    hr = g_pEndptVol->GetMute(&muted);
    hr = g_pEndptVol->SetMute(!muted, &g_guidMyContext);

Exit:
    if (FAILED(hr)) {
        MessageBox(
            NULL, TEXT("This program requires Windows Vista."), TEXT("Error termination"), MB_OK);
    }
    if (pEnumerator != NULL) {
        g_pEndptVol->UnregisterControlChangeNotify((IAudioEndpointVolumeCallback*)&EPVolEvents);
    }
    SAFE_RELEASE(pEnumerator)
    SAFE_RELEASE(pDevice)
    SAFE_RELEASE(g_pEndptVol)
    CoUninitialize();
}
