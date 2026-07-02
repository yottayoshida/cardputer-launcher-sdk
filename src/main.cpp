// SPDX-License-Identifier: MIT OR Apache-2.0

#include <M5Cardputer.h>
#include <SD.h>
#include <SPI.h>

#include "CardputerLauncher.h"

#define SD_SPI_SCK_PIN 40
#define SD_SPI_MISO_PIN 39
#define SD_SPI_MOSI_PIN 14
#define SD_SPI_CS_PIN 12

using namespace cardputer_launcher;

namespace {

DisplayPort display;
Toast toast;
Keyboard keyboard;
ConfigLoader config;
LogStore logs;
WifiManager wifi;
HttpClient http;
AppRegistry registry;
Launcher launcher(registry);
WebhookLauncherApp webhookApp;
LogViewerApp logViewerApp;
AboutApp aboutApp;

bool initializeSd() {
  SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);
  if (!SD.begin(SD_SPI_CS_PIN, SPI, 25000000)) {
    return false;
  }
  return SD.cardType() != CARD_NONE;
}

}  // namespace

void setup() {
  Serial.begin(115200);
  auto cfg = M5.config();
  M5Cardputer.begin(cfg, true);
  display.begin();

  const bool sdAvailable = initializeSd();
  config.setSdAvailable(sdAvailable);
  logs.begin(sdAvailable);

  registry.add(&webhookApp);
  registry.add(&logViewerApp);
  registry.add(&aboutApp);

  AppContext ctx{display, config, logs, wifi, http, toast, sdAvailable};
  launcher.begin(ctx);
  launcher.render(ctx);
}

void loop() {
  M5Cardputer.update();

  AppContext ctx{display, config, logs, wifi, http, toast, config.sdAvailable()};
  InputEvent event = keyboard.poll();
  launcher.handleInput(ctx, event);
  launcher.tick(ctx);
  launcher.render(ctx);
  delay(30);
}
