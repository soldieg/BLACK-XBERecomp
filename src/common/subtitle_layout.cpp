#include "common/subtitle_layout.h"

namespace subtitles {
Point2D TransformPoint(Point2D point, std::initializer_list<Matrix2D> transforms) {
    for (const Matrix2D& m : transforms) {
        const float x = point.x, y = point.y;
        point.x = m.a * x + m.c * y + m.x;
        point.y = m.b * x + m.d * y + m.y;
    }
    return point;
}

uint16_t DisplayGlyph(uint16_t codepoint, bool portuguese_font) {
    if (!portuguese_font) return codepoint;
    if (codepoint == 0x00E3) return 0x00E4;  // ã -> ä
    if (codepoint == 0x00C3) return 0x00C4;  // Ã -> Ä
    if (codepoint == 0x00F5) return 0x00F6;  // õ -> ö
    if (codepoint == 0x00D5) return 0x00D6;  // Õ -> Ö
    return codepoint;
}

float CenteredCaptionX(float output_width, float unscaled_text_width) {
    return (output_width - unscaled_text_width) * 0.5f;
}

std::string_view PreviewSample() {
    return "Prévia da legenda";
}
}  // namespace subtitles
