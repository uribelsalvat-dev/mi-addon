#include <windows.h>
#include <imgui.h>
#include <reshade.hpp>

extern "C" __declspec(dllexport) const char *NAME = "Mi Add-on";
extern "C" __declspec(dllexport) const char *DESCRIPTION = "Mi primer add-on con panel";

struct Control { const char *nombre; float min, max; };

static const Control controles[6] = {
    {"Exposicion", -1.0f, 1.0f},
    {"Contraste",   0.0f, 2.0f},
    {"Saturacion",  0.0f, 2.0f},
    {"Calidez",    -1.0f, 1.0f},
    {"Vineta",      0.0f, 1.0f},
    {"Sombras",    -1.0f, 1.0f},
};

struct Preset { const char *nombre; float valores[6]; };

static const Preset presets[4] = {
    {"Neutro",    {0.0f, 1.0f, 1.0f,  0.0f, 0.0f,  0.0f}},
    {"Dia",       {0.0f, 1.1f, 1.1f,  0.1f, 0.0f,  0.0f}},
    {"Atardecer", {-0.1f, 1.1f, 1.2f, 0.5f, 0.2f, -0.1f}},
    {"Noche",     {0.2f, 1.2f, 0.8f, -0.3f, 0.3f,  0.2f}},
};

static void poner(reshade::api::effect_runtime *runtime, int i, float valor)
{
    reshade::api::effect_uniform_variable var =
        runtime->find_uniform_variable("MiColor.fx", controles[i].nombre);
    if (var.handle != 0)
        runtime->set_uniform_value_float(var, valor);
}

static void dibujar_panel(reshade::api::effect_runtime *runtime)
{
    ImGui::TextUnformatted("Presets");
    for (int p = 0; p < 4; ++p)
    {
        if (p > 0)
            ImGui::SameLine();
        if (ImGui::Button(presets[p].nombre))
            for (int i = 0; i < 6; ++i)
                poner(runtime, i, presets[p].valores[i]);
    }

    ImGui::Separator();

    for (int i = 0; i < 6; ++i)
    {
        reshade::api::effect_uniform_variable var =
            runtime->find_uniform_variable("MiColor.fx", controles[i].nombre);
        if (var.handle == 0)
        {
            ImGui::TextDisabled("%s: no encontrada", controles[i].nombre);
            continue;
        }

        float valor = 0.0f;
        runtime->get_uniform_value_float(var, &valor, 1);
        if (ImGui::SliderFloat(controles[i].nombre, &valor, controles[i].min, controles[i].max))
            runtime->set_uniform_value_float(var, valor);
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
