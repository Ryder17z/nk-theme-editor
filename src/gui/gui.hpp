/* macros */
#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

#define UNUSED(a) (void)a
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#define LEN(a) (sizeof(a)/sizeof(a)[0])

#ifdef __APPLE__
#define NK_SHADER_VERSION "#version 150\n"
#else
#define NK_SHADER_VERSION "#version 300 es\n"
#endif

#include <filesystem>
#include <SDL.h>
#include "nk_setup.hpp"
#include "nuklear.h"
#include "nuklear_sdl_renderer.h"

#include "../tinyfd/tinyfiledialogs.h"

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800

#define WINDOW_MIN_WIDTH 960
#define WINDOW_MIN_HEIGHT 660

#define DEFAULT_BG_COLOR_RED 0.17
#define DEFAULT_BG_COLOR_GREEN 0.21
#define DEFAULT_BG_COLOR_BLUE 0.24
#define DEFAULT_COLOR_ALPHA 1.0

constexpr double margin_left = 0.015;
constexpr double margin_top = 0.015;
constexpr double margin_right = 0.015;
constexpr double margin_bottom = 0.015;
float side_panel_width = 0.285;
float side_panel_margin = 0.01;
float middle_panel_pos;
float middle_panel_width;

double borders[4];
int prev_window_w = WINDOW_MIN_WIDTH, prev_window_h = WINDOW_MIN_HEIGHT;
int ui_panel_height;
int color_picker_height;

#include "themes.hpp"

auto selectedPath = std::filesystem::current_path().string();
auto WorkingDir = std::filesystem::current_path().string();
const char* BaseFilename;

void ButtonColorRect(struct nk_context* ctx, struct nk_color color) {

    auto const original_style = ctx->style.button;

    struct nk_style_button button_style = ctx->style.button;
    button_style.normal = nk_style_item_color(color);
    button_style.hover = nk_style_item_color(color);
    button_style.active = nk_style_item_color(color);
    button_style.text_normal = nk_rgb(255, 255, 255); // White text color
    button_style.text_hover = nk_rgb(255, 255, 255); // White text color
    button_style.text_active = nk_rgb(255, 255, 255); // White text color
    button_style.border_color = nk_rgb(0, 0, 0); // Black border color
    button_style.rounding = 0; // No rounding
    ctx->style.button = button_style;
    nk_button_symbol(ctx, NK_SYMBOL_PLUS);

    ctx->style.button = button_style;

    ctx->style.button = original_style;
}

int AppSettings(struct nk_context* ctx, nk_colorf& bg);
int ThemeColorPicker(struct nk_context* ctx, int color_idx);
std::string themeFile; // path to currently used theme file
int settings_popup = nk_false;

void SaveSettings() {
    portini::Document document;

    auto& themeSection = document.CreateSection("theme");
    themeSection.CreateKey("file");
    themeSection["file"] = themeFile;

    if (!document.SerializeToFile("config.ini")) {
        std::ostringstream oss;
        oss << "Failed to save data to file." << std::endl;
        const std::string result = oss.str();
        tinyfd_messageBox("Error", result.c_str(), "ok", "error", 1);
    }
}

int LoadSettings() {
    portini::Document doc;
    if (doc.ParseFromFile("config.ini")) {
        // Access the loaded data
        if (doc.HasSection("theme")) { // not required
            portini::Section& themeSection = doc.GetSection("theme");
            portini::Key key = themeSection.GetKey("file");
            themeFile = key.GetValue();

            if (themeFile.length() <= 0)
                return 1; // use default theme instead

            std::filesystem::path currentPath = std::filesystem::current_path();
            std::string themeFilename = currentPath.string() + "/themes/" + themeFile;

            if (!loadTheme(themeFilename.c_str()))
            {
                themeFile = "";
                SaveSettings(); // Reset settings
            }
        }
        return 1;
    }
    else {
        std::ostringstream oss;
        oss << "Failed to load data from config.ini - Settings will be reset." << std::endl;
        const std::string result = oss.str();
        tinyfd_messageBox("Error", result.c_str(), "ok", "error", 1);
    }
    return 0;
}

