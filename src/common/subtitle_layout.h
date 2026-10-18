#pragma once

#include <initializer_list>
#include <cstdint>
#include <string_view>

namespace subtitles {
struct Point2D { float x = 0.0f; float y = 0.0f; };
struct Matrix2D { float a = 1.0f; float b = 0.0f; float c = 0.0f; float d = 1.0f; float x = 0.0f; float y = 0.0f; };

// Applies clip transforms in order from the text field's local clip to the screen stage.
Point2D TransformPoint(Point2D point, std::initializer_list<Matrix2D> local_to_stage);

// Portuguese subtitle fonts may omit tilde-A while containing the umlaut glyphs.
// Keep source text intact and select an available display glyph only at render time.
uint16_t DisplayGlyph(uint16_t codepoint, bool portuguese_font);

// Coordinates and widths in the subtitle renderer's logical 480-line frame.
float CenteredCaptionX(float output_width, float unscaled_text_width);
std::string_view PreviewSample();
}  // namespace subtitles
