#include <windows.h>
#include <imgui.h>
#include <reshade.hpp>

extern "C" __declspec(dllexport) const char *NAME = "Mi Add-on";
extern "C" __declspec(dllexport) const char *DESCRIPTION = "Mi primer add-on con panel";

static float exposicion = 0.0f;

static void dibujar_panel(reshade::api::effect_runtime *runtime)
{
    ImGui::TextUnformatted("Controlo MiColor.fx");
    if (ImGui::SliderFloat("Exposicion", &exposicion, -1.0f, 1.0f))
    {
        reshade::api::effect_uniform_variable var =
            runtime->find_uniform_variable("MiColor.fx", "Exposicion");
        if (var.handle != 0)
            runtime->set_uniform_value_float(var, exposicion);
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        if (!reshade::register_addon(hModule))
            return FALSE;
        reshade::register_overlay("Mi Add-on", dibujar_panel);
        break;
    case DLL_PROCESS_DETACH:
        reshade::unregister_addon(hModule);
        break;
    }
    return TRUE;
}