// 0-255 scaled to range: 0 - 1.0
const float step_normalized = 1.0 / 256;

void tinyfd_ColorPicker_Popup(nk_colorf & color)
{
    unsigned char lRgbColor[3];
    char const* lTheHexColor;
    lTheHexColor = tinyfd_colorChooser("choose a nice color", "#FF0077", lRgbColor, lRgbColor);

    if (!lTheHexColor)
    {
        tinyfd_messageBox("Error", "hexcolor is invalid.", "ok", "error", 1);
        return;
    }

    if (lTheHexColor)
    {
        color.r = lRgbColor[0] / 255.0;
        color.g = lRgbColor[1] / 255.0;
        color.b = lRgbColor[2] / 255.0;
    }
}

void ResetColor_Popup(struct nk_context* ctx, nk_colorf& color, int& popup_state, float vpos=-1.0f, bool is_nktheme_color = false, int color_idx = -1) {
    static struct nk_rect s = { 20, (float)(ui_panel_height * 0.75), 220, 90 };  

    if (vpos >= 0.0)
        s.y = vpos;

    if (nk_button_label(ctx, "Reset"))
        popup_state = 1;

    if (popup_state)
    {
        if (nk_popup_begin(ctx, NK_POPUP_STATIC, "Confirm Reset?", NK_WINDOW_TITLE, s))
        {
            nk_layout_row_dynamic(ctx, 25, 2);
            if (nk_button_label(ctx, "OK")) {        

                printf("reset color, idx: %d, is_nktheme_color: %d\n", color_idx, is_nktheme_color);

                if (is_nktheme_color == false || (color_idx == NK_COLOR_COUNT)) {
                    color.r = DEFAULT_BG_COLOR_RED;
                    color.g = DEFAULT_BG_COLOR_GREEN;
                    color.b = DEFAULT_BG_COLOR_BLUE;
                    color.a = DEFAULT_COLOR_ALPHA;
                }
                else ResetThemeColor(color_idx, ctx);

                popup_state = 0;
                nk_popup_close(ctx);
            }
            if (nk_button_label(ctx, "Cancel")) {
                popup_state = 0;
                nk_popup_close(ctx);
            }
            nk_popup_end(ctx);
        }
        else popup_state = nk_false;
    }
}

void ColorPicker_Widget(struct nk_context* ctx, nk_colorf& color, char& hexstring, int& hexlen, int& popup_state, nk_color_format color_fmt = NK_RGB, bool OK_Button = false, bool is_theme_color = false, int color_idx = -1, bool no_reset=false)
{
    nk_layout_row_dynamic(ctx, color_picker_height, 1);
    color = nk_color_picker(ctx, color, color_fmt);
    float ratios[] = { 0.15f, 0.70f, 0.15f };

    nk_layout_row_dynamic(ctx, 3, 1); // spacer
    static char textrgb[4][8];

    sprintf(textrgb[0], "%0.0f", color.r * 255.0f);
    sprintf(textrgb[1], "%0.0f", color.g * 255.0f);
    sprintf(textrgb[2], "%0.0f", color.b * 255.0f);
    
    // swapping out buggy input fields for robust sliders
    nk_layout_row(ctx, NK_DYNAMIC, 25, 3, ratios);
    nk_label(ctx, "R:", NK_TEXT_LEFT);
    color.r = (nk_slide_float(ctx, 0.0f, color.r, 1.0f, step_normalized));
    nk_label(ctx, textrgb[0], NK_TEXT_LEFT);
    nk_label(ctx, "G:", NK_TEXT_LEFT);
    color.g = (nk_slide_float(ctx, 0.0f, color.g, 1.0f, step_normalized));
    nk_label(ctx, textrgb[1], NK_TEXT_LEFT);
    nk_label(ctx, "B:", NK_TEXT_LEFT);
    color.b = (nk_slide_float(ctx, 0.0f, color.b, 1.0f, step_normalized));
    nk_label(ctx, textrgb[2], NK_TEXT_LEFT);

    if( color_fmt == NK_RGBA)
    {
        sprintf(textrgb[3], "%0.2f", color.a);
        nk_label(ctx, "A:", NK_TEXT_LEFT);
        color.a = nk_slide_float(ctx, 0.0f, color.a, 1.0, 0.05f);
        nk_label(ctx, textrgb[3], NK_TEXT_LEFT);
    }
    
    if(OK_Button && color_idx >= 0)
    {
        float ratio_three[] = { 0.10f, 0.70f, 0.20f };
        nk_layout_row(ctx, NK_DYNAMIC, 30, 3, ratio_three);
        if (nk_button_label(ctx, "OK"))
        {
            if (color_idx < NK_COLOR_COUNT) appcolorpicker_popup[color_idx] = false;
            else appbg_colorpicker_popup = false;
        }
    }
    else
    {
        float ratio_two[] = { 0.74f, 0.25f };
        nk_layout_row(ctx, NK_DYNAMIC, 30, 2, ratio_two);
    }

    if (nk_button_label(ctx, "Use system color picker"))
    {
        tinyfd_ColorPicker_Popup(color);
    }

    if (no_reset) return; // temporary work-around: A bug is preventing resetting of individual parts of the theme. For now it's all or nothing.
    struct nk_rect s = { middle_panel_pos, (float)(ui_panel_height * 0.75), 220, 90 };
    ResetColor_Popup(ctx, color, popup_state, color_picker_height, is_theme_color, color_idx);
}

