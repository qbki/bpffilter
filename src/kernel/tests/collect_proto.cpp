// NOLINTBEGIN(cppcoreguidelines-avoid-do-while)
#include <catch2/catch_test_macros.hpp>
#include <linux/ip.h>

#include "../common.h"
#include "../utils.bpf.h"

TEST_CASE("collect_proto")
{
  auto make_config = [=]() -> ConfigData {
    return { .filter_flags = FILTER_TCP | FILTER_UDP };
  };

  SECTION("should ignore capturing when IP protocol is neither TCP nor UDP")
  {
    auto config = make_config();
    __u8 expected_flags = FILTER_TCP | FILTER_UDP;
    iphdr ip_header{ .protocol = IPPROTO_ICMP };
    REQUIRE(collect_proto(&config, &ip_header, &expected_flags) == 0);
    REQUIRE(expected_flags == (FILTER_TCP | FILTER_UDP));
  }

  SECTION("should ignore capturing when filter flags are not set")
  {
    ConfigData config{ .filter_flags = 0 };
    __u8 expected_flags = 0;
    iphdr ip_header{ .protocol = IPPROTO_TCP };
    REQUIRE(collect_proto(&config, &ip_header, &expected_flags) == 0);
    REQUIRE(expected_flags == 0);
    ip_header.protocol = IPPROTO_UDP;
    REQUIRE(collect_proto(&config, &ip_header, &expected_flags) == 0);
    REQUIRE(expected_flags == 0);
  }

  SECTION("should set capture flag when protocol is TCP")
  {
    auto config = make_config();
    __u8 expected_flags = FILTER_TCP | FILTER_UDP;
    iphdr ip_header{ .protocol = IPPROTO_TCP };
    REQUIRE(collect_proto(&config, &ip_header, &expected_flags) == FILTER_TCP);
    REQUIRE(expected_flags == FILTER_TCP);
  }

  SECTION("should set capture flag when protocol is UDP")
  {
    auto config = make_config();
    __u8 expected_flags = FILTER_TCP | FILTER_UDP;
    iphdr ip_header{ .protocol = IPPROTO_UDP };
    REQUIRE(collect_proto(&config, &ip_header, &expected_flags) == FILTER_UDP);
    REQUIRE(expected_flags == FILTER_UDP);
  }
}
// NOLINTEND(cppcoreguidelines-avoid-do-while)
