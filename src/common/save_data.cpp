#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shlobj.h>

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cwctype>
#include <filesystem>
#include <string>
#include <vector>

#include "common/save_data.h"
#include "common/save_storage.h"
#include "common/native_dialogs.h"
#include "core/log.h"

namespace game {

namespace fs = std::filesystem;

namespace {

std::wstring Lower(std::wstring value) {
    std::transform(value.begin(), value.end(), value.begin(), [](wchar_t c) { return wchar_t(std::towlower(c)); });
    return value;
}

bool IsHddFolder(const fs::path& path) {
    const std::wstring name = Lower(path.filename().wstring());
    if (name == L"hdd") return true;
    if (name.size() < 4 || name.substr(0, 3) != L"hdd") return false;
    return std::all_of(name.begin() + 3, name.end(), [](wchar_t c) { return c >= L'0' && c <= L'9'; });
}

std::vector<fs::path> FindProfiles(const fs::path& hdd) {
    std::vector<fs::path> profiles;
    std::string error;
    save_storage::FindProfileFiles(hdd, profiles, error);
    if (!error.empty()) profiles.push_back(hdd / L"E" / L"UDATA" / L"45410083" / L"Profile");
    return profiles;
}

bool HasProfile(const fs::path& hdd) { return !FindProfiles(hdd).empty(); }

bool ValidateHddProfiles(const fs::path& hdd, std::string& error) {
    const std::vector<fs::path> profiles = FindProfiles(hdd);
    if (profiles.empty()) {
        error = "No Profile file was found in " + hdd.string();
        return false;
    }
    for (const fs::path& profile : profiles)
        if (!save_storage::ValidateProfile(profile, error)) return false;
    return true;
}

bool SkipSearchDirectory(const fs::path& path) {
    const std::wstring name = Lower(path.filename().wstring());
    static const std::vector<std::wstring> skipped = {L"assets", L"language", L"out", L"build", L".git", L".vs",
                                                       L".superpowers", L"node_modules", L"crashdumps", L"logs"};
    return std::find(skipped.begin(), skipped.end(), name) != skipped.end();
}

fs::path FindProjectSearchRoot(const fs::path& exe_dir) {
    fs::path current = exe_dir;
    for (int level = 0; level < 8 && !current.empty(); ++level) {
        std::error_code ec;
        if (fs::is_regular_file(current / L"CMakeLists.txt", ec) || fs::is_directory(current / L".git", ec))
            return current.parent_path();
        fs::path parent = current.parent_path();
        if (parent == current) break;
        current = std::move(parent);
    }
    return exe_dir.parent_path();
}

std::vector<fs::path> FindLegacyHdds(const fs::path& exe_dir, int instance) {
    const fs::path direct = exe_dir / (instance > 1 ? L"hdd" + std::to_wstring(instance) : L"hdd");
    std::vector<fs::path> candidates;
    if (HasProfile(direct)) candidates.push_back(direct);

    // If a release was unpacked into a new sibling folder, look in that local release
    // collection for an older HDD. Never scan unrelated drives or user folders.
    const fs::path search_root = FindProjectSearchRoot(exe_dir);
    std::error_code ec;
    if (!fs::is_directory(search_root, ec) || ec) return candidates;
    fs::recursive_directory_iterator it(search_root, fs::directory_options::skip_permission_denied, ec), end;
    size_t visited = 0;
    while (!ec && it != end && visited++ < 50000) {
        if (it.depth() >= 6 && it->is_directory(ec)) it.disable_recursion_pending();
        if (it->is_directory(ec)) {
            if (SkipSearchDirectory(it->path())) {
                it.disable_recursion_pending();
            } else if (IsHddFolder(it->path()) && HasProfile(it->path())) {
                fs::path candidate = fs::weakly_canonical(it->path(), ec);
                if (!ec && std::find(candidates.begin(), candidates.end(), candidate) == candidates.end())
                    candidates.push_back(std::move(candidate));
                it.disable_recursion_pending();
            }
        }
        it.increment(ec);
    }
    return candidates;
}

std::vector<fs::path> FindPreservedAppDataHdds(const fs::path& app_root, const fs::path& active_hdd) {
    std::vector<fs::path> candidates;
    std::error_code ec;
    fs::directory_iterator it(app_root, ec), end;
    if (ec) return candidates;

    const std::wstring prefix = active_hdd.filename().wstring();
    for (; it != end; it.increment(ec)) {
        if (ec) break;
        const std::wstring name = Lower(it->path().filename().wstring());
        const std::wstring lower_prefix = Lower(prefix);
        if (name.rfind(lower_prefix + L".preserved.", 0) != 0 &&
            name.rfind(lower_prefix + L".corrupt-backup.", 0) != 0)
            continue;
        std::error_code entry_error;
        if (it->is_directory(entry_error) && !entry_error && HasProfile(it->path()))
            candidates.push_back(it->path());
    }
    return candidates;
}

std::vector<fs::path> RecoverableLegacyHdds(const std::vector<fs::path>& candidates, std::string& rejected) {
    std::vector<fs::path> valid;
    for (const fs::path& candidate : candidates) {
        std::string error;
        if (save_storage::VerifyReadable(candidate, error) && ValidateHddProfiles(candidate, error)) {
            valid.push_back(candidate);
        } else {
            if (!rejected.empty()) rejected += "\n";
            rejected += error;
        }
    }
    return valid;
}

fs::path LocalDataRoot() {
    PWSTR local = nullptr;
    const HRESULT hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, nullptr, &local);
    if (FAILED(hr) || !local) return {};
    fs::path root(local);
    CoTaskMemFree(local);
    return root / L"BLACK-XBERecomp" / L"45410083";
}

fs::path UniqueSibling(const fs::path& target, const wchar_t* marker) {
    const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    return target.parent_path() /
           (target.filename().wstring() + marker + std::to_wstring(GetCurrentProcessId()) + L"." + std::to_wstring(now));
}

bool ImportLegacy(const fs::path& legacy, const fs::path& target, std::string& error) {
    if (!save_storage::VerifyReadable(legacy, error)) return false;
    if (!ValidateHddProfiles(legacy, error)) return false;
    const fs::path staging = UniqueSibling(target, L".importing.");
    if (!save_storage::CopyTreeNoReplace(legacy, staging, error)) {
        std::error_code ec;
        fs::remove_all(staging, ec);
        return false;
    }
    if (!save_storage::VerifyReadable(staging, error)) {
        std::error_code ec;
        fs::remove_all(staging, ec);
        return false;
    }
    std::error_code ec;
    fs::rename(staging, target, ec);
    if (ec) {
        error = "Could not activate verified save import " + staging.string() + " at " + target.string() + ": " + ec.message();
        fs::remove_all(staging, ec);
        return false;
    }
    LOGI("save: imported HDD from %ls to stable user data at %ls; original left untouched", legacy.c_str(), target.c_str());
    return true;
}

fs::path Fail(const std::wstring& message, const std::string& detail) {
    LOGE("save: startup blocked: %s", detail.c_str());
    const std::wstring title = native_dialogs::Text(native_dialogs::TextId::SaveDataErrorTitle);
    MessageBoxW(nullptr, message.c_str(), title.c_str(), MB_OK | MB_ICONERROR);
    return {};
}

enum class RecoveryChoice { RestoreVerifiedCopy, StartFresh, Cancel };

RecoveryChoice AskRecovery(const fs::path& damaged_hdd, const std::vector<fs::path>& verified_older_hdds) {
    const std::wstring message = native_dialogs::SaveRecoveryMessage(damaged_hdd, verified_older_hdds);
    UINT flags = MB_YESNOCANCEL | MB_ICONWARNING | MB_DEFBUTTON3;
    if (verified_older_hdds.size() == 1) {
        const std::wstring title = native_dialogs::Text(native_dialogs::TextId::SaveRecoveryTitle);
        const int answer = MessageBoxW(nullptr, message.c_str(), title.c_str(), flags);
        if (answer == IDYES) return RecoveryChoice::RestoreVerifiedCopy;
        if (answer == IDNO) return RecoveryChoice::StartFresh;
        return RecoveryChoice::Cancel;
    }

    const std::wstring title = native_dialogs::Text(native_dialogs::TextId::SaveRecoveryTitle);
    const int answer = MessageBoxW(nullptr, message.c_str(), title.c_str(), flags);
    return answer == IDYES ? RecoveryChoice::StartFresh : RecoveryChoice::Cancel;
}

bool StartFresh(const fs::path& target, bool preserve_target, std::string& error) {
    std::error_code ec;
    fs::path backup;
    if (preserve_target) {
        backup = UniqueSibling(target, L".corrupt-backup.");
        fs::rename(target, backup, ec);
        if (ec) {
            error = "Could not preserve unreadable HDD " + target.string() + " as " + backup.string() + ": " + ec.message();
            return false;
        }
        LOGW("save: unreadable HDD preserved at %ls before fresh profile creation", backup.c_str());
    }
    if (!fs::create_directories(target, ec) && ec) {
        error = "Could not create fresh save HDD " + target.string() + ": " + ec.message();
        if (!backup.empty()) {
            std::error_code restore_error;
            fs::rename(backup, target, restore_error);
            if (restore_error) error += "; could not restore the original HDD: " + restore_error.message();
        }
        return false;
    }
    LOGI("save: initialized an empty HDD at %ls; BLACK will create the new profile using its own initializer", target.c_str());
    return true;
}

}  // namespace

