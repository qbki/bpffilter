// NOLINTBEGIN(cppcoreguidelines-avoid-do-while)
#include <catch2/catch_test_macros.hpp>

#include "../utils.bpf.h"

TEST_CASE("remove_bits")
{
  SECTION("should remove bit")
  {
    REQUIRE(remove_bits(0b1, 0b1) == 0b0);
    REQUIRE(remove_bits(0b11, 0b10) == 0b01);
  }
  SECTION("should keep all bits when 0 passed to bits_for_removing")
  {
    REQUIRE(remove_bits(0b11, 0) == 0b11);
    REQUIRE(remove_bits(0b0, 0) == 0b0);
  }
}
// NOLINTEND(cppcoreguidelines-avoid-do-while)
