#include "parking_spaces/parking_spaces.h"

#include <valhalla/gurka.h>
#include <valhalla/midgard/sequence.h>
#include <valhalla/mjolnir/util.h>
#include <valhalla/test.h>

#include <gtest/gtest.h>

#ifndef PS_ROOT
#def PS_ROOT
#endif

#ifndef PS_BUILD_DIR
#def PS_BUILD_DIR
#endif

using namespace valhalla;
using namespace valhalla::mjolnir;
using namespace valhalla::gurka;

namespace {
constexpr auto kMapEpsilon = 0.002;

std::string point_ll_diff(midgard::PointLL& a, midgard::PointLL& b) {
  return std::format("A lat({}),lon({}) vs. B lat({}),lon({}) | Distance(m): {}", a.lat(), a.lng(),
                     b.lat(), b.lng(), a.Distance(b));
}

map buildtiles_parking(const nodelayout& layout,
                       const ways& ways,
                       const nodes& nodes,
                       const relations& relations,
                       const boost::property_tree::ptree& config) {

  map result{config, layout};
  auto workdir = config.get<std::string>("mjolnir.tile_dir");

  // Sanity check so that we don't blow away / by mistake
  if (workdir == "/") {
    throw std::runtime_error("Can't use / for tests, as we need to clean it out first");
  }

  if (std::filesystem::exists(workdir))
    std::filesystem::remove_all(workdir);
  std::filesystem::create_directories(workdir);

  auto pbf_filename = workdir + "/map.pbf";
  std::cerr << "[          ] generating map PBF at " << pbf_filename << std::endl;
  detail::build_pbf(result.nodes, ways, nodes, relations, pbf_filename);
  std::cerr << "[          ] building tiles in " << result.config.get<std::string>("mjolnir.tile_dir")
            << std::endl;
  midgard::logging::Configure({{"type", ""}});

  mjolnir::build_tile_set(result.config, {pbf_filename}, mjolnir::BuildStage::kInitialize,
                          mjolnir::BuildStage::kTransit);
  parking_spaces::process_parking_spaces(result.config, pbf_filename);
  mjolnir::build_tile_set(result.config, {pbf_filename}, mjolnir::BuildStage::kHierarchy,
                          mjolnir::BuildStage::kValidate);

  return result;
}
} // namespace

TEST(StandAlone, parse_nodes_empty) {

  auto conf =
      test::make_config(PS_BUILD_DIR "/test/data/parse_nodes", {{"mjolnir.concurrency", "1"}});

  std::filesystem::create_directories(PS_BUILD_DIR "/test/data/parse_nodes");

  const std::string ascii_map = R"(
      A--------B
      | 4  1   |
      |  3   2 |
      C--------D
    )";
  auto layout = gurka::detail::map_to_coordinates(ascii_map, 10);
  gurka::ways ways = {
      {"AB", {{"highway", "residential"}}},
  };

  const auto pbf_file = PS_BUILD_DIR "/test/data/parse_nodes/map.pbf";
  gurka::detail::build_pbf(layout, ways, {}, {}, pbf_file);

  parking_spaces::process_parking_spaces(conf, pbf_file);
  EXPECT_EQ(midgard::sequence<
                parking_spaces::parking_space_node>(PS_BUILD_DIR
                                                    "/test/data/parse_nodes/parking_space.bin",
                                                    false)
                .size(),
            0);
}

TEST(StandAlone, parse_nodes_basic) {

  std::string data_dir = PS_BUILD_DIR "/test/data/parse_nodes_basic";
  auto conf = test::make_config(data_dir, {{"mjolnir.concurrency", "1"}});

  std::filesystem::create_directories(data_dir);

  const std::string ascii_map = R"(
      A--------B
      | 4  1   |
      |  3   2 |
      C--------D
    )";
  auto layout = gurka::detail::map_to_coordinates(ascii_map, 10, {7.5, 52.54});
  gurka::ways ways = {
      {"AB", {{"highway", "residential"}}},
  };

  gurka::nodes nodes{
      {"1", {{"amenity", "parking_space"}, {"osm_id", "12"}}},
      {"2", {{"amenity", "parking_space"}, {"osm_id", "13"}}},
      {"3", {{"amenity", "parking_space"}, {"osm_id", "14"}}},
      {"4", {{"amenity", "parking_space"}, {"osm_id", "15"}}},
  };

  const auto pbf_file = data_dir + "/map.pbf";
  gurka::detail::build_pbf(layout, ways, nodes, {}, pbf_file);

  parking_spaces::process_parking_spaces(conf, pbf_file);

  midgard::sequence<parking_spaces::parking_space_node> seq(data_dir + "/parking_space.bin");

  {
    EXPECT_EQ(seq.size(), 4);
    EXPECT_EQ((*seq.at(0)).node.osmid_, 12);
    EXPECT_NEAR((*seq.at(0)).node.latlng().lat(), 52.54, kMapEpsilon);
    EXPECT_NEAR((*seq.at(0)).node.latlng().lng(), 7.5001, kMapEpsilon);
    EXPECT_EQ((*seq.at(0)).level, parking_spaces::kInvalidLevel);
  }

  {
    EXPECT_EQ((*seq.at(1)).node.osmid_, 13);
    EXPECT_NEAR((*seq.at(1)).node.latlng().lat(), 52.54, kMapEpsilon);
    EXPECT_NEAR((*seq.at(1)).node.latlng().lng(), 7.5001, kMapEpsilon);
    EXPECT_EQ((*seq.at(1)).level, parking_spaces::kInvalidLevel);
  }

  {
    EXPECT_EQ((*seq.at(2)).node.osmid_, 14);
    EXPECT_NEAR((*seq.at(2)).node.latlng().lat(), 52.54, kMapEpsilon);
    EXPECT_NEAR((*seq.at(2)).node.latlng().lng(), 7.5001, kMapEpsilon);
    EXPECT_EQ((*seq.at(2)).level, parking_spaces::kInvalidLevel);
  }

  {
    EXPECT_EQ((*seq.at(3)).node.osmid_, 15);
    EXPECT_NEAR((*seq.at(3)).node.latlng().lat(), 52.54, kMapEpsilon);
    EXPECT_NEAR((*seq.at(3)).node.latlng().lng(), 7.5001, kMapEpsilon);
    EXPECT_EQ((*seq.at(3)).level, parking_spaces::kInvalidLevel);
  }
}

