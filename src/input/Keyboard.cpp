// SPDX-License-Identifier: MIT OR Apache-2.0

#include "input/Keyboard.h"

#include <M5Cardputer.h>

namespace cardputer_launcher {

InputEvent Keyboard::poll() {
  if (!M5Cardputer.Keyboard.isChange() || !M5Cardputer.Keyboard.isPressed()) {
    return {};
  }

  Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
  if (status.enter) {
    return {InputAction::Select, '\n'};
  }
  if (status.del) {
    return {InputAction::Back, '\b'};
  }

  for (char key : status.word) {
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
    return {InputAction::Character, key};
  }

  return {};
}

}  // namespace cardputer_launcher

