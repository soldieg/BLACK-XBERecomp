#include "common/subtitles_data.h"

#include <algorithm>
#include <charconv>

namespace subtitles {
namespace {
int64_t Timestamp(std::string_view text) {
    int h = 0, m = 0, s = 0, ms = 0;
    const size_t c1 = text.find(':'), c2 = c1 == text.npos ? text.npos : text.find(':', c1 + 1);
    const size_t comma = c2 == text.npos ? text.npos : text.find(',', c2 + 1);
    if (c1 == text.npos || c2 == text.npos || comma == text.npos || text.find_first_of(":,", comma + 1) != text.npos) return -1;
    auto number = [](std::string_view part, int& value) {
        const auto [end, error] = std::from_chars(part.data(), part.data() + part.size(), value);
        return error == std::errc{} && end == part.data() + part.size();
    };
    if (!number(text.substr(0, c1), h) || !number(text.substr(c1 + 1, c2 - c1 - 1), m) ||
        !number(text.substr(c2 + 1, comma - c2 - 1), s) || !number(text.substr(comma + 1), ms) ||
        h < 0 || m < 0 || m > 59 || s < 0 || s > 59 || ms < 0 || ms > 999) return -1;
    return ((int64_t(h) * 60 + m) * 60 + s) * 1000 + ms;
}
std::string_view StripCr(std::string_view line) {
    if (!line.empty() && line.back() == '\r') line.remove_suffix(1);
    return line;
}
bool Blank(std::string_view line) {
    return std::all_of(line.begin(), line.end(), [](unsigned char c) { return c == ' ' || c == '\t' || c == '\r'; });
}
}

std::vector<Cue> ParseSrt(std::string_view contents) {
    if (contents.starts_with("\xEF\xBB\xBF")) contents.remove_prefix(3);
    std::vector<Cue> cues;
    size_t pos = 0;
    while (pos < contents.size()) {
        size_t end = contents.find('\n', pos);
        if (end == std::string_view::npos) end = contents.size();
        auto line = StripCr(contents.substr(pos, end - pos));
        const size_t next = end == contents.size() ? end : end + 1;
        const size_t arrow = line.find(" --> ");
        if (arrow == std::string_view::npos) { pos = next; continue; }
        const size_t right = arrow + 5;
        const size_t right_end = line.find(' ', right);
        Cue cue{Timestamp(line.substr(0, arrow)), Timestamp(line.substr(right, right_end == std::string_view::npos ? line.size() - right : right_end - right)), {}};
        pos = next;
        if (cue.start_ms < 0 || cue.end_ms <= cue.start_ms) continue;
        while (pos < contents.size()) {
            end = contents.find('\n', pos);
            if (end == std::string_view::npos) end = contents.size();
            line = StripCr(contents.substr(pos, end - pos));
            pos = end == contents.size() ? end : end + 1;
            if (Blank(line)) break;
            if (!cue.text.empty()) cue.text.push_back('\n');
            cue.text.append(line);
        }
        if (!cue.text.empty()) cues.push_back(std::move(cue));
    }
    return cues;
}
const Cue* CueAt(const std::vector<Cue>& cues, int64_t elapsed_ms) {
    auto it = std::find_if(cues.begin(), cues.end(), [elapsed_ms](const Cue& c) { return elapsed_ms >= c.start_ms && elapsed_ms < c.end_ms; });
    return it == cues.end() ? nullptr : &*it;
}
std::vector<uint16_t> DecodeUtf8(std::string_view text) {
    std::vector<uint16_t> out;
    out.reserve(text.size());
    for (size_t i = 0; i < text.size();) {
        const uint8_t lead = uint8_t(text[i]);
        uint32_t cp = 0xFFFD, minimum = 0;
        size_t len = 1;
        if (lead < 0x80) cp = lead;
        else if ((lead & 0xE0) == 0xC0) { cp = lead & 0x1F; len = 2; minimum = 0x80; }
        else if ((lead & 0xF0) == 0xE0) { cp = lead & 0x0F; len = 3; minimum = 0x800; }
        else if ((lead & 0xF8) == 0xF0) { cp = lead & 7; len = 4; minimum = 0x10000; }
        bool valid = len == 1 || i + len <= text.size();
        for (size_t j = 1; valid && j < len; ++j) {
            uint8_t c = uint8_t(text[i + j]);
            if ((c & 0xC0) != 0x80) valid = false;
            else cp = (cp << 6) | (c & 0x3F);
        }
        valid = valid && (len == 1 || (cp >= minimum && cp <= 0x10FFFF && !(cp >= 0xD800 && cp <= 0xDFFF)));
        if (!valid) { out.push_back(0xFFFD); ++i; continue; }
        if (cp <= 0xFFFF) out.push_back(uint16_t(cp));
        else { cp -= 0x10000; out.push_back(uint16_t(0xD800 + (cp >> 10))); out.push_back(uint16_t(0xDC00 + (cp & 0x3FF))); }
        i += len;
    }
    return out;
}
bool SessionClock::Update(std::string_view identity, bool active, Clock::time_point now) {
    if (!active || identity.empty()) { bool changed = active_; active_ = false; identity_.clear(); started_ = {}; return changed; }
    if (!active_ || identity_ != identity) { active_ = true; identity_ = identity; started_ = now; return true; }
    return false;
}
std::optional<int64_t> SessionClock::Elapsed(Clock::time_point now) const {
    if (!active_ || now < started_) return std::nullopt;
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - started_).count();
}
}  // namespace subtitles