TEST(StandAlone, parse_nodes_levels) {

  std::string data_dir = PS_BUILD_DIR "/test/data/parse_nodes_basic";
  auto conf = test::make_config(data_dir, {{"mjolnir.concurrency", "1"}});

  std::filesystem::create_directories(data_dir);

  const std::string ascii_map = R"(
      A-a-----------------B
      | 4                 |
      |                  1|
      |                   |6
      |        3    5   2 |
      C-------------------D
    )";
  auto layout = gurka::detail::map_to_coordinates(ascii_map, 10, {7.5, 52.54});
  gurka::ways ways = {
      {"AaB", {{"highway", "residential"}}},
      {"AC", {{"highway", "residential"}}},
      {"CD", {{"highway", "residential"}}},
      {"DB", {{"highway", "residential"}}},
  };

  gurka::nodes nodes{
      {"1", {{"amenity", "parking_space"}, {"level", "1"}, {"osm_id", "12"}}},
      {"2", {{"amenity", "parking_space"}, {"level", "0"}, {"osm_id", "13"}}},
      {"3", {{"amenity", "parking_space"}, {"level", "-12"}, {"osm_id", "14"}}},
      {"4", {{"amenity", "parking_space"}, {"level", "566"}, {"osm_id", "15"}}},
      {"5", {{"amenity", "parking_space"}, {"osm_id", "16"}}},
      {"6", {{"amenity", "parking_space"}, {"level", "75.35"}, {"osm_id", "17"}}},
  };

  const auto pbf_file = data_dir + "/map.pbf";
  gurka::detail::build_pbf(layout, ways, nodes, {}, pbf_file);
  buildtiles_parking(layout, ways, nodes, {}, conf);

  midgard::sequence<parking_spaces::parking_space_node> seq(data_dir + "/parking_space.bin");

  EXPECT_EQ(seq.size(), 6);

  {
    EXPECT_EQ((*seq.at(0)).node.osmid_, 12);
    EXPECT_NEAR((*seq.at(0)).node.latlng().lat(), 52.54, kMapEpsilon);
    EXPECT_NEAR((*seq.at(0)).node.latlng().lng(), 7.5001, kMapEpsilon);
    EXPECT_EQ((*seq.at(0)).level, 1.f);
  }

  {
    EXPECT_EQ((*seq.at(1)).node.osmid_, 13);
    EXPECT_NEAR((*seq.at(1)).node.latlng().lat(), 52.54, kMapEpsilon);
    EXPECT_NEAR((*seq.at(1)).node.latlng().lng(), 7.5001, kMapEpsilon);
    EXPECT_EQ((*seq.at(1)).level, 0.f);
  }

  {
    EXPECT_EQ((*seq.at(2)).node.osmid_, 14);
    EXPECT_NEAR((*seq.at(2)).node.latlng().lat(), 52.54, kMapEpsilon);
    EXPECT_NEAR((*seq.at(2)).node.latlng().lng(), 7.5001, kMapEpsilon);
    EXPECT_EQ((*seq.at(2)).level, -12.f);
    EXPECT_NEAR((*seq.at(2)).level_precision, 0.f, 0.0001f);
  }

  {
    EXPECT_EQ((*seq.at(3)).node.osmid_, 15);
    EXPECT_NEAR((*seq.at(3)).node.latlng().lat(), 52.54, kMapEpsilon);
    EXPECT_NEAR((*seq.at(3)).node.latlng().lng(), 7.5001, kMapEpsilon);
    EXPECT_EQ((*seq.at(3)).level, 566.f);
    EXPECT_NEAR((*seq.at(3)).level_precision, 0.f, 0.00001f);
  }

  {
    EXPECT_EQ((*seq.at(4)).node.osmid_, 16);
    EXPECT_NEAR((*seq.at(4)).node.latlng().lat(), 52.54, kMapEpsilon);
    EXPECT_NEAR((*seq.at(4)).node.latlng().lng(), 7.5001, kMapEpsilon);
  }

  {
    EXPECT_EQ((*seq.at(5)).node.osmid_, 17);
    EXPECT_NEAR((*seq.at(5)).node.latlng().lat(), 52.54, kMapEpsilon);
    EXPECT_NEAR((*seq.at(5)).node.latlng().lng(), 7.5001, kMapEpsilon);
    EXPECT_EQ((*seq.at(5)).level, 75.35f);
    EXPECT_EQ((*seq.at(5)).level_precision, 2.f);
  }

  // find node 4
  {
    auto reader = test::make_clean_graphreader(conf.get_child("mjolnir"));
    const auto& node = gurka::findNode(*reader, layout, "A");
    auto ni = reader->nodeinfo(node);
    EXPECT_EQ(ni->edge_count(), 3);

    auto edge_id = node;
    edge_id.set_id(ni->edge_index() + (ni->edge_count() - 1));
    const auto* de = reader->directededge(edge_id);
    EXPECT_EQ(de->forwardaccess(),
              valhalla::baldr::kVehicularAccess | valhalla::baldr::kPedestrianAccess);

    auto node4 = de->endnode();
    auto ni4 = reader->nodeinfo(node4);

    // check the new node
    EXPECT_EQ(ni4->edge_count(), 2);
    EXPECT_EQ(ni4->type(), baldr::NodeType::kParking);

    midgard::DistanceApproximator<midgard::PointLL> da(layout.at("A"));

    // check the outgoing edges
    edge_id.set_id(ni4->edge_index());
    auto _4out1 = reader->directededge(edge_id);
    EXPECT_EQ(_4out1->use(), baldr::Use::kParkingAisle);
    EXPECT_EQ(_4out1->classification(), baldr::RoadClass::kServiceOther);
    EXPECT_EQ(_4out1->length(), 22); // in ASCII map units, this should be 3 * 10m grid size, but
                                     // gurka does some weird projection to mercator and then back
                                     // so this does not work; find a better way to test this

    auto _4ei1 = reader->edgeinfo(edge_id);
    auto _4out1_shape = _4ei1.shape();

    EXPECT_EQ(_4out1_shape.size(), 3);
    EXPECT_TRUE(_4out1_shape.at(0).ApproximatelyEqual(layout.at("A")))
        << point_ll_diff(_4out1_shape.at(0), layout.at("A"));
    EXPECT_TRUE(_4out1_shape.at(1).ApproximatelyEqual(layout.at("a")))
        << point_ll_diff(_4out1_shape.at(1), layout.at("a"));
    EXPECT_TRUE(_4out1_shape.at(2).ApproximatelyEqual(layout.at("4")))
        << point_ll_diff(_4out1_shape.at(2), layout.at("4"));

    EXPECT_EQ(_4ei1.levels().first.size(), 1);
    EXPECT_EQ(_4ei1.levels().first.at(0).first, 566);
    EXPECT_EQ(_4ei1.levels().first.at(0).second, 566);
    EXPECT_EQ(_4ei1.levels().second, 0);

    edge_id.set_id(ni4->edge_index() + 1);
    auto _4out2 = reader->directededge(edge_id);
    EXPECT_EQ(_4out2->use(), baldr::Use::kParkingAisle);
    EXPECT_EQ(_4out2->classification(), baldr::RoadClass::kServiceOther);
    auto _4ei2 = reader->edgeinfo(edge_id);
    EXPECT_EQ(_4ei2.levels().first.size(), 1);
    EXPECT_EQ(_4ei2.levels().first.at(0).first, 566);
    EXPECT_EQ(_4ei2.levels().first.at(0).second, 566);
    EXPECT_EQ(_4ei2.levels().second, 0);
  }

  // find node 3
  {

    auto reader = test::make_clean_graphreader(conf.get_child("mjolnir"));
    const auto& node = gurka::findNode(*reader, layout, "C");
    auto ni = reader->nodeinfo(node);
    EXPECT_EQ(ni->edge_count(), 5); // C->D, C->A, plus the 3 edges to parking nodes 3,5 and 2

    auto edge_id = node;
    edge_id.set_id(ni->edge_index() + (ni->edge_count() - 1));
    const auto* de = reader->directededge(edge_id);
    EXPECT_EQ(de->forwardaccess(),
              valhalla::baldr::kVehicularAccess | valhalla::baldr::kPedestrianAccess);
  }
}
