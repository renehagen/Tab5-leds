#include <M5Unified.h>
#include <M5GFX.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN   54
#define NUM_LEDS  1

// Canvas is tied to the single, M5Unified-owned display
M5Canvas canvas(&M5.Display);
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  auto cfg = M5.config();
  cfg.output_power  = true;   // keep Grove 5V on
  cfg.clear_display = true;   // force clean panel each boot
  M5.begin(cfg);

  delay(50);                  // let LCD/backlight settle

  auto& lcd = M5.Display;     // single display owner
  lcd.setColorDepth(16);      // RGB565
  lcd.setSwapBytes(false);    // if you still get green, change to 'true'
  lcd.fillScreen(TFT_BLACK);

  canvas.setColorDepth(16);
  canvas.setTextSize(2);
  canvas.createSprite(lcd.width(), 48);
  canvas.fillScreen(TFT_BLACK);
  canvas.setTextDatum(textdatum_t::middle_center);
  canvas.setTextColor(TFT_WHITE);
  canvas.drawString("Hello Tab5 (ESP32-P4 + M5GFX)", canvas.width()/2, 24);
  canvas.pushSprite(0, (lcd.height()-48)/2);

  // NeoPixel
  strip.begin();
  strip.clear();
  strip.setBrightness(48);
  strip.show();
}

void loop() {
  // Blink first pixel on GPIO 54
  strip.setPixelColor(0, strip.Color(255, 0, 0));
  strip.show();
  delay(500);
  strip.setPixelColor(0, 0);
  strip.show();
  delay(500);

  M5.update();
}
