#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

namespace cardputer_launcher {

enum class InputAction {
  None,
  Up,
  Down,
  Left,
  Right,
  Select,
  Back,
  Clear,
  ToggleSearch,
  Confirm,
  Cancel,
  Character,
};

// TextEntry lets w/s/y/n reach the caller as literal Character events instead
// of being intercepted as legacy navigation shortcuts.
enum class InputMode {
  Navigation,
  TextEntry,
};

struct InputEvent {
  InputEvent() = default;
  InputEvent(InputAction inputAction, char inputCharacter)
      : action(inputAction), character(inputCharacter) {}

  InputAction action = InputAction::None;
  char character = '\0';
};

class Keyboard {
 public:
  InputEvent poll(InputMode mode = InputMode::Navigation);
};

}  // namespace cardputer_launcher
