#include "app.hpp"

int WINAPI WinMain(HINSTANCE /*hInstance*/,
                   HINSTANCE /*hPrevInstance*/,
                   LPSTR /*lpCmdLine*/,
                   int /*nCmdShow*/)
{
    mic_tools::App app{};
    app.run();

    return 0;
}
