#pragma once

#include <initializer_list>

namespace subtitles {
struct Point2D { float x = 0.0f; float y = 0.0f; };
struct Matrix2D { float a = 1.0f; float b = 0.0f; float c = 0.0f; float d = 1.0f; float x = 0.0f; float y = 0.0f; };

// Applies clip transforms in order from the text field's local clip to the screen stage.
Point2D TransformPoint(Point2D point, std::initializer_list<Matrix2D> local_to_stage);
}  // namespace subtitles
