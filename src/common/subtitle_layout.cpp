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
}  // namespace subtitles
