// load and save themes

#include "../tinyfd/tinyfiledialogs.h"

template<typename T>
std::vector<T> ExtractNumbers(const std::string& input) {
    std::regex separator("[\\s,]+");
    std::sregex_token_iterator it(input.begin(), input.end(), separator, -1);
    std::sregex_token_iterator end;

    std::vector<T> numbers;
    for (; it != end; ++it) {
        std::istringstream iss(*it);
        T number;
        if (iss >> std::boolalpha >> number)
            numbers.push_back(number);
    }

    return numbers;
}

int global_theme_reset = nk_false;
const std::vector<std::string> nk_color_strings = {
    "NK_COLOR_TEXT",
    "NK_COLOR_WINDOW",
    "NK_COLOR_HEADER",
    "NK_COLOR_BORDER",
    "NK_COLOR_BUTTON",
    "NK_COLOR_BUTTON_HOVER",
    "NK_COLOR_BUTTON_ACTIVE",
    "NK_COLOR_TOGGLE",
    "NK_COLOR_TOGGLE_HOVER",
    "NK_COLOR_TOGGLE_CURSOR",
    "NK_COLOR_SELECT",
    "NK_COLOR_SELECT_ACTIVE",
    "NK_COLOR_SLIDER",
    "NK_COLOR_SLIDER_CURSOR",
    "NK_COLOR_SLIDER_CURSOR_HOVER",
    "NK_COLOR_SLIDER_CURSOR_ACTIVE",
    "NK_COLOR_PROPERTY",
    "NK_COLOR_EDIT",
    "NK_COLOR_EDIT_CURSOR",
    "NK_COLOR_COMBO",
    "NK_COLOR_CHART",
    "NK_COLOR_CHART_COLOR",
    "NK_COLOR_CHART_COLOR_HIGHLIGHT",
    "NK_COLOR_SCROLLBAR",
    "NK_COLOR_SCROLLBAR_CURSOR",
    "NK_COLOR_SCROLLBAR_CURSOR_HOVER",
    "NK_COLOR_SCROLLBAR_CURSOR_ACTIVE",
    "NK_COLOR_TAB_HEADER"
};

std::vector<std::string> nk_color_text = nk_color_strings;
void setup_color_text() {
    for (auto& color : nk_color_text) {
        if (color.size() > 3) {
            color = color.substr(3);
            auto it = color.begin() + 1;
            while (it != color.end()) {
                if (*(it - 1) != ' ') {
                    if (*it == '_') {
                        *it = ' ';
                    }
                    else *it = std::tolower(*it);
                }
                ++it;
            }
        }
    }
}

struct nk_color theme[NK_COLOR_COUNT];
nk_colorf color_float[NK_COLOR_COUNT];
bool theme_initialized = false;
static int reset_bgcolor_popup = nk_false;
static int reset_appcolor_popup[static_cast<int>(nk_style_colors::NK_COLOR_COUNT)];
static int appcolorpicker_popup[static_cast<int>(nk_style_colors::NK_COLOR_COUNT)];
static int appbg_colorpicker_popup = nk_false;
static struct nk_colorf bg = { DEFAULT_BG_COLOR_RED, DEFAULT_BG_COLOR_GREEN, DEFAULT_BG_COLOR_BLUE, DEFAULT_COLOR_ALPHA * 255 };
struct nk_color default_theme[NK_COLOR_COUNT];

void ResetTheme()
{
    bg = { DEFAULT_BG_COLOR_RED, DEFAULT_BG_COLOR_GREEN, DEFAULT_BG_COLOR_BLUE, DEFAULT_COLOR_ALPHA * 255 };
    for (int i = 0; i < NK_COLOR_COUNT; i++)
        theme[i] = default_theme[i];
}

