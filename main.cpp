#include <iostream>
#include <vector>
#include <string>

#include <boost/program_options.hpp>

int main(int argc, char** argv)
{
    using namespace boost::program_options;

    options_description desc("Options");
    desc.add_options()
        ("help,h", "produce help message")
        (",b", "exact size in bytes")
        (",a", "data of all files")
        (",c", "summary of total size")
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

    if (vm.count("input-paths")) {
        auto x = vm["input-paths"].as<std::vector<std::string>>();
        for (auto& t : x) std::cout << t << "\n";

    }

    return EXIT_SUCCESS;
}
