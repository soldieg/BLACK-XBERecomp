#pragma once

#include <filesystem>
#include <string>

namespace game::save_storage {

// Copy a complete legacy HDD tree to a new directory without replacing any file.
// The source is never removed. A failure leaves the caller responsible for discarding
// its private staging directory, not the source or an existing destination.
bool CopyTreeNoReplace(const std::filesystem::path& source, const std::filesystem::path& destination,
                       std::string& error);

// Read every regular file below root to confirm the mounted save tree is accessible.
bool VerifyReadable(const std::filesystem::path& root, std::string& error);

// Validate the proven Profile layout and per-slot additive checksums without
// changing bytes or claiming to authenticate the game's final signature.
bool ValidateProfile(const std::filesystem::path& profile, std::string& error);

}  // namespace game::save_storage