void ColorPanel_Widget(struct nk_context* ctx, nk_colorf &color1, char& hexstring, int& hexlen, nk_colorf &color2, struct InvertOptions &filterFX, int &swapfx, int& popup_state, int color_idx = -1)
{
    nk_layout_row_dynamic(ctx, 3, 1); // spacer
    float ratio_two[] = { 0.74f, 0.25f };
    ColorPicker_Widget(ctx, color1, hexstring, hexlen, popup_state, NK_RGB, false, false, color_idx);
}

int maingui(struct nk_context* ctx, SDL_Window* win, SDL_Renderer* renderer) {

    if(!theme_initialized) {
        theme_initialized = true;

        SetupDefaultTheme();

        if (!LoadSettings())
            SaveSettings(); // create a default config.ini

        selectedPath = std::filesystem::current_path().string().c_str();
        WorkingDir = std::filesystem::current_path().string().c_str();
    }

    SDL_SetRenderDrawColor(renderer, bg.r * 255, bg.g * 255, bg.b * 255, DEFAULT_COLOR_ALPHA);

    int winflags;
    if(settings_popup)
        winflags = NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_NOT_INTERACTIVE;
    else
        winflags = NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_MOVABLE;

    float ratio_three[] = { 0.25f, 0.50f, 0.25f };

    int window_w, window_h;
    SDL_GetWindowSize(win, &window_w, &window_h);

    if (window_w != prev_window_w || window_h != prev_window_h) {
        // Window size has changed
        prev_window_w = window_w;
        prev_window_h = window_h;

        borders[0] = window_w * margin_left;
        borders[1] = window_h * margin_top;
        borders[2] = window_w * ((1 - margin_right) - margin_left);
        borders[3] = window_h * ((1 - margin_bottom) - margin_top);

        ui_panel_height = borders[3] - (borders[1] * 6);
        color_picker_height = ui_panel_height - (borders[1] * 40); // roughly 40%

        middle_panel_pos = side_panel_width + margin_left;
        middle_panel_width = 1 - (((side_panel_width * 2) + margin_right) + (side_panel_margin / 2));

        // Perform the calculations based on the new window size
        // printf("Borders: %.2f %.2f %.2f %.2f\n", borders[0], borders[1], borders[2], borders[3]);
    }

    if (nk_begin(ctx, "MainGUI", nk_rect(borders[2] * 0.75, borders[1], borders[2] * 0.25f, borders[3] * 0.25f), winflags))
    {
        nk_style_from_table(ctx, theme);

        nk_layout_row_dynamic(ctx, 40, 3);
        if (nk_button_label(ctx, "Settings"))
            settings_popup = nk_true;
    }
    nk_end(ctx);

    if (settings_popup)
    {
        AppSettings(ctx, bg);
        
        if (appbg_colorpicker_popup)
            ThemeColorPicker(ctx, NK_COLOR_COUNT);
        else
        {
            for (int i = 0; i <= NK_COLOR_COUNT; i++)
            {
                if (appcolorpicker_popup[i])
                    ThemeColorPicker(ctx, i);
            }
        }
    }
        
    return !nk_window_is_closed(ctx, "MainGUI");
}

