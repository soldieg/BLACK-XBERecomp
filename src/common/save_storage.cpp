#include "common/save_storage.h"

#include <array>
#include <fstream>

namespace game::save_storage {

namespace {

uint32_t ReadU32(const std::array<uint8_t, 0x1800>& bytes, size_t offset) {
    return uint32_t(bytes[offset]) | (uint32_t(bytes[offset + 1]) << 8) | (uint32_t(bytes[offset + 2]) << 16) |
           (uint32_t(bytes[offset + 3]) << 24);
}

bool FilesEqual(const std::filesystem::path& left, const std::filesystem::path& right, std::string& error) {
    std::ifstream a(left, std::ios::binary), b(right, std::ios::binary);
    if (!a || !b) {
        error = "Could not open both files for save migration comparison: " + left.string() + " and " + right.string();
        return false;
    }
    std::array<char, 64 * 1024> x{}, y{};
    do {
        a.read(x.data(), std::streamsize(x.size()));
        b.read(y.data(), std::streamsize(y.size()));
        if (a.gcount() != b.gcount() || !std::equal(x.begin(), x.begin() + a.gcount(), y.begin())) return false;
        if (a.bad() || b.bad()) {
            error = "Could not read files while comparing save data: " + left.string() + " and " + right.string();
            return false;
        }
    } while (a.gcount() != 0);
    return true;
}

}  // namespace

bool CopyTreeNoReplace(const std::filesystem::path& source, const std::filesystem::path& destination,
                       std::string& error) {
    namespace fs = std::filesystem;
    std::error_code ec;
    if (!fs::is_directory(source, ec) || ec) {
        error = "Legacy HDD folder is missing or unreadable: " + source.string();
        return false;
    }
    if (fs::exists(destination, ec) || ec) {
        error = "Save migration destination already exists; refusing to replace it: " + destination.string();
        return false;
    }
    if (!fs::create_directories(destination, ec) || ec) {
        error = "Could not create save migration staging folder " + destination.string() + ": " + ec.message();
        return false;
    }

    fs::recursive_directory_iterator it(source, fs::directory_options::none, ec), end;
    if (ec) {
        error = "Could not enumerate legacy HDD folder " + source.string() + ": " + ec.message();
        return false;
    }
    for (; it != end; it.increment(ec)) {
        if (ec) {
            error = "Could not enumerate legacy HDD folder " + source.string() + ": " + ec.message();
            return false;
        }
        const fs::path relative = fs::relative(it->path(), source, ec);
        if (ec) {
            error = "Could not resolve a legacy save path under " + source.string() + ": " + ec.message();
            return false;
        }
        const fs::path target = destination / relative;
        const fs::file_status status = it->symlink_status(ec);
        if (ec || fs::is_symlink(status)) {
            error = "Legacy HDD contains an unreadable or symbolic-link entry: " + it->path().string();
            return false;
        }
        if (fs::is_directory(status)) {
            fs::create_directories(target, ec);
            if (ec) {
                error = "Could not create save folder " + target.string() + ": " + ec.message();
                return false;
            }
            continue;
        }
        if (!fs::is_regular_file(status)) continue;
        fs::create_directories(target.parent_path(), ec);
        if (ec) {
            error = "Could not create save folder " + target.parent_path().string() + ": " + ec.message();
            return false;
        }
        if (!fs::copy_file(it->path(), target, fs::copy_options::none, ec) || ec) {
            error = "Could not copy legacy save file " + it->path().string() + " to " + target.string() + ": " + ec.message();
            return false;
        }
        std::string compare_error;
        if (!FilesEqual(it->path(), target, compare_error)) {
            error = compare_error.empty() ? "Copied save file failed byte-for-byte verification: " + target.string()
                                          : std::move(compare_error);
            return false;
        }
    }
    return true;
}

bool VerifyReadable(const std::filesystem::path& root, std::string& error) {
    namespace fs = std::filesystem;
    std::error_code ec;
    if (!fs::is_directory(root, ec) || ec) {
        error = "Save HDD folder is missing or unreadable: " + root.string();
        return false;
    }
    fs::recursive_directory_iterator it(root, fs::directory_options::none, ec), end;
    if (ec) {
        error = "Could not enumerate save HDD folder " + root.string() + ": " + ec.message();
        return false;
    }
    std::array<char, 64 * 1024> buffer{};
    for (; it != end; it.increment(ec)) {
        if (ec) {
            error = "Could not enumerate save HDD folder " + root.string() + ": " + ec.message();
            return false;
        }
        const fs::file_status status = it->symlink_status(ec);
        if (ec || fs::is_symlink(status)) {
            error = "Save HDD contains an unreadable or symbolic-link entry: " + it->path().string();
            return false;
        }
        if (!fs::is_regular_file(status)) continue;
        std::ifstream file(it->path(), std::ios::binary);
        if (!file) {
            error = "Could not open existing save HDD file for reading: " + it->path().string();
            return false;
        }
        while (file) file.read(buffer.data(), std::streamsize(buffer.size()));
        if (!file.eof()) {
            error = "Could not finish reading existing save HDD file: " + it->path().string();
            return false;
        }
    }
    return true;
}

bool ValidateProfile(const std::filesystem::path& profile, std::string& error) {
    namespace fs = std::filesystem;
    std::error_code ec;
    const uintmax_t size = fs::file_size(profile, ec);
    if (ec || size != 0x1800) {
        error = "Profile has an unreadable or unexpected size at " + profile.string() +
                (ec ? ": " + ec.message() : ": expected 6144 bytes, got " + std::to_string(size));
        return false;
    }
    std::array<uint8_t, 0x1800> bytes{};
    std::ifstream file(profile, std::ios::binary);
    if (!file || !file.read(reinterpret_cast<char*>(bytes.data()), std::streamsize(bytes.size()))) {
        error = "Could not read all 6144 bytes of Profile at " + profile.string();
        return false;
    }

    constexpr size_t slot_size = 0x778;
    constexpr size_t payload_size = 0x770;
    for (size_t slot = 0; slot < 3; ++slot) {
        const size_t base = slot * slot_size;
        const uint32_t marker = ReadU32(bytes, base);
        if ((marker != 0x26 && marker != 0) || ReadU32(bytes, base + 4) != slot_size) {
            error = "Profile slot " + std::to_string(slot + 1) + " has an invalid marker or record size: " + profile.string();
            return false;
        }
        if (marker == 0x26) {
            uint32_t sum = 0;
            for (size_t offset = 0; offset < payload_size; offset += 4) sum += ReadU32(bytes, base + offset);
            if (sum != ReadU32(bytes, base + payload_size)) {
                error = "Profile slot " + std::to_string(slot + 1) + " has an invalid checksum: " + profile.string();
                return false;
            }
        }
    }
    return true;
}

}  // namespace game::save_storage
