#include "parking_spaces/parking_spaces.h"

#include <valhalla/gurka.h>
#include <valhalla/midgard/sequence.h>
#include <valhalla/test.h>

#include <gtest/gtest.h>

#ifndef PS_ROOT
#def PS_ROOT
#endif

#ifndef PS_BUILD_DIR
#def PS_BUILD_DIR
#endif

using namespace valhalla;

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

  parking_spots::process_parking_spaces(conf, pbf_file);
  EXPECT_EQ(midgard::sequence<
                parking_spots::parking_space_node>(PS_BUILD_DIR
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

  parking_spots::process_parking_spaces(conf, pbf_file);

  midgard::sequence<parking_spots::parking_space_node> seq(data_dir + "/parking_space.bin");

  {
    EXPECT_EQ(seq.size(), 4);
    EXPECT_EQ((*seq.at(0)).osm_id, 12);
    EXPECT_NEAR((*seq.at(0)).lat, 52.54, 0.001);
    EXPECT_NEAR((*seq.at(0)).lon, 7.5001, 0.001);
    EXPECT_EQ((*seq.at(0)).level, parking_spots::kInvalidLevel);
  }

  {
    EXPECT_EQ((*seq.at(1)).osm_id, 13);
    EXPECT_NEAR((*seq.at(1)).lat, 52.54, 0.001);
    EXPECT_NEAR((*seq.at(1)).lon, 7.5001, 0.001);
    EXPECT_EQ((*seq.at(1)).level, parking_spots::kInvalidLevel);
  }

  {
    EXPECT_EQ((*seq.at(2)).osm_id, 14);
    EXPECT_NEAR((*seq.at(2)).lat, 52.54, 0.001);
    EXPECT_NEAR((*seq.at(2)).lon, 7.5001, 0.001);
    EXPECT_EQ((*seq.at(2)).level, parking_spots::kInvalidLevel);
  }

  {
    EXPECT_EQ((*seq.at(3)).osm_id, 15);
    EXPECT_NEAR((*seq.at(3)).lat, 52.54, 0.001);
    EXPECT_NEAR((*seq.at(3)).lon, 7.5001, 0.001);
    EXPECT_EQ((*seq.at(3)).level, parking_spots::kInvalidLevel);
  }
}

TEST(StandAlone, parse_nodes_levels) {

  std::string data_dir = PS_BUILD_DIR "/test/data/parse_nodes_basic";
  auto conf = test::make_config(data_dir, {{"mjolnir.concurrency", "1"}});

  std::filesystem::create_directories(data_dir);

  const std::string ascii_map = R"(
      A--------B
      | 4  1   |
      |  3 5 2 |
      C--------D
    )";
  auto layout = gurka::detail::map_to_coordinates(ascii_map, 10, {7.5, 52.54});
  gurka::ways ways = {
      {"AB", {{"highway", "residential"}}},
  };

  gurka::nodes nodes{
      {"1", {{"amenity", "parking_space"}, {"level", "1"}, {"osm_id", "12"}}},
      {"2", {{"amenity", "parking_space"}, {"level", "0"}, {"osm_id", "13"}}},
      {"3", {{"amenity", "parking_space"}, {"level", "-12"}, {"osm_id", "14"}}},
      {"4", {{"amenity", "parking_space"}, {"level", "566"}, {"osm_id", "15"}}},
      {"5", {{"amenity", "parking_space"}, {"osm_id", "16"}}},
  };

  const auto pbf_file = data_dir + "/map.pbf";
  gurka::detail::build_pbf(layout, ways, nodes, {}, pbf_file);

  parking_spots::process_parking_spaces(conf, pbf_file);

  midgard::sequence<parking_spots::parking_space_node> seq(data_dir + "/parking_space.bin");

  EXPECT_EQ(seq.size(), 5);
  {
    EXPECT_EQ((*seq.at(0)).osm_id, 12);
    EXPECT_NEAR((*seq.at(0)).lat, 52.54, 0.001);
    EXPECT_NEAR((*seq.at(0)).lon, 7.5001, 0.001);
    EXPECT_EQ((*seq.at(0)).level, 1.f);
  }

  {
    EXPECT_EQ((*seq.at(1)).osm_id, 13);
    EXPECT_NEAR((*seq.at(1)).lat, 52.54, 0.001);
    EXPECT_NEAR((*seq.at(1)).lon, 7.5001, 0.001);
    EXPECT_EQ((*seq.at(1)).level, 0.f);
  }

  {
    EXPECT_EQ((*seq.at(2)).osm_id, 14);
    EXPECT_NEAR((*seq.at(2)).lat, 52.54, 0.001);
    EXPECT_NEAR((*seq.at(2)).lon, 7.5001, 0.001);
    EXPECT_EQ((*seq.at(2)).level, -12.f);
  }

  {
    EXPECT_EQ((*seq.at(3)).osm_id, 15);
    EXPECT_NEAR((*seq.at(3)).lat, 52.54, 0.001);
    EXPECT_NEAR((*seq.at(3)).lon, 7.5001, 0.001);
    EXPECT_EQ((*seq.at(3)).level, 566.f);
  }
}
