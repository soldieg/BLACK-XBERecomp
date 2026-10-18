#pragma once
#include <string>
#include <string_view>
#include <cstdint>

namespace app {
// Current cutscene subtitle, empty when the active movie has no matching SRT.
std::string CurrentSubtitle();
void UpdateSubtitleSession(uint32_t film, uint32_t movie, std::string_view stem, bool active);
struct SubtitlePreview { std::string text; float x = 0; float y = 0; bool active = false; };
void SetSubtitlePreview(std::string text, float x, float y, bool active);
SubtitlePreview CurrentSubtitlePreview();
}
