#include <iostream>
#include <vector>
#include <string>

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

uintmax_t get_total_size(std::filesystem::path const& path, Unit unit, Callback_t callback) {
    using namespace std::filesystem;

    uintmax_t total = 0;
    std::queue<directory_entry> paths;
    paths.push(directory_entry(path));

    while (!paths.empty()) {
        auto path = paths.front();
        paths.pop();

        for (auto const& entry: directory_iterator(path)) {
            if (entry.is_regular_file()) {
                uintmax_t size = 0;

                switch (unit) {
                default:
                    throw std::runtime_error("Can't use the unit!");

                case Unit::Bytes:
                    size += entry.file_size();
                    break;

                case Unit::Blocks:
                    size += std::ceil(entry.file_size() / 512.0L);
                    break;
                }

                callback(entry.path().filename(), size);

                total += size;
            } else {
                paths.push(entry);
            }
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
        ("files-from", "list of paths")
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

    uintmax_t summary = 0;

    if (vm.count("input-paths")) {
        const auto paths = vm["input-paths"].as<std::vector<std::string>>();

        const auto unit = vm.count("bytes") ? Unit::Bytes : Unit::Blocks;
        const auto need_print = vm.count("all-files-data");
        const auto callback = [need_print] (auto const& path, uintmax_t size) {
            if (need_print) {
                std::cout << format(path, size);
            }
        };

        for (auto const& path : paths) {
            const auto size = get_total_size(path, unit, callback);
            summary += size;

            std::cout << format(path, size);
        }

        if (vm.count("summary")) {
            std::cout << "Summary: " << summary << '\n';
        }
    }

    return EXIT_SUCCESS;
}
