#include <windows.h>
#include <reshade.hpp>

extern "C" __declspec(dllexport) const char *NAME = "Mi Add-on";
extern "C" __declspec(dllexport) const char *DESCRIPTION = "Mi primer add-on grafico";

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        if (!reshade::register_addon(hModule))
            return FALSE;
        reshade::log::message(reshade::log::level::info,
                              "Mi Add-on cargado");
        break;
    case DLL_PROCESS_DETACH:
        reshade::unregister_addon(hModule);
        break;
    }
    return TRUE;
}
