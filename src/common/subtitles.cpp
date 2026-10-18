#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <mutex>
#include <string>
#include <vector>

#include "app/app.h"
#include "common/options.h"
#include "common/subtitles.h"
#include "common/subtitles_data.h"
#include "core/log.h"
#include "game/layout.h"
#include "xdk/hle.h"

using namespace kernel;

namespace {
std::mutex g_lock;
std::string g_name;
std::vector<subtitles::Cue> g_cues;
subtitles::SessionClock g_clock;
std::chrono::steady_clock::time_point g_last_load;
app::SubtitlePreview g_preview;

std::vector<subtitles::Cue> ReadSrt(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return {};
    const std::string contents{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    return subtitles::ParseSrt(contents);
}
std::string Locale() {
    const auto& language = game::BootLanguage();
    if (language == "esp") return "es";
    if (language == "fra") return "fr";
    if (language == "deu") return "de";
    if (language == "ita") return "it";
    return language == "int" ? "en-US" : language;
}
}

void app::SetSubtitlePreview(std::string text, float x, float y, bool active) {
    std::lock_guard guard(g_lock);
    g_preview = {std::move(text), x, y, active};
}
app::SubtitlePreview app::CurrentSubtitlePreview() {
    std::lock_guard guard(g_lock);
    return g_preview;
}
void app::UpdateSubtitleSession(uint32_t film, uint32_t movie, std::string_view stem, bool active) {
    const std::string identity = active && !stem.empty()
        ? Locale() + "/" + std::to_string(film) + "/" + std::string(stem) + "/" + std::to_string(movie)
        : std::string{};
    std::lock_guard guard(g_lock);
    if (g_clock.Update(identity, active, std::chrono::steady_clock::now())) {
        g_name.clear();
        g_cues.clear();
        g_last_load = {};
    }
}

std::string app::CurrentSubtitle() {
    if (!game::Opt().subtitles_enabled) return {};
    const uint32_t game_object = R32(layout::kGame);
    if (!game_object) return {};
    const uint32_t film = R32(game_object + layout::kCurrentFilm);
    const uint32_t player = game_object + layout::kFrontEnd + layout::kFilmPlayer;
    constexpr uint32_t kFilmName = 0x84, kFilmState = 0x1A8, kFilmPlaying = 0x1C;
    const uint32_t state = R32(player + kFilmState);
    if (film >= layout::kFilms || state < 4 || state > kFilmPlaying) {
        std::lock_guard guard(g_lock);
        g_name.clear(); g_cues.clear();
        g_clock.Update({}, false, std::chrono::steady_clock::now());
        return {};
    }
    const std::string name = ReadCString(player + kFilmName, 8);
    if (name.size() < 4) return {};
    std::string stem = name.substr(0, 4);
    if (stem.back() == '.') stem.pop_back();
    const uint32_t movie = R32(player);
    const std::string locale = Locale();
    const std::string identity = locale + "/" + std::to_string(film) + "/" + stem + "/" + std::to_string(movie);
    std::lock_guard guard(g_lock);
    const auto now = std::chrono::steady_clock::now();
    if (identity != g_name) {
        g_name = identity; g_cues.clear(); g_last_load = {};
        g_clock.Update(identity, true, now);
    } else {
        g_clock.Update(identity, true, now);
    }
    if (g_cues.empty() && (g_last_load.time_since_epoch().count() == 0 || now - g_last_load >= std::chrono::seconds(1))) {
        g_last_load = now;
        auto path = app::ExeDir() / "language" / locale / "Subtitles" / (stem + ".srt");
        g_cues = ReadSrt(path);
        if (g_cues.empty() && locale != "en-US") {
            path = app::ExeDir() / "language" / "en-US" / "Subtitles" / (stem + ".srt");
            g_cues = ReadSrt(path);
        }
        if (!g_cues.empty()) LOGR("subtitles: loaded %zu cues for %s (%s)", g_cues.size(), stem.c_str(), locale.c_str());
    }
    const auto elapsed = g_clock.Elapsed(now);
    const auto* cue = elapsed ? subtitles::CueAt(g_cues, *elapsed) : nullptr;
    return cue ? cue->text : std::string{};
}
