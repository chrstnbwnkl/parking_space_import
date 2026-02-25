#include "argparse_utils.h"
#include "cxxopts.hpp"
#include "parking_spaces/parking_spaces.h"

#include <boost/property_tree/ptree.hpp>

int main(int argc, char* argv[]) {
  const auto program = std::filesystem::path(__FILE__).stem().string();
  // args
  std::vector<std::string> input_files;
  boost::property_tree::ptree config;
  cxxopts::Options
      options(program,
              "Parse nodes marked as amenity=parking_space and correlate them to a Valhalla graph");
  auto add_opt = options.add_options();
  add_opt("input", "Input .pbf", cxxopts::value<std::string>());
  add_opt("h,help", "Print this help message.");
  add_opt("v,version", "Print the version of this software.");
  add_opt("c,config", "Path to the configuration file", cxxopts::value<std::string>());
  add_opt("i,inline-config", "Inline JSON config", cxxopts::value<std::string>());

  options.parse_positional({"input"});
  options.positional_help("[INPUT_OSM_FILE]");
  auto result = options.parse(argc, argv);
  if (!parse_common_args(program, options, result, &config, "mjolnir.logging", true))
    return EXIT_SUCCESS;

  parking_spaces::process_parking_spaces(config, result["input"].as<std::string>());
}