int ThemeColorPicker(struct nk_context* ctx, int color_idx)
{
    static char hexstring[NK_COLOR_COUNT][10];
    static char bghexstring[8];
    static int bghexlen = 8;
    float settings_width = borders[2] * (middle_panel_width * 0.75);

    if (nk_begin(ctx, "Theme Color Picker", nk_rect((borders[2] - settings_width) * 0.5, borders[1], settings_width, ui_panel_height * 0.75), NK_WINDOW_CLOSABLE | NK_WINDOW_NO_SCROLLBAR))
    {
        if (color_idx == NK_COLOR_COUNT)
        {
            ColorPicker_Widget(ctx, bg, *bghexstring, bghexlen, reset_bgcolor_popup, NK_RGB, true, false, color_idx);
        }
        else
        {
            int hexlen = 10;
            ColorPicker_Widget(ctx, color_float[color_idx], *hexstring[color_idx], hexlen, reset_appcolor_popup[color_idx], NK_RGBA, true, true, color_idx, true);
            theme[color_idx].r = color_float[color_idx].r * 255.0;
            theme[color_idx].g = color_float[color_idx].g * 255.0;
            theme[color_idx].b = color_float[color_idx].b * 255.0;
            theme[color_idx].a = color_float[color_idx].a * 255.0;
        }
    }
    else appcolorpicker_popup[color_idx] = false;
    nk_end(ctx);
    return !nk_window_is_closed(ctx, "Theme Color Picker");
}

