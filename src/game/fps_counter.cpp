// Frame rate (top left) and the main menu watermark, drawn by the game in its small font over the finished frame.
#include <algorithm>
#include <bit>
#include <cstdio>
#include <cstring>
#include <optional>
#include <string_view>
#include <vector>

#include "app/app.h"
#include "common/aspect.h"
#include "common/options.h"
#include "game/flash.h"
#include "game/layout.h"
#include "common/subtitles.h"
#include "common/subtitles_data.h"
#include "xdk/hle.h"
#include "xdk/layout.h"

using namespace kernel;
using namespace layout;
using flash::kClipNext;
using flash::kClipX;
using flash::kClipY;

namespace {

// the game's 2D layer: Text(font, wide text, x, y, size, rgba floats), viewport pixels from the top left
constexpr uint32_t kBegin = 0xD99A0;
constexpr uint32_t kText = 0xD9600;
constexpr uint32_t kFlush = 0xD4C50;
constexpr uint32_t kEnd = 0xD3740;
constexpr uint32_t kApplyStates = 0x29EF0;
constexpr uint32_t kQuads = 0x2F3710;
constexpr uint32_t kTexture = 0x56EF30;
constexpr uint32_t kFont = 0x2F433C;  // the small font

// on a frame 480 high
constexpr float kSize = 15;
constexpr float kMargin = 8;
constexpr float kGrey = 186 / 255.0f;  // the HUD's text

// as SCDA: at rest it ends 80 in from the right, on the row of the menu's button legend
constexpr char kWatermark[] = "XBE Recomp V1";
constexpr float kWatermarkRight = 80;
constexpr float kWatermarkTop = 427;
// the page's Main slides its legend in from the right and back out; x is the confirm prompt's at rest (frame 29)
struct TitlePage {
    const char* name;
    float legend_rest_x;
};
// longest name first, they are compared by prefix
constexpr TitlePage kTitlePages[] = {{"BlackTitleMenu2", -44.0f}, {"BlackTitleMenu", -53.5f}};

struct View {
    float unit;    // viewport pixels per line of a 480 frame
    float narrow;  // a wide frame is shown stretched: glyphs are drawn narrower by as much
};

// the glyph 0xD9600 draws for a code
uint32_t Glyph(uint32_t font, uint16_t code) {
    uint32_t glyph = R32(font + kFontGlyphs + (code & 0x7F) * 4);
    uint32_t last = R32(font + kFontLastGlyph);
    while (G<uint16_t>(glyph + kGlyphCode) != code && glyph != last) glyph += kGlyphSize;
    return glyph;
}

// in lines of a 480 frame, at kSize
float TextWidth(uint32_t font, const char* text) {
    float advance = 0;
    for (const char* c = text; *c; c++) advance += G<float>(Glyph(font, uint8_t(*c)) + kGlyphAdvance);
    return advance * G<float>(font + kFontScale) * kSize;
}

float TextWidth(uint32_t font, const std::vector<uint16_t>& text) {
    float advance = 0;
    for (uint16_t code : text) advance += G<float>(Glyph(font, code) + kGlyphAdvance);
    return advance * G<float>(font + kFontScale) * kSize * float(game::Opt().subtitle_size) / 100.0f;
}

// x and y in lines of a 480 frame from the output's top left
void DrawText(X86Context& ctx, uint32_t font, const View& view, const char* text, float x, float y, bool shadow) {
    static const uint32_t block = mem::AllocSystem(0x60);
    uint32_t colour = block;
    uint32_t chars = block + 0x10;
    int n = std::min(int(std::strlen(text)), 31);
    for (int i = 0; i <= n; i++) G<uint16_t>(chars + i * 2) = i < n ? uint8_t(text[i]) : 0;
    for (int s = shadow; s >= 0; s--) {
        float rgb = s ? 0.0f : kGrey;  // the shadow black
        for (int i = 0; i < 4; i++) G<float>(colour + i * 4) = i == 3 ? 1.0f : rgb;
        float px = (x + s) * view.unit * view.narrow;
        float py = (y + s) * view.unit;
        CallGuest(ctx, kText, {font, chars, std::bit_cast<uint32_t>(px), std::bit_cast<uint32_t>(py),
                               std::bit_cast<uint32_t>(kSize * view.unit), colour});
    }
}

void DrawCaptionLine(X86Context& ctx, uint32_t font, const View& view,
                     const std::vector<uint16_t>& text, float x, float y, bool bold,
                     uint32_t colour, uint32_t shadow) {
    if (text.empty()) return;
    static const uint32_t block = mem::AllocSystem(0x1000);
    const uint32_t rgba = block;
    const uint32_t chars = block + 0x10;
    const size_t count = std::min<size_t>(text.size(), 500);
    for (size_t i = 0; i <= count; ++i) G<uint16_t>(chars + i * 2) = i < count ? text[i] : 0;
    const float scale = float(game::Opt().subtitle_size) / 100.0f;
    const float native_size = kSize * scale;
    const float px = x * view.unit * view.narrow;
    const float py = y * view.unit;
    const float shadow_offset = shadow == 2 ? 1.5f : shadow == 1 ? 0.75f : 0.0f;
    auto draw = [&](float dx, float dy, float r, float g, float b, float a) {
        G<float>(rgba) = r; G<float>(rgba + 4) = g; G<float>(rgba + 8) = b; G<float>(rgba + 12) = a;
        CallGuest(ctx, kText, {font, chars, std::bit_cast<uint32_t>(px + dx * view.unit),
            std::bit_cast<uint32_t>(py + dy * view.unit), std::bit_cast<uint32_t>(native_size * view.unit), rgba});
    };
    if (shadow_offset > 0) draw(shadow_offset, shadow_offset, 0, 0, 0, 1);
    const float r = colour == 1 ? 1.0f : 1.0f;
    const float g = colour == 1 ? 0.88f : 1.0f;
    const float b = colour == 1 ? 0.38f : 1.0f;
    draw(0, 0, r, g, b, 1);
    if (bold) draw(0.35f, 0, r, g, b, 1);
}

std::vector<std::vector<uint16_t>> WrapCaption(uint32_t font, std::string_view utf8, float max_width) {
    const auto decoded = subtitles::DecodeUtf8(utf8);
    std::vector<std::vector<uint16_t>> lines;
    std::vector<uint16_t> line, word;
    auto append_word = [&] {
        if (word.empty()) return;
        if (!line.empty()) line.push_back(' ');
        for (uint16_t code : word) {
            line.push_back(code);
            if (line.size() > 1 && TextWidth(font, line) > max_width) {
                const uint16_t last = line.back();
                line.pop_back();
                if (!line.empty()) lines.push_back(std::move(line));
                line.clear();
                line.push_back(last);
            }
        }
        word.clear();
    };
    auto flush_word = [&] {
        if (word.empty()) return;
        auto candidate = line;
        if (!candidate.empty()) candidate.push_back(' ');
        candidate.insert(candidate.end(), word.begin(), word.end());
        if (!line.empty() && TextWidth(font, candidate) > max_width) {
            lines.push_back(std::move(line));
            line.clear();
        }
        append_word();
    };
    for (uint16_t code : decoded) {
        if (code == '\n') { flush_word(); lines.push_back(std::move(line)); line.clear(); }
        else if (code == ' ' || code == '\t') flush_word();
        else word.push_back(code);
    }
    flush_word();
    if (!line.empty()) lines.push_back(std::move(line));
    return lines;
}

const TitlePage* CurrentTitlePage() {
    uint32_t ui = R32(kUi);
    if (!ui) return nullptr;
    const char* name = static_cast<const char*>(HostPtr(ui + kPageName));
    for (const TitlePage& page : kTitlePages)
        if (_strnicmp(name, page.name, std::strlen(page.name)) == 0) return &page;
    return nullptr;
}

uint32_t Child(uint32_t clip, const char* name) {
    int left = 4096;
    for (uint32_t child = flash::FirstChild(clip); child && left; child = R32(child + kClipNext), left--)
        if (flash::IsClip(child) && _stricmp(flash::Name(child), name) == 0) return child;
    return 0;
}

// a prompt is a clip of a "Button" icon and a "Text"; the lowest one confirms
uint32_t ConfirmPrompt(uint32_t main) {
    uint32_t found = 0;
    int left = 4096;
    for (uint32_t child = flash::FirstChild(main); child && left; child = R32(child + kClipNext), left--) {
        if (!flash::IsClip(child) || !Child(child, "Button") || !Child(child, "Text")) continue;
        if (!found || G<float>(child + kClipY) > G<float>(found + kClipY)) found = child;
    }
    return found;
}

// how far the legend is from its rest, in lines of a 480 frame like a stage pixel; none while the page has no legend
std::optional<float> LegendSlide(const TitlePage& page) {
    uint32_t prompt = ConfirmPrompt(Child(flash::Level(flash::kMenuLevel), "Main"));
    if (!prompt) return std::nullopt;
    return G<float>(prompt + kClipX) - page.legend_rest_x;
}

}  // namespace

