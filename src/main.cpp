#include <M5Unified.h>
#include <M5GFX.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN   54
#define NUM_LEDS  144

// Display	5 Inch IPS TFT (1280×720, 720P), Driver IC: ST7123

// Canvas is tied to the single, M5Unified-owned display
M5Canvas canvas(&M5.Display);
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Effect mode and animation variables
uint8_t currentEffect = 0;  // 0 = Rainbow, 1 = Fire, 2 = Meteor, 3 = KITT
uint16_t rainbowOffset = 0;
uint16_t animationCounter = 0;

// Button definitions
struct Button {
  int x, y, w, h;
  const char* label;
  uint32_t color;
};

Button buttons[4] = {
  {160, 100, 400, 120, "RAINBOW", TFT_BLUE},
  {160, 240, 400, 120, "FIRE", TFT_RED},
  {160, 380, 400, 120, "METEOR", TFT_PURPLE},
  {160, 520, 400, 120, "KITT", TFT_DARKGREY}
};

// Helper function to generate rainbow colors
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

// Draw buttons on screen
void drawButtons() {
  auto& lcd = M5.Display;
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextDatum(textdatum_t::middle_center);
  lcd.setTextSize(3);
  
  // Title
  lcd.setTextColor(TFT_WHITE);
  lcd.drawString("LED Effect Selector", lcd.width()/2, 80);
  
  // Draw all buttons
  for(int i = 0; i < 4; i++) {
    // Highlight selected button
    uint32_t btnColor = (i == currentEffect) ? TFT_GREEN : buttons[i].color;
    lcd.fillRoundRect(buttons[i].x, buttons[i].y, buttons[i].w, buttons[i].h, 10, btnColor);
    lcd.setTextColor(TFT_WHITE);
    lcd.drawString(buttons[i].label, buttons[i].x + buttons[i].w/2, buttons[i].y + buttons[i].h/2);
  }
}

// Effect 1: Moving Rainbow
void rainbowEffect() {
  for(int i = 0; i < NUM_LEDS; i++) {
    int pixelHue = ((i * 256 / NUM_LEDS) + rainbowOffset) & 255;
    strip.setPixelColor(i, Wheel(pixelHue));
  }
  rainbowOffset = (rainbowOffset + 1) & 255;
}

// Effect 2: Fire effect with flickering flames
void fireEffect() {
  static byte heat[144];
  
  // Cool down every cell a little
  for(int i = 0; i < NUM_LEDS; i++) {
    int cooling = heat[i] - random(0, ((55 * 10) / NUM_LEDS) + 2);
    heat[i] = (cooling < 0) ? 0 : cooling;
  }
  
  // Heat from each cell drifts up and diffuses
  for(int k = NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }
  
  // Randomly ignite new sparks near the bottom
  if(random(255) < 120) {
    int y = random(7);
    int heating = heat[y] + random(160, 255);
    heat[y] = (heating > 255) ? 255 : heating;
  }
  
  // Convert heat to LED colors
  for(int j = 0; j < NUM_LEDS; j++) {
    byte temperature = heat[j];
    byte t192 = round((temperature / 255.0) * 191);
    
    byte heatramp = t192 & 0x3F; // 0..63
    heatramp <<= 2; // scale up to 0..252
    
    if(t192 > 0x80) {
      // Hottest: white
      strip.setPixelColor(j, strip.Color(255, 255, heatramp));
    } else if(t192 > 0x40) {
      // Medium: yellow to orange
      strip.setPixelColor(j, strip.Color(255, heatramp, 0));
    } else {
      // Coolest: black to red
      strip.setPixelColor(j, strip.Color(heatramp, 0, 0));
    }
  }
}

// Effect 3: Meteor shower effect
void meteorEffect() {
  static int meteorPos[3] = {0, 48, 96};  // 3 meteors
  static int meteorSpeed[3] = {2, 3, 2};
  
  // Fade all LEDs
  for(int i = 0; i < NUM_LEDS; i++) {
    uint32_t color = strip.getPixelColor(i);
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    strip.setPixelColor(i, strip.Color(r * 0.85, g * 0.85, b * 0.85));
  }
  
  // Draw meteors
  for(int m = 0; m < 3; m++) {
    // Draw meteor tail
    for(int i = 0; i < 12; i++) {
      int pos = meteorPos[m] - i;
      if(pos >= 0 && pos < NUM_LEDS) {
        uint8_t brightness = 255 - (i * 20);
        // Cycle through colors for each meteor
        if(m == 0) strip.setPixelColor(pos, strip.Color(brightness, brightness/2, brightness));  // Blue-white
        else if(m == 1) strip.setPixelColor(pos, strip.Color(brightness, brightness/4, brightness/2));  // Purple
        else strip.setPixelColor(pos, strip.Color(brightness/2, brightness, brightness/3));  // Cyan
      }
    }
    
    // Move meteor
    meteorPos[m] += meteorSpeed[m];
    if(meteorPos[m] >= NUM_LEDS + 12) {
      meteorPos[m] = -12;
    }
  }
}

// Effect 4: KITT Knight Rider scanner effect
void kittEffect() {
  static int position = 0;
  static int direction = 4;  // Move 4 LEDs at a time for much faster speed
  
  // Fade all LEDs
  for(int i = 0; i < NUM_LEDS; i++) {
    uint32_t color = strip.getPixelColor(i);
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    strip.setPixelColor(i, strip.Color(r * 0.75, g * 0.75, b * 0.75));
  }
  
  // Draw the scanner with trailing effect
  int scannerWidth = 8;
  for(int i = 0; i < scannerWidth; i++) {
    int pos = position - i;
    if(pos >= 0 && pos < NUM_LEDS) {
      // Create brightness falloff for the trail
      uint8_t brightness = 255 - (i * 30);
      strip.setPixelColor(pos, strip.Color(brightness, 0, 0));
    }
  }
  
  // Move the scanner much faster
  position += direction;
  
  // Bounce at the ends
  if(position >= NUM_LEDS - 1) {
    direction = -4;
    position = NUM_LEDS - 1;
  } else if(position <= 0) {
    direction = 4;
    position = 0;
  }
}

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

  // NeoPixel
  strip.begin();
  strip.clear();
  strip.setBrightness(48);
  strip.show();
  
  // Draw initial UI
  drawButtons();
}

void loop() {
  M5.update();
  
  // Check for touch input
  if(M5.Touch.getCount()) {
    auto touch = M5.Touch.getDetail();
    if(touch.wasPressed()) {
      // Check which button was pressed
      for(int i = 0; i < 4; i++) {
        if(touch.x >= buttons[i].x && touch.x <= buttons[i].x + buttons[i].w &&
           touch.y >= buttons[i].y && touch.y <= buttons[i].y + buttons[i].h) {
          currentEffect = i;
          drawButtons();  // Redraw to show selection
          break;
        }
      }
    }
  }
  
  // Run current effect
  switch(currentEffect) {
    case 0:
      rainbowEffect();
      break;
    case 1:
      fireEffect();
      break;
    case 2:
      meteorEffect();
      break;
    case 3:
      kittEffect();
      break;
  }
  
  strip.show();
  delay(20);
}
