#pragma once

#include <filesystem>

namespace game {

// Finds the user's version-independent HDD directory, safely imports one legacy copy
// when needed, validates readability, and returns an empty path after a logged error.
std::filesystem::path PrepareSaveDataDirectory(const std::filesystem::path& exe_dir, int instance);

}  // namespace game
