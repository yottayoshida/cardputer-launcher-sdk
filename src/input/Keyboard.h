#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

namespace cardputer_launcher {

enum class InputAction {
  None,
  Up,
  Down,
  Select,
  Back,
  Confirm,
  Cancel,
  Character,
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
  InputEvent poll();
};

}  // namespace cardputer_launcher
