#include "screen.hxx"
#include "utils.hxx"

Screen::Screen()
{
  print("\033[?1049h", false);
}

Screen::~Screen()
{
  print("\033[?1049l", false);
}

Screen&
Screen::clear()
{
  print("\033[2J", false);
  return *this;
}

Screen&
Screen::go_home()
{
  print("\033[H", false);
  return *this;
}