void SetupDefaultTheme() {
    default_theme[NK_COLOR_TEXT] = nk_rgba(210, 210, 210, 255);
    default_theme[NK_COLOR_WINDOW] = nk_rgba(57, 67, 71, 235);
    default_theme[NK_COLOR_HEADER] = nk_rgba(51, 51, 56, 220);
    default_theme[NK_COLOR_BORDER] = nk_rgba(46, 46, 46, 255);
    default_theme[NK_COLOR_BUTTON] = nk_rgba(28, 33, 31, 215);
    default_theme[NK_COLOR_BUTTON_HOVER] = nk_rgba(58, 58, 58, 255);
    default_theme[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(67, 67, 67, 255);
    default_theme[NK_COLOR_TOGGLE] = nk_rgba(50, 58, 61, 255);
    default_theme[NK_COLOR_TOGGLE_HOVER] = nk_rgba(45, 53, 56, 255);
    default_theme[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(185, 185, 185, 255);
    default_theme[NK_COLOR_SELECT] = nk_rgba(57, 67, 61, 255);
    default_theme[NK_COLOR_SELECT_ACTIVE] = nk_rgba(185, 185, 185, 255);
    default_theme[NK_COLOR_SLIDER] = nk_rgba(40, 48, 51, 255);
    default_theme[NK_COLOR_SLIDER_CURSOR] = nk_rgba(20, 28, 21, 215);
    default_theme[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(85, 85, 85, 255);
    default_theme[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(185, 185, 185, 255);
    default_theme[NK_COLOR_PROPERTY] = nk_rgba(50, 58, 61, 255);
    default_theme[NK_COLOR_EDIT] = nk_rgba(50, 58, 61, 225);
    default_theme[NK_COLOR_EDIT_CURSOR] = nk_rgba(210, 210, 210, 255);
    default_theme[NK_COLOR_COMBO] = nk_rgba(50, 58, 61, 255);
    default_theme[NK_COLOR_CHART] = nk_rgba(50, 58, 61, 255);
    default_theme[NK_COLOR_CHART_COLOR] = nk_rgba(48, 83, 111, 255);
    default_theme[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 0, 0, 255);
    default_theme[NK_COLOR_SCROLLBAR] = nk_rgba(28, 23, 21, 255);
    default_theme[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(50, 58, 61, 255);
    default_theme[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(85, 85, 85, 255);
    default_theme[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(185, 185, 185, 255);
    default_theme[NK_COLOR_TAB_HEADER] = nk_rgba(28, 23, 21, 255);

    ResetTheme(); // copy default theme to the global theme
    setup_color_text();
}

int loadTheme(const char* fname) {
    portini::Document doc;
    if (!doc.ParseFromFile(fname)) {
        std::ostringstream oss;
        oss << "Failed to load " << fname << std::endl;
        const std::string result = oss.str();
        tinyfd_messageBox("Error", result.c_str(), "ok", "error", 1);
        return 0;
    }

    if (!doc.HasSection("theme")) {
        std::ostringstream oss;
        oss << "Missing [theme] section in " << fname << std::endl;
        const std::string result = oss.str();
        tinyfd_messageBox("Error", result.c_str(), "ok", "error", 1);
        return 0;
    }
    portini::Section& themeSection = doc.GetSection("theme");

    int idx = 0;
    for (const auto& color : nk_color_strings) {
        if (!themeSection.HasKey(nk_color_strings.at(idx))) {
            std::ostringstream oss;
            oss << "Missing key " << color << " in [theme] section" << std::endl;
            const std::string result = oss.str();
            tinyfd_messageBox("Error", result.c_str(), "ok", "error", 1);
            return 0;
        }
        portini::Key key = themeSection.GetKey(color);
        std::string value = key.GetValue();
        std::istringstream iss(value);
        // Extract integers
        std::vector<int> integers = ExtractNumbers<int>(value);

        // Printing the extracted integers
        auto amount = integers.size();
        if (amount != 4)
        {
            std::ostringstream oss;
            oss << "Invalid value " << value << " for key 创" << color << "`` in [theme] section" << std::endl;
            const std::string result = oss.str();
            tinyfd_messageBox("Error", result.c_str(), "ok", "error", 1);
            return 0;
        }
        for (int i = 0; i < amount; ++i) {
            if (integers[i] > 255 || integers[i] < 0)
            {
                std::ostringstream oss;
                oss << "Invalid value " << value << " for key 创" << color << "`` in [theme] section" << std::endl << "Out-of-range: " << i << "=" << integers[i] << std::endl;
                const std::string result = oss.str();
                tinyfd_messageBox("Error", result.c_str(), "ok", "error", 1);
                return 0;
            }
        }

        theme[idx].r = integers[0]; // Red
        theme[idx].g = integers[1]; // Green
        theme[idx].b = integers[2]; // Blue
        theme[idx].a = integers[3]; // Alpha
        idx++;
    }

    if (!doc.HasSection("background")) {
        std::ostringstream oss;
        oss << "Missing [background] section in " << fname << std::endl;
        const std::string result = oss.str();
        tinyfd_messageBox("Error", result.c_str(), "ok", "error", 1);
        return 0;
    }
    portini::Section& backgroundSection = doc.GetSection("background");

    if (!backgroundSection.HasKey("bg")) {
        tinyfd_messageBox("Error", "Missing key 创bg`` in [background] section", "ok", "error", 1);
        return 0;
    }
    const portini::Key bgKey = backgroundSection.GetKey("bg");

    // Extract floats
    std::string bgValue = bgKey.GetValue();
    std::vector<float> floats = ExtractNumbers<float>(bgValue);

    auto bgfloats = floats.size();
    if (bgfloats != 4 && bgfloats != 3) {
        std::ostringstream oss;
        oss << "Invalid value " << bgValue << " for key 创bg`` in [background] section" << std::endl;
        const std::string result = oss.str();
        tinyfd_messageBox("Error", result.c_str(), "ok", "error", 1);
        return 0;
    }

    for (int i = 0; i < 3; ++i) {
        // skip 'alpha' as it has no effect here
        if (floats[i] > 1.1f || floats[i] < -0.1f)
        {
            std::ostringstream oss;
            oss << "Invalid value " << floats[i] << " for key 创bg`` in [background] section" << std::endl << "Out-of-range: " << i << "=" << floats[i] << std::endl;
            const std::string result = oss.str();
            tinyfd_messageBox("Error", result.c_str(), "ok", "error", 1);
            return 0;
        }
    }

    bg.r = floats[0];
    bg.g = floats[1];
    bg.b = floats[2];
    bg.a = floats[3];

    return 1;
}
void saveTheme(const char* fname)
{
    portini::Document doc;
    portini::Section& themeSection = doc.CreateSection("theme");

    int i = 0;
    for (auto& color : nk_color_strings) {
        std::string key = color;
        std::stringstream value;
        value << std::to_string(theme[i].r) << ", ";
        value << std::to_string(theme[i].g) << ", ";
        value << std::to_string(theme[i].b) << ", ";
        value << std::to_string(theme[i].a);
        themeSection.CreateKey(key) = value.str();
        i++;
    }

    portini::Section& backgroundSection = doc.CreateSection("background");

    std::stringstream value;
    value << std::to_string(bg.r) << ", ";
    value << std::to_string(bg.g) << ", ";
    value << std::to_string(bg.b) << ", ";
    value << std::to_string(bg.a);
    backgroundSection.CreateKey("bg") = value.str();

    doc.SerializeToFile(fname);
}

void ResetThemeColor(int& color_idx, struct nk_context* ctx)
{
    theme[color_idx] = default_theme[color_idx];
}