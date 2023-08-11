#ifndef DU_HPP
#define DU_HPP

#include <filesystem>
#include <functional>

namespace du {

enum class Unit {
    Bytes, Blocks
};

using Callback_t = std::function<void(std::filesystem::path const&, uintmax_t)>;
using ErrorHandler_t = std::function<void(std::error_code)>;

uintmax_t get_total_size(std::filesystem::path const& path, Unit unit, Callback_t callback, ErrorHandler_t error_handler);

std::string format(std::filesystem::path const& path, uintmax_t size);

}

#endif // DU_HPP
