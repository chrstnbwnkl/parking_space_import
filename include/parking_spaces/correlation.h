#pragma once
#include <valhalla/mjolnir/osmdata.h>

#include <boost/property_tree/ptree_fwd.hpp>
namespace parking_spaces {
void correlate_parking_spaces(const boost::property_tree::ptree& pt,
                              const std::string& parking_nodes_bin);
} // namespace parking_spaces
