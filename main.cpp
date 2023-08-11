#include <iostream>
#include <vector>
#include <deque>
#include <string>
#include <fstream>

#include <boost/program_options.hpp>

#include <filesystem>
#include <queue>
#include <functional>
#include <string_view>
#include <sstream>

enum class Unit {
    Bytes, Blocks
};

using Callback_t = std::function<void(std::filesystem::path const&, uintmax_t)>;
using ErrorHandler_t = std::function<void(std::error_code const&)>;

uintmax_t get_total_size(std::filesystem::path const& path, Unit unit, Callback_t callback, ErrorHandler_t error_handler) {
    using namespace std::filesystem;

    uintmax_t total = 0;
    std::error_code errc;

    for (auto const& entry: directory_iterator(path, errc)) {
        if (errc) {
            error_handler(errc);
            continue;
        }

        if (entry.is_directory()) {
            auto size = get_total_size(entry, unit, callback, error_handler);

            if (size == 0 && unit == Unit::Blocks) {
                size = 1;
            }

            callback(entry, size);
        } else {
            uintmax_t size = 0;

            switch (unit) {
            default:
                throw std::runtime_error("Can't use the unit!");

            case Unit::Bytes:
                size += entry.file_size(errc);
                break;

            case Unit::Blocks:
                size += std::ceil(entry.file_size(errc) / 512.0L);
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

    return total;
}

std::string format(std::filesystem::path const& path, uintmax_t size) {
    std::stringstream ss;
    ss << size << '\t' << path << '\n';
    return ss.str();
}

int main(int argc, char** argv)
{
    using namespace boost::program_options;

    options_description desc("Options");
    desc.add_options()
        ("help,h", "produce help message")
        ("bytes,b", "exact size in bytes")
        ("all-files-data,a", "data of all files")
        ("summary,c", "print summary of total size at the ent")
        ("summary-only,s", "print only summary of total size")
        ("from-file", value<std::string>(), "read list of paths")
        ("input-paths", value<std::vector<std::string>>(), "")
    ;

    positional_options_description p;
    p.add("input-paths", -1);

    const auto parsed_options = command_line_parser(argc, argv)
            .options(desc)
            .positional(p)
            .run();

    variables_map vm;
    store(parsed_options, vm);
    notify(vm);

    if (vm.count("help")) {
        std::cout << desc << '\n';
        return EXIT_SUCCESS;
    }

    const auto unit = vm.count("bytes") ? Unit::Bytes : Unit::Blocks;
    const auto need_print_all_files = vm.count("all-files-data");
    const auto need_print_only_summary = vm.count("summary-only");

    uintmax_t summary = 0;
    std::vector<std::string> paths;

    if (vm.count("input-paths")) {
        paths = vm["input-paths"].as<decltype(paths)>();
    } else if (vm.count("from-file")) {
        std::ifstream s {vm["from-file"].as<std::string>()};
        std::string line = "";

        while (std::getline(s, line)) {
            paths.push_back(line);
        }
    }

    const auto callback = [=] (auto const& path, uintmax_t size) {
        if (need_print_all_files && !need_print_only_summary) {
            std::cout << format(path, size);
        }
    };

    constexpr auto error_handler = [](auto const& error_code) {
        std::cerr << error_code.message() << '\n';
    };

    for (auto const& path : paths) {
        const auto size = get_total_size(path, unit, callback, error_handler);
        summary += size;

        if (!need_print_only_summary) {
            std::cout << format(path, size);
        }
    }

    if (vm.count("summary") || need_print_only_summary) {
        std::cout << "\nSummary: " << summary << '\n';
    }

    return EXIT_SUCCESS;
}
