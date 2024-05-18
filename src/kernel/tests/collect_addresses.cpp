// NOLINTBEGIN(cppcoreguidelines-avoid-do-while)
#include <catch2/catch_test_macros.hpp>

#include "../common.h"
#include "../utils.bpf.h"

TEST_CASE("collect_addresses")
{
  const __u32 SRC = 1;
  const __u32 DST = 2;
  const __u32 WRONG_SRC = 1001;
  const __u32 WRONG_DST = 1002;
  auto make_config = [=]() -> ConfigData {
    return { .src_address = SRC, .dst_address = DST };
  };
  SECTION("should ignore addresses when config flags are zero")
  {
    auto config = make_config();
    REQUIRE(collect_addresses(&config, SRC, DST) == 0);
    REQUIRE(collect_addresses(&config, SRC, WRONG_DST) == 0);
    REQUIRE(collect_addresses(&config, WRONG_SRC, DST) == 0);
    REQUIRE(collect_addresses(&config, WRONG_SRC, WRONG_DST) == 0);
  }

  SECTION("should set capture flags when addresses are equal")
  {
    auto config = make_config();
    auto both_addresses = FILTER_SRC_ADDRESS | FILTER_DST_ADDRESS;
    config.filter_flags = both_addresses;
    REQUIRE(collect_addresses(&config, SRC, DST) == both_addresses);
    REQUIRE(collect_addresses(&config, SRC, WRONG_DST) == FILTER_SRC_ADDRESS);
    REQUIRE(collect_addresses(&config, WRONG_SRC, DST) == FILTER_DST_ADDRESS);
    REQUIRE(collect_addresses(&config, WRONG_SRC, WRONG_DST) == 0);
  }

  SECTION("should set a capture flag for the source address only")
  {
    auto config = make_config();
    config.filter_flags = FILTER_SRC_ADDRESS;
    REQUIRE(collect_addresses(&config, SRC, DST) == FILTER_SRC_ADDRESS);
    REQUIRE(collect_addresses(&config, SRC, WRONG_DST) == FILTER_SRC_ADDRESS);
    REQUIRE(collect_addresses(&config, WRONG_SRC, DST) == 0);
    REQUIRE(collect_addresses(&config, WRONG_SRC, WRONG_DST) == 0);
  }

  SECTION("should set a capture flag for the destination address only")
  {
    auto config = make_config();
    config.filter_flags = FILTER_DST_ADDRESS;
    REQUIRE(collect_addresses(&config, SRC, DST) == FILTER_DST_ADDRESS);
    REQUIRE(collect_addresses(&config, SRC, WRONG_DST) == 0);
    REQUIRE(collect_addresses(&config, WRONG_SRC, DST) == FILTER_DST_ADDRESS);
    REQUIRE(collect_addresses(&config, WRONG_SRC, WRONG_DST) == 0);
  }
}
// NOLINTEND(cppcoreguidelines-avoid-do-while)
