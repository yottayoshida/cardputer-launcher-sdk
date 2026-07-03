// SPDX-License-Identifier: MIT OR Apache-2.0

#include "input/Keyboard.h"

#include <M5Cardputer.h>

namespace cardputer_launcher {

InputEvent Keyboard::poll(InputMode mode) {
  if (!M5Cardputer.Keyboard.isChange() || !M5Cardputer.Keyboard.isPressed()) {
    return {};
  }

  Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

  // fn-layer keys never coexist with status.word (the driver returns before
  // populating word while fn is held), so these are always safe to check
  // first regardless of mode.
  if (status.esc) {
    return {InputAction::Cancel, 0};
  }
  if (status.tab) {
    return {InputAction::ToggleSearch, 0};
  }
  if (status.up) {
    return {InputAction::Up, 0};
  }
  if (status.down) {
    return {InputAction::Down, 0};
  }
  if (status.left) {
    return {InputAction::Left, 0};
  }
  if (status.right) {
    return {InputAction::Right, 0};
  }
  if (status.del) {
    return {InputAction::Clear, 0};
  }
  if (status.enter) {
    return {InputAction::Select, '\n'};
  }
  if (status.backspace) {
    return {InputAction::Back, '\b'};
  }

  for (char key : status.word) {
    if (mode == InputMode::Navigation) {
      if (key == 'w' || key == 'W' || key == 'k' || key == 'K') {
        return {InputAction::Up, key};
      }
      if (key == 's' || key == 'S' || key == 'j' || key == 'J') {
        return {InputAction::Down, key};
      }
      if (key == 'y' || key == 'Y') {
        return {InputAction::Confirm, key};
      }
      if (key == 'n' || key == 'N') {
        return {InputAction::Cancel, key};
      }
    }
    return {InputAction::Character, key};
  }

  return {};
}

}  // namespace cardputer_launcher

