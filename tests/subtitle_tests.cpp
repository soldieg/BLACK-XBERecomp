#include "common/subtitle_layout.h"
#include "common/subtitles_data.h"

#include <chrono>
#include <iostream>
#include <string>
#include <vector>

namespace {
int failures = 0;

void Check(bool ok, const char* name) {
    if (!ok) {
        std::cerr << "FAIL: " << name << '\n';
        ++failures;
    }
}

void ParsesBomCrLfAndMultilineCues() {
    const std::string srt = "\xEF\xBB\xBF" "1\r\n00:00:01,250 --> 00:00:03,500\r\nKellar: linha um\r\nlinha dois\r\n\r\n";
    const auto cues = subtitles::ParseSrt(srt);
    Check(cues.size() == 1, "one valid cue parsed from BOM and CRLF SRT");
    if (!cues.empty()) {
        Check(cues[0].start_ms == 1250, "cue start timestamp parsed");
        Check(cues[0].end_ms == 3500, "cue end timestamp parsed");
        Check(cues[0].text == "Kellar: linha um\nlinha dois", "multiline cue text preserved");
    }
}

void DecodesUtf8ForTheGameFont() {
    const auto text = subtitles::DecodeUtf8("Ação — Привет");
    const std::vector<uint16_t> expected = {'A', 0x00E7, 0x00E3, 'o', ' ', 0x2014, ' ',
                                             0x041F, 0x0440, 0x0438, 0x0432, 0x0435, 0x0442};
    Check(text == expected, "UTF-8 caption becomes native font code points");
}

void PortugueseMissingTildeGlyphUsesUmlautFallback() {
    Check(subtitles::DisplayGlyph(0x00E3, true) == 0x00E4, "Portuguese ã uses the available ä glyph");
    Check(subtitles::DisplayGlyph(0x00C3, true) == 0x00C4, "Portuguese Ã uses the available Ä glyph");
    Check(subtitles::DisplayGlyph(0x00F5, true) == 0x00F6, "Portuguese õ uses the available ö glyph");
    Check(subtitles::DisplayGlyph(0x00D5, true) == 0x00D6, "Portuguese Õ uses the available Ö glyph");
    Check(subtitles::DisplayGlyph(0x00E3, false) == 0x00E3, "other locales keep the original glyph");
}

void RejectsInvalidCueRangesAndTimestamps() {
    const std::string srt =
        "1\n00:00:02,000 --> 00:00:01,000\nreversed\n\n"
        "2\n00:90:00,000 --> 00:90:01,000\ninvalid minutes\n\n"
        "3\n00:00:01,000 --> 00:00:02,000\nvalid\n\n";
    const auto cues = subtitles::ParseSrt(srt);
    Check(cues.size() == 1, "invalid timestamps and reversed cue ranges are discarded");
}

void SelectsCueWithHalfOpenTimeRange() {
    const auto cues = subtitles::ParseSrt("1\n00:00:01,000 --> 00:00:02,000\nhello\n\n");
    Check(subtitles::CueAt(cues, 999) == nullptr, "cue is inactive before its start");
    Check(subtitles::CueAt(cues, 1000) != nullptr, "cue starts inclusively");
    Check(subtitles::CueAt(cues, 1999) != nullptr, "cue remains active before its end");
    Check(subtitles::CueAt(cues, 2000) == nullptr, "cue expires at its end");
}

void MovieSessionClockDoesNotRestartWhilePolling() {
    using Clock = std::chrono::steady_clock;
    subtitles::SessionClock clock;
    const auto t0 = Clock::time_point{};
    Check(clock.Update("pt-BR/3/03_N", true, t0), "first active session starts");
    Check(clock.Elapsed(t0 + std::chrono::milliseconds(400)).value_or(-1) == 400,
          "elapsed time follows active movie session");
    Check(!clock.Update("pt-BR/3/03_N", true, t0 + std::chrono::milliseconds(500)),
          "polling same session does not reset its clock");
    Check(clock.Elapsed(t0 + std::chrono::milliseconds(700)).value_or(-1) == 700,
          "same session continues elapsed time");
    Check(clock.Update("pt-BR/4/05_N", true, t0 + std::chrono::milliseconds(800)),
          "new movie session resets the clock");
    Check(clock.Elapsed(t0 + std::chrono::milliseconds(950)).value_or(-1) == 150,
          "new session has independent elapsed time");
    Check(clock.Update({}, false, t0 + std::chrono::seconds(1)), "inactive movie retires session");
    Check(!clock.Elapsed(t0 + std::chrono::seconds(2)).has_value(), "retired session has no elapsed time");
}

void PreviewAnchorIncludesMenuAndStageTransforms() {
    const subtitles::Point2D under_r{18.0f, 30.0f};
    const subtitles::Matrix2D row{1.0f, 0.0f, 0.0f, 1.0f, 70.0f, 120.0f};
    const subtitles::Matrix2D main{1.25f, 0.0f, 0.0f, 1.0f, 240.0f, 12.0f};
    const subtitles::Matrix2D stage{1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f};
    const subtitles::Point2D screen = subtitles::TransformPoint(under_r, {row, main, stage});
    Check(screen.x == 350.0f && screen.y == 162.0f,
          "preview point under R is transformed from its menu row through Main to the stage");
}

void SubtitlePresentationHelpersUseOneCompactPreviewAndLogicalCentering() {
    Check(subtitles::PreviewSample() == "Prévia da legenda", "preview contains only its short caption sample");
    Check(subtitles::CenteredCaptionX(960.0f, 400.0f) == 280.0f,
          "caption centers using logical width before widescreen glyph scaling");
}
}  // namespace

int main() {
    ParsesBomCrLfAndMultilineCues();
    DecodesUtf8ForTheGameFont();
    PortugueseMissingTildeGlyphUsesUmlautFallback();
    RejectsInvalidCueRangesAndTimestamps();
    SelectsCueWithHalfOpenTimeRange();
    MovieSessionClockDoesNotRestartWhilePolling();
    PreviewAnchorIncludesMenuAndStageTransforms();
    SubtitlePresentationHelpersUseOneCompactPreviewAndLogicalCentering();
    return failures ? 1 : 0;
}
