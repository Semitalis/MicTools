#include "notifier.hpp"

#include "app.hpp"
#include "resources.hpp"
#include "types.hpp"

namespace mic_tools {

void Notifier::notify(const char* title, const char* message)
{
    PostMessage(App::hwnd(),
                WM_MT_MESSAGE,
                reinterpret_cast<WPARAM>(new std::string(title)),
                reinterpret_cast<WPARAM>(new std::string(message)));
    // MessageBoxA(NULL, message, title, MB_OK);
}

} // namespace mic_tools
