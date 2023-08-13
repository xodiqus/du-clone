#include "du.hpp"

#include <sstream>
#include <cmath>
#include <forward_list>
#include <future>

namespace du {

uintmax_t get_total_size(const std::filesystem::path &path, Unit unit, Callback_t callback, ErrorHandler_t error_handler) {
    using namespace std::filesystem;

    uintmax_t total = 0;
    std::error_code errc;
    std::forward_list<std::future<uintmax_t>> async_results;

    for (auto const& entry: directory_iterator(path, errc)) {
        if (errc) {
            error_handler(errc);
            continue;
        }

        if (entry.is_directory()) {
            auto f = std::async(std::launch::async, [=] {
                const auto size = get_total_size(entry, unit, callback, error_handler);
                callback(entry, size);

                return size;
            });

            async_results.push_front(std::move(f));

        } else {

            uintmax_t size = 0;

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

            callback(entry, size);
            total += size;
        }
    }

    for (auto& f: async_results) {
        total += f.get();
    }

    if (total == 0 && unit == Unit::Blocks) {
        total = 1;
    }

    return total;
}

std::string format(const std::filesystem::path &path, uintmax_t size) {
    std::stringstream ss;
    ss << size << '\t' << path.native() << '\n';
    return ss.str();
}

}