// D3DVIEWPORT8
constexpr uint32_t kViewportWidth = 8;
constexpr uint32_t kViewportHeight = 12;

// the back buffer is the target here, the frame is shown next
void DrawOverlay(X86Context& ctx, uint8_t*) {
    uint32_t font = R32(kFont);
    uint32_t viewport = SYM(D3D_Device) + XDK_CDevice_Viewport;
    float w = float(R32(viewport + kViewportWidth));
    float h = float(R32(viewport + kViewportHeight));
    bool fps = game::Opt().fps_counter;
    std::string caption = game::Opt().subtitles_enabled ? app::CurrentSubtitle() : std::string{};
    const app::SubtitlePreview preview = app::CurrentSubtitlePreview();
    const TitlePage* title = CurrentTitlePage();
    std::optional<float> slide = title ? LegendSlide(*title) : std::nullopt;
    bool watermark = slide.has_value();
    if (!(fps || watermark || !caption.empty() || (preview.active && !preview.text.empty())) || !font || !(h > 0)) return;
    View view{h / 480, w / h / game::Aspect()};
    float& width = G<float>(font + kFontScale);
    float game_width = width;
    float output_width = game::Aspect() * 480;
    float watermark_x = output_width - kWatermarkRight - TextWidth(font, kWatermark) + slide.value_or(0.0f);
    CallGuest(ctx, kBegin, {});
    width = game_width * view.narrow;
    if (fps) {
        char text[32];
        std::snprintf(text, sizeof text, "FPS: %.1f", double(game::g_fps.load()));
        DrawText(ctx, font, view, text, kMargin, kMargin, true);
    }
    if (watermark) DrawText(ctx, font, view, kWatermark, watermark_x, kWatermarkTop, false);
    if (!caption.empty()) {
        const float output_width = game::Aspect() * 480.0f;
        const float max_width = output_width * 0.82f;
        const auto lines = WrapCaption(font, caption, max_width);
        const float line_height = kSize * float(game::Opt().subtitle_size) / 100.0f * 1.35f;
        const float bottom = 480.0f * 0.91f;
        const float first_y = bottom - line_height * float(lines.size());
        for (size_t i = 0; i < lines.size(); ++i)
            DrawCaptionLine(ctx, font, view, lines[i], (output_width - TextWidth(font, lines[i])) * 0.5f,
                            first_y + line_height * float(i), game::Opt().subtitle_bold,
                            game::Opt().subtitle_color, game::Opt().subtitle_shadow);
    }
    if (preview.active && !preview.text.empty()) {
        const auto lines = WrapCaption(font, preview.text, output_width * 0.82f);
        const float line_height = kSize * float(game::Opt().subtitle_size) / 100.0f * 1.35f;
        for (size_t i = 0; i < lines.size(); ++i)
            DrawCaptionLine(ctx, font, view, lines[i], preview.x,
                            preview.y + line_height * float(i), game::Opt().subtitle_bold,
                            game::Opt().subtitle_color, game::Opt().subtitle_shadow);
    }
    width = game_width;
    if (R32(kQuads)) CallGuest(ctx, kFlush, {});
    CallGuest(ctx, kEnd, {});
    W32(kTexture, 0);
    CallGuest(ctx, kApplyStates, {});
}
