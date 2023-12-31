#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <atomic>

#include <boost/program_options.hpp>
#include <boost/thread/thread.hpp>

#include "du.hpp"

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
        ("input-paths", value<std::vector<std::string>>(), "input ones, a size of which gets to calc")
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

    const auto need_print_all_files = vm.count("all-files-data");
    const auto need_print_only_summary = vm.count("summary-only");

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

    using du::Unit;
    using du::get_total_size;
    using du::format;

    const auto callback = [=] (auto const& path, uintmax_t size) {
        if (need_print_all_files && !need_print_only_summary) {
            std::cout << format(path, size);
        }
    };

    constexpr auto error_handler = [](auto error_code) {
        std::cerr << error_code.message() << '\n';
    };

    const auto unit = vm.count("bytes") ? Unit::Bytes : Unit::Blocks;
    std::atomic<uintmax_t> summary = 0;
    boost::thread_group threads;
    const auto thread_count = boost::thread::hardware_concurrency();

    for (size_t i = 0; i < thread_count; ++i)
    {
        auto t = [=, &summary, &paths] {
            for (size_t j = i; j < paths.size(); j += thread_count) {
                const auto& path = paths[j];
                const auto size = get_total_size(path, unit, callback, error_handler);
                summary += size;

                if (!need_print_only_summary) {
                    std::cout << format(path, size);
                }
            }
        };

        threads.create_thread(std::move(t));
    }

    threads.join_all();

    if (vm.count("summary") || need_print_only_summary) {
        std::cout << "\nSummary: " << summary << '\n';
    }

    return EXIT_SUCCESS;
}
