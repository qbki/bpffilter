#include <ranges>

#include "utils.hxx"

std::vector<std::string>
split(const std::string& value, const std::string& delimiter)
{
  auto view = value | std::views::split(delimiter)
                    | std::views::transform([](const auto& v) {
                        return std::string(v.begin(), v.end());
                      });
  return {view.begin(), view.end()};
}
