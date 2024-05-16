#pragma once

class Screen {
public:

  /**
   * Stores a current terminal state and creates
   * an alternative buffer for subsequent output.
   */
  Screen();

  Screen(const Screen&) = default;
  Screen(Screen&&) = delete;
  Screen& operator=(const Screen&) = default;
  Screen& operator=(Screen&&) = delete;

  /**
   * Returns from the alternative buffer that was created
   * in constructor and restore the original terminal state.
   */
  ~Screen();

  /**
   * Clears the terminal screen
   */
  Screen& clear();

  /**
   * Moves a cursor to the home position (0, 0)
   */
  Screen& go_home();
};
