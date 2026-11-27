#include <windows.h>
#include <imgui.h>
#include <reshade.hpp>

extern "C" __declspec(dllexport) const char *NAME = "Mi Add-on";
extern "C" __declspec(dllexport) const char *DESCRIPTION = "Mi primer add-on con panel";

static const int N = 28;   // todas las variables
static const int NP = 19;  // los presets solo tocan las 19 primeras

struct Control { const char *nombre; float min, max; const char *formato; };

static const Control controles[N] = {
    // Color (0 a 12)
    {"Exposicion",    -2.0f,     2.0f, "%.2f"},
    {"Contraste",      0.5f,     1.5f, "%.2f"},
    {"Saturacion",     0.0f,     2.0f, "%.2f"},
    {"Calidez",       -1.0f,     1.0f, "%.2f"},
    {"Temperatura", 3000.0f, 10000.0f, "%.0f"},
    {"TinteR",         0.5f,     1.5f, "%.2f"},
    {"TinteG",         0.5f,     1.5f, "%.2f"},
    {"TinteB",         0.5f,     1.5f, "%.2f"},
    {"Sombras",       -1.0f,     1.0f, "%.2f"},
    {"Luces",         -1.0f,     1.0f, "%.2f"},
    {"Negros",        -1.0f,     1.0f, "%.2f"},
    {"Blancos",       -1.0f,     1.0f, "%.2f"},
    {"DesatLuces",     0.0f,     1.0f, "%.2f"},
    // Imagen (13 a 15)
    {"Nitidez",        0.0f,     1.5f, "%.2f"},
    {"Grano",          0.0f,     0.2f, "%.3f"},
    {"Vineta",         0.0f,     1.0f, "%.2f"},
    // Efectos: bloom (16 a 18)
    {"Bloom",          0.0f,     2.0f, "%.2f"},
    {"BloomUmbral",    0.3f,    0.95f, "%.2f"},
    {"BloomRadio",     1.0f,    30.0f, "%.1f"},
    // Efectos: bruma (19 a 21)
    {"Bruma",          0.0f,     1.0f, "%.2f"},
    {"BrumaInicio",    0.0f,     0.5f, "%.3f"},
    {"BrumaFin",      0.05f,     1.0f, "%.3f"},
    // Efectos: rayos de sol (22 a 27)
    {"RayosIntensidad",0.0f,     2.0f, "%.2f"},
    {"RayosLongitud",  0.1f,     1.0f, "%.2f"},
    {"RayosUmbral",    0.3f,    0.95f, "%.2f"},
    {"SolX",           0.0f,     1.0f, "%.3f"},
    {"SolY",           0.0f,     1.0f, "%.3f"},
    {"Destello",       0.0f,     1.0f, "%.2f"},
};

struct Preset { const char *nombre; float valores[NP]; };

// Orden: Exposicion, Contraste, Saturacion, Calidez, Temperatura,
// TinteR, TinteG, TinteB, Sombras, Luces, Negros, Blancos, DesatLuces,
// Nitidez, Grano, Vineta, Bloom, BloomUmbral, BloomRadio
static const Preset presets[6] = {
    {"Neutro",
     {0.0f, 1.0f, 1.0f, 0.0f, 6500.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 0.0f, 0.7f, 12.0f}},
    {"Dia",
     {0.0f, 1.1f, 1.1f, 0.1f, 6500.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
      0.3f, 0.0f, 0.0f, 0.2f, 0.7f, 12.0f}},
    {"Atardecer",
     {-0.1f, 1.1f, 1.2f, 0.5f, 7500.0f, 1.0f, 1.0f, 1.0f, -0.1f, 0.1f, 0.0f, 0.0f, 0.0f,
      0.2f, 0.0f, 0.2f, 0.5f, 0.6f, 14.0f}},
    {"Noche",
     {0.2f, 1.2f, 0.8f, -0.3f, 5500.0f, 1.0f, 1.0f, 1.0f, 0.2f, -0.1f, 0.1f, 0.0f, 0.0f,
      0.2f, 0.02f, 0.3f, 0.4f, 0.6f, 16.0f}},
    {"Natural",
     {0.0f, 1.05f, 0.95f, 0.05f, 6500.0f, 1.0f, 1.0f, 1.0f, 0.1f, -0.05f, 0.0f, 0.0f, 0.1f,
      0.3f, 0.01f, 0.05f, 0.1f, 0.8f, 10.0f}},
    {"Cinematico",
     {-0.1f, 1.25f, 0.85f, -0.1f, 6200.0f, 1.0f, 1.0f, 1.0f, -0.1f, 0.0f, 0.1f, 0.0f, 0.15f,
      0.4f, 0.03f, 0.35f, 0.4f, 0.7f, 14.0f}},
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

static void dibujar_sliders(reshade::api::effect_runtime *runtime, int desde, int hasta)
{
    for (int i = desde; i < hasta; ++i)
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
        if (ImGui::SliderFloat(controles[i].nombre, &valor,
                               controles[i].min, controles[i].max, controles[i].formato))
            runtime->set_uniform_value_float(var, valor);
    }
}