int AppSettings(struct nk_context* ctx, nk_colorf& bg)
{
    const int idx_middle = NK_COLOR_COUNT / 2;
    float settings_width = borders[2] * (middle_panel_width * 1.75);
    float ratios[] = { 0.75f, 0.24f };

    for (int i = 0; i < NK_COLOR_COUNT; i++)
    {
        color_float[i].r = theme[i].r / 255.0;
        color_float[i].g = theme[i].g / 255.0;
        color_float[i].b = theme[i].b / 255.0;
        color_float[i].a = theme[i].a / 255.0;
    }

    if (nk_begin(ctx, "Settings", nk_rect( (borders[2]-settings_width) * 0.5, borders[1], settings_width, ui_panel_height), NK_WINDOW_CLOSABLE | NK_WINDOW_NO_SCROLLBAR))
    {
        nk_layout_row_dynamic(ctx, 40, 3);
        nk_label(ctx, "", NK_TEXT_ALIGN_CENTERED); // spacer
        if (nk_group_begin(ctx, "ThemeButtons", NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR))
        {
            nk_layout_row_dynamic(ctx, 30, 4);
            nk_label(ctx, "Theme", NK_TEXT_ALIGN_CENTERED | NK_TEXT_ALIGN_MIDDLE);
            if (nk_button_label(ctx, "Save")) {
                std::filesystem::path currentPath = std::filesystem::current_path();
                std::string themeFilename = currentPath.string() + "/theme.ini";
                const char* filepath = themeFilename.c_str();
                char const* lTheSaveFileName;
                const char* lFilterPatterns[1] = { "*.ini" };
                lTheSaveFileName = tinyfd_saveFileDialog( "Save theme as...", filepath, 1, lFilterPatterns, "INI Theme File");

                if (!lTheSaveFileName)
                {
                    tinyfd_messageBox( "Error", "Save file name is invalid.", "ok", "error", 1);
                }
                else saveTheme(lTheSaveFileName);
            }
            if (nk_button_label(ctx, "Load")) {
                std::filesystem::path currentPath = std::filesystem::current_path();
                std::string themeFilename = currentPath.string() + "/theme.ini";
                const char* filepath = themeFilename.c_str();
                char const* lTheOpenFileName;
                const char* lFilterPatterns[1] = { "*.ini" };
                lTheOpenFileName = tinyfd_openFileDialog( "Load theme", filepath, 1, lFilterPatterns, "INI Theme File", 0);

                if (!lTheOpenFileName)
                {
                    tinyfd_messageBox("Error", "Open file name is invalid.", "ok", "error", 0);
                }
                else
                {
                    if (!loadTheme(lTheOpenFileName))
                    {
                        ResetTheme(); // Revert to default
                        themeFile = "";
                        SaveSettings(); // Reset settings
                    }
                    else
                    {
                        std::filesystem::path filePath(lTheOpenFileName);
                        std::filesystem::path fileName = filePath.filename();
                        themeFile = fileName.string();
                        SaveSettings();
                    }
                }
            }
            if (nk_button_label(ctx, "Reset") || global_theme_reset) {
                global_theme_reset = nk_true;
                static struct nk_rect s = { 20, (float)(ui_panel_height * 0.01), 220, 90 };
                if (nk_popup_begin(ctx, NK_POPUP_STATIC, "Reset theme?", NK_WINDOW_TITLE, s))
                {
                    nk_layout_row_dynamic(ctx, 25, 2);
                    if (nk_button_label(ctx, "OK")) {
                        ResetTheme();
                        global_theme_reset = nk_false;
                        nk_popup_close(ctx);
                    }
                    if (nk_button_label(ctx, "Cancel")) {
                        global_theme_reset = nk_false;
                        nk_popup_close(ctx);
                    }
                    nk_popup_end(ctx);
                }
                else global_theme_reset = nk_false;
            }
            nk_group_end(ctx);
        }
        nk_label(ctx, "", NK_TEXT_ALIGN_CENTERED); // spacer

        float ratio_two[] = { 0.5f, 0.5f };
        nk_layout_row(ctx, NK_DYNAMIC, ui_panel_height*0.85, 2, ratio_two);

        const std::size_t vectorSize = nk_color_text.size();
        const std::size_t halfSize = vectorSize / 2;

        if (nk_group_begin(ctx, "SettingsLeft", NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
            nk_layout_row(ctx, NK_DYNAMIC, 25, 2, ratios);
            nk_label(ctx, "Application Background Color", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
            if (nk_button_color(ctx, nk_rgba_cf(bg)) || appbg_colorpicker_popup)
            {
                appbg_colorpicker_popup = true;
            }

            for (std::size_t i = 0; i < halfSize; ++i) {
                nk_layout_row(ctx, NK_DYNAMIC, 25, 2, ratios);
                nk_label(ctx, nk_color_text[i].c_str(), NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
                if (nk_button_color(ctx, theme[i]))
                    appcolorpicker_popup[i] = true;
            }

            nk_group_end(ctx);
        }
        if (nk_group_begin(ctx, "SettingsRight", NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
            //nk_layout_row_dynamic(ctx, 20, 1);
            for (std::size_t i = halfSize; i < vectorSize; ++i) {
                nk_layout_row(ctx, NK_DYNAMIC, 25, 2, ratios);
                nk_label(ctx, nk_color_text[i].c_str(), NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
                if (nk_button_color(ctx, theme[i]))
                    appcolorpicker_popup[i] = true;
            }
            nk_layout_row_dynamic(ctx, 20, 1);        
            nk_label(ctx, "* Note: Some color slots are", NK_TEXT_ALIGN_CENTERED);
            nk_label(ctx, "  unused in this application.", NK_TEXT_ALIGN_CENTERED); // manually wrapping the text as nk_label_wrap() has no effect here
            nk_group_end(ctx);
        }
    }
    else settings_popup = nk_false;
    
    nk_end(ctx);
    return !nk_window_is_closed(ctx, "Settings");
}
