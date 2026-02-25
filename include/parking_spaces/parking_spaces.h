#pragma once

#include "boost/property_tree/ptree_fwd.hpp"
#include "node.h"

#include <valhalla/mjolnir/osmnode.h>

namespace parking_spaces {
constexpr float kInvalidLevel = std::numeric_limits<float>::max();

void process_parking_spaces(const boost::property_tree::ptree&, std::string_view);
} // namespace parking_spaces