static void dibujar_color_bruma(reshade::api::effect_runtime *runtime)
{
    reshade::api::effect_uniform_variable var =
        runtime->find_uniform_variable("MiColor.fx", "BrumaColor");
    if (var.handle == 0)
    {
        ImGui::TextDisabled("BrumaColor: no encontrada");
        return;
    }
    float col[3] = {0.0f, 0.0f, 0.0f};
    runtime->get_uniform_value_float(var, col, 3);
    if (ImGui::ColorEdit3("Color de la bruma", col))
        runtime->set_uniform_value_float(var, col, 3);
}

static void estilo_inicio()
{
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 4.0f);
    ImGui::PushStyleColor(ImGuiCol_Button,           ImVec4(0.80f, 0.48f, 0.10f, 0.85f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,    ImVec4(0.95f, 0.60f, 0.15f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,     ImVec4(1.00f, 0.70f, 0.25f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab,       ImVec4(0.95f, 0.60f, 0.15f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(1.00f, 0.75f, 0.30f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_CheckMark,        ImVec4(1.00f, 0.70f, 0.25f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg,          ImVec4(0.14f, 0.14f, 0.16f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_Tab,              ImVec4(0.30f, 0.20f, 0.08f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_TabHovered,       ImVec4(0.95f, 0.60f, 0.15f, 1.00f));
}

static void estilo_fin()
{
    ImGui::PopStyleColor(9);
    ImGui::PopStyleVar(2);
}

static void dibujar_panel(reshade::api::effect_runtime *runtime)
{
    estilo_inicio();

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
            for (int i = 0; i < NP; ++i)
                poner(runtime, i, presets[p].valores[i]);
    }

    if (ImGui::Button("Guardar mi preset"))
        guardar_mio(runtime);
    ImGui::SameLine();
    if (ImGui::Button("Cargar mi preset"))
        cargar_mio(runtime);

    ImGui::Separator();

    if (ImGui::BeginTabBar("pestanas"))
    {
        if (ImGui::BeginTabItem("Color"))
        {
            dibujar_sliders(runtime, 0, 13);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Imagen"))
        {
            dibujar_sliders(runtime, 13, 16);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Efectos"))
        {
            ImGui::TextUnformatted("Bloom");
            dibujar_sliders(runtime, 16, 19);

            ImGui::Separator();
            ImGui::TextUnformatted("Bruma por distancia");
            dibujar_sliders(runtime, 19, 22);
            dibujar_color_bruma(runtime);

            ImGui::Separator();
            ImGui::TextUnformatted("Rayos de sol");
            dibujar_sliders(runtime, 22, 28);

            if (ImGui::Button("Apagar bruma y rayos"))
            {
                poner(runtime, 19, 0.0f);
                poner(runtime, 22, 0.0f);
                poner(runtime, 27, 0.0f);
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    estilo_fin();
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
