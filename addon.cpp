#include <windows.h>
#include <imgui.h>
#include <reshade.hpp>

extern "C" __declspec(dllexport) const char *NAME = "Mi Add-on";
extern "C" __declspec(dllexport) const char *DESCRIPTION = "Mi primer add-on con panel";

static const int N = 11;

struct Control { const char *nombre; float min, max; };

static const Control controles[N] = {
    {"Exposicion",  -2.0f,  2.0f},
    {"Contraste",    0.5f,  1.5f},
    {"Saturacion",   0.0f,  2.0f},
    {"Calidez",     -1.0f,  1.0f},
    {"Vineta",       0.0f,  1.0f},
    {"Sombras",     -1.0f,  1.0f},
    {"Luces",       -1.0f,  1.0f},
    {"Nitidez",      0.0f,  1.5f},
    {"Bloom",        0.0f,  2.0f},
    {"BloomUmbral",  0.3f,  0.95f},
    {"BloomRadio",   1.0f, 30.0f},
};

struct Preset { const char *nombre; float valores[N]; };

static const Preset presets[6] = {
    {"Neutro",     {0.0f, 1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, 0.0f, 0.7f, 12.0f}},
    {"Dia",        {0.0f, 1.1f,  1.1f,  0.1f,  0.0f,  0.0f,  0.0f,  0.3f, 0.2f, 0.7f, 12.0f}},
    {"Atardecer",  {-0.1f, 1.1f, 1.2f,  0.5f,  0.2f, -0.1f,  0.1f,  0.2f, 0.5f, 0.6f, 14.0f}},
    {"Noche",      {0.2f, 1.2f,  0.8f, -0.3f,  0.3f,  0.2f, -0.1f,  0.2f, 0.4f, 0.6f, 16.0f}},
    {"Natural",    {0.0f, 1.05f, 0.95f, 0.05f, 0.05f, 0.1f, -0.05f, 0.3f, 0.1f, 0.8f, 10.0f}},
    {"Cinematico", {-0.1f, 1.25f, 0.85f, -0.1f, 0.35f, -0.1f, 0.0f,  0.4f, 0.4f, 0.7f, 14.0f}},
};

static void poner(reshade::api::effect_runtime *runtime, int i, float valor)
{
    reshade::api::effect_uniform_variable var =
        runtime->find_uniform_variable("MiColor.fx", controles[i].nombre);
    if (var.handle != 0)
        runtime->set_uniform_value_float(var, valor);
}

static void guardar_mio(reshade::api::effect_runtime *runtime)
{
    for (int i = 0; i < N; ++i)
    {
        reshade::api::effect_uniform_variable var =
            runtime->find_uniform_variable("MiColor.fx", controles[i].nombre);
        if (var.handle == 0)
            continue;
        float valor = 0.0f;
        runtime->get_uniform_value_float(var, &valor, 1);
        reshade::set_config_value(nullptr, "MiAddon", controles[i].nombre, valor);
    }
}

static void cargar_mio(reshade::api::effect_runtime *runtime)
{
    for (int i = 0; i < N; ++i)
    {
        float valor = 0.0f;
        if (reshade::get_config_value(nullptr, "MiAddon", controles[i].nombre, valor))
            poner(runtime, i, valor);
    }
}

static bool efecto_activo(reshade::api::effect_runtime *runtime, const char *efecto)
{
    bool activo = false;
    runtime->enumerate_techniques(efecto,
        [&](reshade::api::effect_runtime *rt, reshade::api::effect_technique t) {
            if (rt->get_technique_state(t))
                activo = true;
        });
    return activo;
}

static void activar_efecto(reshade::api::effect_runtime *runtime, const char *efecto, bool activar)
{
    runtime->enumerate_techniques(efecto,
        [&](reshade::api::effect_runtime *rt, reshade::api::effect_technique t) {
            rt->set_technique_state(t, activar);
        });
}

static void dibujar_panel(reshade::api::effect_runtime *runtime)
{
    bool color_on = efecto_activo(runtime, "MiColor.fx");
    if (ImGui::Checkbox("Efecto MiColor activado", &color_on))
        activar_efecto(runtime, "MiColor.fx", color_on);

    bool smaa_on = efecto_activo(runtime, "SMAA.fx");
    if (ImGui::Checkbox("Suavizado SMAA", &smaa_on))
        activar_efecto(runtime, "SMAA.fx", smaa_on);

    ImGui::Separator();
    ImGui::TextUnformatted("Presets");
    for (int p = 0; p < 6; ++p)
    {
        if (p % 3 != 0)
            ImGui::SameLine();
        if (ImGui::Button(presets[p].nombre))
            for (int i = 0; i < N; ++i)
                poner(runtime, i, presets[p].valores[i]);
    }

    if (ImGui::Button("Guardar mi preset"))
        guardar_mio(runtime);
    ImGui::SameLine();
    if (ImGui::Button("Cargar mi preset"))
        cargar_mio(runtime);

    ImGui::Separator();

    for (int i = 0; i < N; ++i)
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
