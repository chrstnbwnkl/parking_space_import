#pragma once
#include <valhalla/mjolnir/osmnode.h>
namespace parking_spots {

struct parking_space_node {
  valhalla::mjolnir::OSMNode node;
  float level;
  float level_precision;
};

static_assert(sizeof(parking_space_node) == 48u);
static_assert(std::is_trivially_copyable_v<parking_space_node>,
              "parking_space_node must be trivially copyable");
} // namespace parking_spots
