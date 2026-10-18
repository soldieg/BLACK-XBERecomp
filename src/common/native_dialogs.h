#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace native_dialogs {

enum class TextId {
    GameFilesMissing,
    BootFailed,
    SaveDataErrorTitle,
    SaveRecoveryTitle,
    SavePathUnavailable,
    SaveDirectoryCreateFailed,
    SaveFolderCheckFailed,
    SaveRecoveryCancelled,
    SaveRestoreFailed,
    SaveInitializationFailed,
    SaveReadFailed,
};

// Message text follows the Windows display UI language; Windows supplies localized button labels.
std::wstring Text(TextId id);
std::wstring GameReleaseWarning(const std::wstring& release_name);
std::wstring SaveRecoveryMessage(const std::filesystem::path& damaged_hdd,
                                const std::vector<std::filesystem::path>& verified_older_hdds);

}  // namespace native_dialogs
