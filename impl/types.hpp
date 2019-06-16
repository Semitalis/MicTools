#pragma once

#include "resources.hpp"
#include <codecvt>
#include <locale>
#include <shlobj_core.h>
#include <string>

namespace mic_tools {

extern GUID g_guid;

// small helper to auto release a windows resource
template <typename T>
class Raii_ptr final {
public:
    Raii_ptr() : m_ptr{} {}

    Raii_ptr(T* ptr) : m_ptr{ptr} {}

    ~Raii_ptr()
    {
        if (m_ptr) {
            m_ptr->Release();
        }
    }

    T* operator->() { return m_ptr; }

    T*& operator*() { return m_ptr; }

private:
    T* m_ptr;
};

class Message_handler {
public:
    virtual ~Message_handler() = default;

    virtual bool on_message(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) = 0;
};

inline std::wstring s_to_wstring(const char* s)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(s);
}

inline std::string w_to_string(const wchar_t* s)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(s);
}

inline std::string get_app_data_dir()
{
    // fetch default roaming app data path
    PWSTR path;
    auto  hr = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &path);
    if (FAILED(hr)) {
        return "";
    }

    // assemble application path
    auto dir = w_to_string(path) + "\\MicTools";

    // create directory if not existing
    hr = CreateDirectory(dir.c_str(), NULL);
    if (FAILED(hr)) {
        return "";
    }

    return dir;
}

#ifdef WIN32
#    pragma pack(push)
#    pragma pack(2)
typedef struct {
    BYTE  bWidth;       // Width, in pixels, of the image
    BYTE  bHeight;      // Height, in pixels, of the image
    BYTE  bColorCount;  // Number of colors in image (0 if >=8bpp)
    BYTE  bReserved;    // Reserved
    WORD  wPlanes;      // Color Planes
    WORD  wBitCount;    // Bits per pixel
    DWORD dwBytesInRes; // how many bytes in this resource?
    WORD  nID;          // the ID
} GRPICONDIRENTRY, *LPGRPICONDIRENTRY;

typedef struct {
    WORD            idReserved;   // Reserved (must be 0)
    WORD            idType;       // Resource type (1 for icons)
    WORD            idCount;      // How many images?
    GRPICONDIRENTRY idEntries[1]; // The entries for each image
} GRPICONDIR, *LPGRPICONDIR;
#    pragma pack(pop)
#endif

} // namespace mic_tools
