// NOLINTBEGIN(cppcoreguidelines-avoid-do-while)
#include <catch2/catch_test_macros.hpp>

#include "../common.h"
#include "../utils.bpf.h"

TEST_CASE("collect_ports")
{
  const __u32 SRC = 1;
  const __u32 DST = 2;
  const __u32 WRONG_SRC = 1001;
  const __u32 WRONG_DST = 1002;
  auto make_config = [=]() -> ConfigData {
    return { .src_port = SRC, .dst_port = DST };
  };
  SECTION("should ignore ports when config flags are zero")
  {
    auto config = make_config();
    REQUIRE(collect_ports(&config, SRC, DST) == 0);
    REQUIRE(collect_ports(&config, SRC, WRONG_DST) == 0);
    REQUIRE(collect_ports(&config, WRONG_SRC, DST) == 0);
    REQUIRE(collect_ports(&config, WRONG_SRC, WRONG_DST) == 0);
  }

  SECTION("should set capture flags when ports are equal")
  {
    auto config = make_config();
    auto both_ports = FILTER_SRC_PORT | FILTER_DST_PORT;
    config.filter_flags = both_ports;
    REQUIRE(collect_ports(&config, SRC, DST) == both_ports);
    REQUIRE(collect_ports(&config, SRC, WRONG_DST) == FILTER_SRC_PORT);
    REQUIRE(collect_ports(&config, WRONG_SRC, DST) == FILTER_DST_PORT);
    REQUIRE(collect_ports(&config, WRONG_SRC, WRONG_DST) == 0);
  }

  SECTION("should set a capture flag for the source port only")
  {
    auto config = make_config();
    config.filter_flags = FILTER_SRC_PORT;
    REQUIRE(collect_ports(&config, SRC, DST) == FILTER_SRC_PORT);
    REQUIRE(collect_ports(&config, SRC, WRONG_DST) == FILTER_SRC_PORT);
    REQUIRE(collect_ports(&config, WRONG_SRC, DST) == 0);
    REQUIRE(collect_ports(&config, WRONG_SRC, WRONG_DST) == 0);
  }

  SECTION("should set a capture flag for a destination port only")
  {
    auto config = make_config();
    config.filter_flags = FILTER_DST_PORT;
    REQUIRE(collect_ports(&config, SRC, DST) == FILTER_DST_PORT);
    REQUIRE(collect_ports(&config, SRC, WRONG_DST) == 0);
    REQUIRE(collect_ports(&config, WRONG_SRC, DST) == FILTER_DST_PORT);
    REQUIRE(collect_ports(&config, WRONG_SRC, WRONG_DST) == 0);
  }
}
// NOLINTEND(cppcoreguidelines-avoid-do-while)
