#pragma once

#include "boost/property_tree/ptree_fwd.hpp"

#include <cstdint>
#include <numeric>

namespace parking_spots {
constexpr float kInvalidLevel = std::numeric_limits<float>::max();

struct parking_space_node {
  uint64_t osm_id;
  double lat;
  double lon;
  float level;
  uint32_t __padding;
};
static_assert(sizeof(parking_space_node) == 32u);
static_assert(std::is_trivially_copyable_v<parking_space_node>,
              "parking_space_node must be trivially copyable");

void process_parking_spaces(const boost::property_tree::ptree&, std::string_view);
} // namespace parking_spots
