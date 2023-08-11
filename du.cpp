#include "du.hpp"

#include <sstream>
#include <cmath>

namespace du {

uintmax_t get_total_size(const std::filesystem::path &path, Unit unit, Callback_t callback, ErrorHandler_t error_handler) {
    using namespace std::filesystem;

    uintmax_t total = 0;
    std::error_code errc;

    for (auto const& entry: directory_iterator(path, errc)) {
        if (errc) {
            error_handler(errc);
            continue;
        }

        uintmax_t size = 0;

        if (entry.is_directory()) {
            size = get_total_size(entry, unit, callback, error_handler);
        } else {

            switch (unit) {
            default:
                throw std::runtime_error("Can't use the unit!");

            case Unit::Bytes:
                size = entry.file_size(errc);
                break;

            case Unit::Blocks:
                size = std::ceil(entry.file_size(errc) / 512.0L);
                break;
            }

            if (errc) {
                error_handler(errc);
                continue;
            }

        }

        callback(entry, size);
        total += size;
    }

    if (total == 0 && unit == Unit::Blocks) {
        total = 1;
    }

    return total;
}

std::string format(const std::filesystem::path &path, uintmax_t size) {
    std::stringstream ss;
    ss << size << '\t' << path << '\n';
    return ss.str();
}

}