fs::path PrepareSaveDataDirectory(const fs::path& exe_dir, int instance) {
    const fs::path app_root = LocalDataRoot();
    if (app_root.empty()) return Fail(native_dialogs::Text(native_dialogs::TextId::SavePathUnavailable),
                                      "SHGetKnownFolderPath(FOLDERID_LocalAppData) failed.");
    const fs::path target = app_root / (instance > 1 ? L"hdd" + std::to_wstring(instance) : L"hdd");
    std::error_code ec;
    fs::create_directories(app_root, ec);
    if (ec) return Fail(native_dialogs::Text(native_dialogs::TextId::SaveDirectoryCreateFailed),
                        "Could not create " + app_root.string() + ": " + ec.message());

    const bool target_exists = fs::exists(target, ec);
    if (ec)
        return Fail(native_dialogs::Text(native_dialogs::TextId::SaveFolderCheckFailed),
                    "Could not inspect " + target.string() + ": " + ec.message());
    const bool has_stable_profile = target_exists && HasProfile(target);
    bool stable_hdd_invalid = false;
    std::string stable_hdd_problem;
    if (target_exists) {
        if (!save_storage::VerifyReadable(target, stable_hdd_problem)) {
            stable_hdd_invalid = true;
        } else if (has_stable_profile && !ValidateHddProfiles(target, stable_hdd_problem)) {
            stable_hdd_invalid = true;
        }
        if (!stable_hdd_invalid && has_stable_profile) {
            LOGI("save: existing profile found in stable HDD %ls; using it without replacing files", target.c_str());
            return target;
        }
    }

    std::vector<fs::path> legacy = FindLegacyHdds(exe_dir, instance);
    const std::vector<fs::path> app_data_backups = FindPreservedAppDataHdds(app_root, target);
    for (const fs::path& backup : app_data_backups)
        if (std::find(legacy.begin(), legacy.end(), backup) == legacy.end()) legacy.push_back(backup);
    std::string rejected_legacy;
    const std::vector<fs::path> verified_legacy = RecoverableLegacyHdds(legacy, rejected_legacy);
    if (stable_hdd_invalid) LOGE("save: current HDD failed the read/profile check: %s", stable_hdd_problem.c_str());
    if (!rejected_legacy.empty()) LOGE("save: older HDD candidates failed validation:\n%s", rejected_legacy.c_str());
    // With no existing save, mount a clean writable HDD and let BLACK create its
    // own first-profile data. Do not copy the sample KELLAR Profile from assets.
    // Discovery is limited to this checkout/release collection; the source remains untouched.

    bool use_legacy = verified_legacy.size() == 1;
    bool start_fresh = verified_legacy.empty() && !stable_hdd_invalid && rejected_legacy.empty();
    bool backup_stable_target = false;
    if (stable_hdd_invalid || !rejected_legacy.empty() || verified_legacy.size() > 1) {
        const fs::path problem_path = stable_hdd_invalid ? target : (legacy.empty() ? target : legacy.front());
        std::string reason = stable_hdd_invalid ? stable_hdd_problem : rejected_legacy;
        if (verified_legacy.size() > 1) reason = "Multiple older save folders passed the structural and readability checks; no copy was selected automatically.";
        if (reason.empty()) reason = "More than one older save folder was found.";
        if (!reason.empty()) LOGE("save: recovery reason: %s", reason.c_str());
        const RecoveryChoice choice = AskRecovery(problem_path, verified_legacy);
        if (choice == RecoveryChoice::Cancel)
            return Fail(native_dialogs::Text(native_dialogs::TextId::SaveRecoveryCancelled), "User cancelled save recovery.");
        if (choice == RecoveryChoice::RestoreVerifiedCopy && verified_legacy.size() == 1) {
            use_legacy = true;
            start_fresh = false;
            backup_stable_target = target_exists;
        } else {
            use_legacy = false;
            start_fresh = true;
            backup_stable_target = target_exists && stable_hdd_invalid;
        }
    }

    if (use_legacy) {
        fs::path displaced_target;
        if (target_exists) {
            displaced_target = UniqueSibling(target, L".preserved.");
            fs::rename(target, displaced_target, ec);
            if (ec)
                return Fail(native_dialogs::Text(native_dialogs::TextId::SaveRestoreFailed),
                            "Could not preserve " + target.string() + " as " + displaced_target.string() + ": " + ec.message());
            LOGW("save: existing HDD was preserved at %ls before importing a verified older copy", displaced_target.c_str());
        }
        std::string error;
        if (!ImportLegacy(verified_legacy.front(), target, error)) {
            if (!displaced_target.empty()) {
                std::error_code restore_error;
                fs::rename(displaced_target, target, restore_error);
                if (restore_error) error += "; also could not restore preserved HDD " + displaced_target.string() + ": " + restore_error.message();
            }
            return Fail(native_dialogs::Text(native_dialogs::TextId::SaveRestoreFailed),
                        error);
        }
    } else if (start_fresh) {
        std::string error;
        if (!StartFresh(target, backup_stable_target, error))
            return Fail(native_dialogs::Text(native_dialogs::TextId::SaveInitializationFailed), error);
    }

    std::string error;
    if (!save_storage::VerifyReadable(target, error))
        return Fail(native_dialogs::Text(native_dialogs::TextId::SaveReadFailed), error);
    return target;
}

}  // namespace game
