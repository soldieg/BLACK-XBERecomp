#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace subtitles {
struct Cue {
    int64_t start_ms;
    int64_t end_ms;
    std::string text;
};

std::vector<Cue> ParseSrt(std::string_view contents);
const Cue* CueAt(const std::vector<Cue>& cues, int64_t elapsed_ms);
std::vector<uint16_t> DecodeUtf8(std::string_view text);

class SessionClock {
public:
    using Clock = std::chrono::steady_clock;
    bool Update(std::string_view identity, bool active, Clock::time_point now);
    std::optional<int64_t> Elapsed(Clock::time_point now) const;

private:
    std::string identity_;
    Clock::time_point started_{};
    bool active_ = false;
};
}  // namespace subtitles
