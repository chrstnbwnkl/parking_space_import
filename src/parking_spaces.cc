#include "parking_spaces/parking_spaces.h"

#include <valhalla/midgard/logging.h>
#include <valhalla/midgard/sequence.h>

#include <boost/property_tree/ptree.hpp>
#include <osmium/io/pbf_input.hpp>

namespace {

constexpr std::string_view kTempSequencePath = "/parking_space.bin";
constexpr std::string_view kParkingSpaceValue = "parking_space";
constexpr std::string_view kParkingSpaceKey = "amenity";
constexpr std::string_view kLevelKey = "level";

using namespace valhalla::midgard;

/**
 * Parking space nodes cannot be on more than one level.
 * So parse single numeric values, and throw if we find
 * a multi-level value
 */
float parse_level(std::string_view s) {
  float value;
  auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);
  if (ec != std::errc{} || ptr != s.data() + s.size()) {
    throw std::invalid_argument("Not a single numeric value: " + std::string(s));
  }
  return value;
}

class tag_parser {

public:
  explicit tag_parser(std::string_view tmp_sequence_path)
      : sequence_(tmp_sequence_path.data(), true) {};

  bool parse_node(const osmium::Node& node) {
    // nothing to do
    if (node.tags().empty()) {
      return false;
    }

    bool found_parking = false;
    std::string level_value = "";
    for (const auto& tag : node.tags()) {
      if (std::string_view(tag.key()) == kParkingSpaceKey &&
          std::string_view(tag.value()) == kParkingSpaceValue) {
        found_parking = true;
      }

      if (std::string_view(tag.key()) == kLevelKey) {
        level_value = tag.value();
      }
    }

    if (!found_parking) {
      return false;
    }

    // we've found one we care about
    parking_spots::parking_space_node ps_node;
    ps_node.osm_id = node.id();

    if (!level_value.empty()) {
      try {
        ps_node.level = parse_level(level_value);
      } catch (const std::invalid_argument& e) {
        LOG_WARN("Found multi-level parking space: {}; error: {}", ps_node.osm_id, e.what());
        return false;
      }
    } else {
      ps_node.level = parking_spots::kInvalidLevel;
    }

    ps_node.lat = node.location().lat();
    ps_node.lon = node.location().lon();

    sequence_.push_back(ps_node);
    return true;
  }

protected:
  sequence<parking_spots::parking_space_node> sequence_;
};

/**
 * Parse nodes marked with amenity=parking_space into a sequence
 * of structs that we can use to correlate parking spaces to a Valhalla graph
 */
void parse_osm(std::string_view osm_file, std::string_view tmp_dir, bool& found_any) {

  osmium::io::Reader reader(osm_file.data(), osmium::osm_entity_bits::node);
  std::string tmp_fp = tmp_dir.data() + std::string(kTempSequencePath.data());
  tag_parser parser(tmp_fp);
  while (const osmium::memory::Buffer buffer = reader.read()) {
    for (const osmium::memory::Item& item : buffer) {
      found_any |= parser.parse_node(static_cast<const osmium::Node&>(item));
    }
  }
  reader.close(); // Explicit close to get an exception in case of an error.
  LOG_INFO("Wrote sequence to {}", tmp_fp);
}
} // namespace

namespace parking_spots {

void process_parking_spaces(const boost::property_tree::ptree& config, std::string_view osm_file) {
  LOG_INFO("Processing parking  spaces...");
  auto tmp_dir = config.get<std::string>("mjolnir.tile_dir");

  bool found_any = false;
  parse_osm(osm_file, tmp_dir, found_any);

  if (found_any) {
    LOG_INFO("Done parsing parking spaces, found {} nodes",
             sequence<parking_space_node>(std::string(tmp_dir) + kTempSequencePath.data(), false)
                 .size());
  } else {
    LOG_WARN("Did not find any parking space nodes");
  }
}
} // namespace parking_spots
