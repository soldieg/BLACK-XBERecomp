#include "common/save_storage.h"

#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

class TempDir {
public:
    TempDir() {
        path = fs::temp_directory_path() /
               ("black-save-storage-test-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(path);
    }
    ~TempDir() {
        std::error_code ec;
        fs::remove_all(path, ec);
    }
    fs::path path;
};

bool Write(const fs::path& path, const std::string& value) {
    fs::create_directories(path.parent_path());
    std::ofstream file(path, std::ios::binary);
    file.write(value.data(), std::streamsize(value.size()));
    return bool(file);
}

std::string Read(const fs::path& path) {
    std::ifstream file(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

void WriteU32(std::vector<uint8_t>& bytes, size_t offset, uint32_t value) {
    for (unsigned i = 0; i < 4; ++i) bytes[offset + i] = uint8_t(value >> (i * 8));
}

std::vector<uint8_t> ValidProfileBytes() {
    constexpr size_t slot_size = 0x778;
    constexpr size_t slot_payload = 0x770;
    std::vector<uint8_t> bytes(0x1800, 0);
    for (size_t slot = 0; slot < 3; ++slot) {
        const size_t base = slot * slot_size;
        WriteU32(bytes, base, 0x26);
        WriteU32(bytes, base + 4, uint32_t(slot_size));
        bytes[base + 0x76D] = slot == 0 ? 0 : 1;
        uint32_t sum = 0;
        for (size_t offset = 0; offset < slot_payload; offset += 4) {
            const size_t at = base + offset;
            sum += uint32_t(bytes[at]) | (uint32_t(bytes[at + 1]) << 8) | (uint32_t(bytes[at + 2]) << 16) |
                   (uint32_t(bytes[at + 3]) << 24);
        }
        WriteU32(bytes, base + slot_payload, sum);
    }
    return bytes;
}

bool WriteBytes(const fs::path& path, const std::vector<uint8_t>& bytes) {
    fs::create_directories(path.parent_path());
    std::ofstream file(path, std::ios::binary);
    file.write(reinterpret_cast<const char*>(bytes.data()), std::streamsize(bytes.size()));
    return bool(file);
}

bool TestCopiesLegacyFilesAndKeepsSource() {
    TempDir temp;
    const fs::path source = temp.path / "old" / "hdd";
    const fs::path destination = temp.path / "staging";
    const fs::path profile = source / "E" / "UDATA" / "45410083" / "1AAF1A0557B7" / "Profile";
    const std::string payload = "existing-profile-bytes";
    if (!Write(profile, payload)) return false;
    std::string error;
    if (!game::save_storage::CopyTreeNoReplace(source, destination, error)) {
        std::cerr << error << '\n';
        return false;
    }
    return fs::exists(profile) && Read(profile) == payload && Read(destination / fs::relative(profile, source)) == payload;
}

bool TestRefusesToReplaceExistingDestination() {
    TempDir temp;
    const fs::path source = temp.path / "old" / "hdd";
    const fs::path destination = temp.path / "stable";
    const fs::path old_profile = source / "E" / "UDATA" / "45410083" / "Profile";
    const fs::path existing_profile = destination / "E" / "UDATA" / "45410083" / "Profile";
    if (!Write(old_profile, "old-save") || !Write(existing_profile, "current-save")) return false;
    std::string error;
    if (game::save_storage::CopyTreeNoReplace(source, destination, error)) return false;
    return Read(old_profile) == "old-save" && Read(existing_profile) == "current-save";
}

bool TestVerifiesReadableSaveFiles() {
    TempDir temp;
    const fs::path root = temp.path / "hdd";
    if (!Write(root / "E" / "UDATA" / "45410083" / "Profile", "profile-data")) return false;
    std::string error;
    if (!game::save_storage::VerifyReadable(root, error)) {
        std::cerr << error << '\n';
        return false;
    }
    return true;
}

bool TestRejectsMissingSaveRoot() {
    TempDir temp;
    std::string error;
    return !game::save_storage::VerifyReadable(temp.path / "missing", error) && !error.empty();
}

bool TestAcceptsStructurallyValidProfile() {
    TempDir temp;
    const fs::path profile = temp.path / "Profile";
    std::string error;
    return WriteBytes(profile, ValidProfileBytes()) && game::save_storage::ValidateProfile(profile, error);
}

bool TestRejectsProfileWithBadSlotChecksum() {
    TempDir temp;
    const fs::path profile = temp.path / "Profile";
    std::vector<uint8_t> bytes = ValidProfileBytes();
    bytes[0x200] ^= 1;
    std::string error;
    return WriteBytes(profile, bytes) && !game::save_storage::ValidateProfile(profile, error) && error.find("checksum") != std::string::npos;
}

bool TestRejectsTruncatedProfile() {
    TempDir temp;
    const fs::path profile = temp.path / "Profile";
    std::vector<uint8_t> bytes = ValidProfileBytes();
    bytes.resize(0x1000);
    std::string error;
    return WriteBytes(profile, bytes) && !game::save_storage::ValidateProfile(profile, error) && !error.empty();
}

bool TestIgnoresUserFoldersWithoutProfiles() {
    TempDir temp;
    const fs::path hdd = temp.path / "hdd";
    fs::create_directories(hdd / "E" / "UDATA" / "45410083" / "12D617B01AB2");
    std::vector<fs::path> profiles;
    std::string error;
    if (!game::save_storage::FindProfileFiles(hdd, profiles, error)) {
        std::cerr << error << '\n';
        return false;
    }
    return profiles.empty();
}

bool TestFindsExistingProfileAlongsideEmptyUserFolders() {
    TempDir temp;
    const fs::path hdd = temp.path / "hdd";
    const fs::path profile = hdd / "E" / "UDATA" / "45410083" / "1AAF1A0557B7" / "Profile";
    fs::create_directories(hdd / "E" / "UDATA" / "45410083" / "12D617B01AB2");
    if (!WriteBytes(profile, ValidProfileBytes())) return false;
    std::vector<fs::path> profiles;
    std::string error;
    if (!game::save_storage::FindProfileFiles(hdd, profiles, error)) {
        std::cerr << error << '\n';
        return false;
    }
    return profiles.size() == 1 && profiles.front() == profile;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc == 2) {
        std::string error;
        if (!game::save_storage::ValidateProfile(fs::path(argv[1]), error)) {
            std::cerr << "FAIL: profile validation: " << error << '\n';
            return 1;
        }
        std::cout << "PASS: profile structure/checksums " << argv[1] << '\n';
        return 0;
    }
    const std::array tests = {
        std::pair{"copies legacy save without removing source", TestCopiesLegacyFilesAndKeepsSource},
        std::pair{"refuses to replace an existing save", TestRefusesToReplaceExistingDestination},
        std::pair{"reads existing save files", TestVerifiesReadableSaveFiles},
        std::pair{"reports a missing save root", TestRejectsMissingSaveRoot},
        std::pair{"accepts structurally valid profiles", TestAcceptsStructurallyValidProfile},
        std::pair{"rejects a corrupt profile checksum", TestRejectsProfileWithBadSlotChecksum},
        std::pair{"rejects a truncated profile", TestRejectsTruncatedProfile},
        std::pair{"ignores normal user folders without Profile files", TestIgnoresUserFoldersWithoutProfiles},
        std::pair{"finds a real Profile while ignoring empty user folders", TestFindsExistingProfileAlongsideEmptyUserFolders},
    };
    for (const auto& [name, run] : tests) {
        if (!run()) {
            std::cerr << "FAIL: " << name << '\n';
            return 1;
        }
        std::cout << "PASS: " << name << '\n';
    }
    return 0;
}
