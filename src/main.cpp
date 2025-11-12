#include <M5Unified.h>
#include <M5GFX.h>

M5GFX display;
M5Canvas canvas(&display);

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  display.begin();
  canvas.setColorDepth(16);
  canvas.setTextSize(2);
  canvas.createSprite(display.width(), 48);

  canvas.fillScreen(TFT_BLACK);
  canvas.setTextDatum(textdatum_t::middle_center);
  canvas.setTextColor(TFT_WHITE);
  canvas.drawString("Hello Tab5 (ESP32-P4 + M5GFX)", canvas.width()/2, 24);
  canvas.pushSprite(0, (display.height()-48)/2);
}

void loop() { M5.update(); delay(10); }

