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
uint8_t currentEffect = 5;  // 0 = Rainbow, 1 = Fire, 2 = Meteor, 3 = KITT, 4 = OFF, 5 = Solid Color
uint16_t rainbowOffset = 0;
uint16_t animationCounter = 0;
uint8_t ledBrightness = 128;  // Current LED brightness (0-255), 50% = 128
uint32_t solidColor = 0;  // Stored solid color for effect 5

// Color definitions for the modern dark theme
#define BG_COLOR 0x20E4  // Dark gray background (RGB: 33, 37, 41)
#define CARD_COLOR 0x39C7  // Slightly lighter card background
#define TEXT_WHITE 0xFFFF
#define SLIDER_BG 0x4228  // Dark slider background
#define SLIDER_FILL 0x0D7F  // Blue slider fill
#define OFF_BUTTON_COLOR 0x0D7F  // Blue for off button

// Button definitions for 2x2 grid layout
struct Button {
  int x, y, w, h;
  const char* label;
  bool selected;
};

// Effect buttons in 2x2 grid
Button effectButtons[4] = {
  {60, 190, 280, 280, "Rainbow", false},   // Top-left
  {380, 190, 280, 280, "Fire", false},     // Top-right
  {60, 490, 280, 280, "Meteor", false},    // Bottom-left
  {380, 490, 280, 280, "KITT", false}      // Bottom-right
};

// Off button at bottom
Button offButton = {60, 1070, 600, 100, "Off", false};

// Forward declarations
void drawInterface();
void drawHeader();
void drawEffectButtons();
void drawBrightnessSlider();
void drawColorCircles();
void drawOffButton();
void drawGradientBackground(int x, int y, int w, int h, int effectType);
void updateButtonSelection(int oldSelection, int newSelection);
void solidColorEffect();

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

// Draw gradient backgrounds for effect buttons
void drawGradientBackground(int x, int y, int w, int h, int effectType) {
  auto& lcd = M5.Display;
  
  // First draw the rounded rectangle background
  lcd.fillRoundRect(x, y, w, h, 15, CARD_COLOR);
  
  switch(effectType) {
    case 0: { // Rainbow gradient - optimized with bands instead of pixels
      // Draw rainbow bands inside the button area with padding
      int innerX = x + 5;
      int innerY = y + 5;
      int innerW = w - 10;
      int innerH = h - 10;
      
      int bands = 8;
      for(int i = 0; i < bands; i++) {
        float hue = (float)i / bands;
        uint8_t r = (sin(hue * 6.28318 + 0) * 127 + 128);
        uint8_t g = (sin(hue * 6.28318 + 2.094395) * 127 + 128);
        uint8_t b = (sin(hue * 6.28318 + 4.18879) * 127 + 128);
        uint16_t color = lcd.color565(r, g, b);
        
        // Draw vertical bands for simplicity
        int bandX = innerX + (i * innerW / bands);
        int bandWidth = innerW / bands + 1;
        lcd.fillRect(bandX, innerY, bandWidth, innerH - 30, color);
      }
      break;
    }
      
    case 1: { // Fire gradient (orange to red)
      // First ensure we have a dark rounded background
      lcd.fillRoundRect(x, y, w, h, 15, lcd.color565(100, 20, 0));
      // Draw gradient inside the button with padding
      for(int i = 0; i < h - 35; i++) {
        float gradient = (float)i / (h - 35);
        uint8_t r = 255;
        uint8_t g = 140 - (gradient * 100);
        uint8_t b = 0;
        uint16_t color = lcd.color565(r, g, b);
        lcd.fillRect(x + 5, y + 5 + i, w - 10, 1, color);
      }
      break;
    }
      
    case 2: // Meteor (dark with streaks)
      // Draw diagonal streaks inside the button
      lcd.fillRoundRect(x, y, w, h, 15, 0x2104);  // Very dark background
      for(int i = 0; i < 4; i++) {
        int sx = x + 20 + i * 60;
        int sy = y + 10;
        lcd.drawLine(sx, sy, sx + 80, sy + h - 40, 0xC618);
        lcd.drawLine(sx + 1, sy, sx + 81, sy + h - 40, 0x8410);
      }
      break;
      
    case 3: { // KITT (black with red scanner)
      lcd.fillRoundRect(x, y, w, h, 15, TFT_BLACK);
      // Draw red scanner bar
      int barY = y + h - 60;
      lcd.fillRect(x + 20, barY, w - 40, 8, TFT_RED);
      // Add glow effect
      lcd.fillRect(x + 20, barY - 4, w - 40, 4, lcd.color565(128, 0, 0));
      lcd.fillRect(x + 20, barY + 8, w - 40, 4, lcd.color565(128, 0, 0));
      break;
    }
  }
}

// Draw the main interface
void drawInterface() {
  auto& lcd = M5.Display;
  lcd.fillScreen(BG_COLOR);
  
  drawHeader();
  drawEffectButtons();
  drawColorCircles();
  drawBrightnessSlider();
  drawOffButton();
}

// Draw the header with title and icons
void drawHeader() {
  auto& lcd = M5.Display;
  
  // Draw WiFi icon (simplified)
  lcd.setTextColor(0x07E0);  // Green for WiFi
  lcd.fillCircle(60, 70, 4, 0x07E0);
  lcd.drawCircle(60, 70, 12, 0x07E0);
  lcd.drawCircle(60, 70, 20, 0x07E0);
  lcd.drawCircle(60, 70, 28, 0x07E0);
  
  // Title
  lcd.setTextDatum(textdatum_t::middle_center);
  lcd.setTextColor(TEXT_WHITE);
  lcd.setTextSize(4);
  lcd.drawString("LED Control", lcd.width()/2, 70);
  
  // Settings icon (gear - simplified)
  lcd.fillCircle(660, 70, 20, 0x7BEF);  // Gray gear
  lcd.fillCircle(660, 70, 12, BG_COLOR);  // Inner hole
  // Gear teeth
  for(int i = 0; i < 8; i++) {
    float angle = i * 0.785398;  // 45 degrees in radians
    int x1 = 660 + cos(angle) * 18;
    int y1 = 70 + sin(angle) * 18;
    int x2 = 660 + cos(angle) * 24;
    int y2 = 70 + sin(angle) * 24;
    lcd.drawLine(x1, y1, x2, y2, 0x7BEF);
    lcd.drawLine(x1+1, y1, x2+1, y2, 0x7BEF);
    lcd.drawLine(x1, y1+1, x2, y2+1, 0x7BEF);
  }
  
  // "Effects" label
  lcd.setTextDatum(textdatum_t::top_left);
  lcd.setTextSize(3);
  lcd.setTextColor(TEXT_WHITE);
  lcd.drawString("Effects", 60, 140);
}

// Draw the 2x2 grid of effect buttons
void drawEffectButtons() {
  auto& lcd = M5.Display;
  
  for(int i = 0; i < 4; i++) {
    // Draw gradient background
    drawGradientBackground(effectButtons[i].x, effectButtons[i].y, 
                          effectButtons[i].w, effectButtons[i].h, i);
    
    // Draw selection border if selected
    if(i == currentEffect) {
      lcd.drawRoundRect(effectButtons[i].x - 2, effectButtons[i].y - 2, 
                       effectButtons[i].w + 4, effectButtons[i].h + 4, 15, TEXT_WHITE);
      lcd.drawRoundRect(effectButtons[i].x - 3, effectButtons[i].y - 3, 
                       effectButtons[i].w + 6, effectButtons[i].h + 6, 15, TEXT_WHITE);
    }
    
    // Draw label at bottom of each button
    lcd.setTextDatum(textdatum_t::bottom_center);
    lcd.setTextSize(3);
    lcd.setTextColor(TEXT_WHITE);
    lcd.drawString(effectButtons[i].label, 
                  effectButtons[i].x + effectButtons[i].w/2, 
                  effectButtons[i].y + effectButtons[i].h - 10);
  }
}

// Draw color selection circles above brightness slider
void drawColorCircles() {
  auto& lcd = M5.Display;
  
  // Calculate the 8 rainbow colors (same as used in rainbow button)
  uint32_t rainbowColors[8];
  int circleY = 850;  // Y position above brightness slider (25px spacing above and below)
  int startX = 80;
  int spacing = 75;
  int radius = 25;
  
  // Generate the same 8 colors used in the rainbow effect
  for(int i = 0; i < 8; i++) {
    float hue = (float)i / 8;
    uint8_t r = (sin(hue * 6.28318 + 0) * 127 + 128);
    uint8_t g = (sin(hue * 6.28318 + 2.094395) * 127 + 128);
    uint8_t b = (sin(hue * 6.28318 + 4.18879) * 127 + 128);
    
    // Store as RGB565 color for display
    uint16_t color565 = lcd.color565(r, g, b);
    
    // Store as NeoPixel color (RGB format)
    rainbowColors[i] = strip.Color(r, g, b);
    
    // Draw the circle
    int circleX = startX + (i * spacing);
    lcd.fillCircle(circleX, circleY, radius, color565);
    
    // Add white border for better visibility
    lcd.drawCircle(circleX, circleY, radius, TEXT_WHITE);
  }
}

// Draw the brightness slider
void drawBrightnessSlider() {
  auto& lcd = M5.Display;
  
  // Static variable to track last handle position
  static int lastHandleX = -1;
  
  // "Brightness" label (only draw if not already there)
  if(lastHandleX == -1) {
    lcd.setTextDatum(textdatum_t::top_left);
    lcd.setTextSize(3);
    lcd.setTextColor(TEXT_WHITE);
    lcd.drawString("Brightness", 60, 920);
  }
  
  // Percentage value - clear old value first
  lcd.fillRect(580, 920, 80, 30, BG_COLOR);
  lcd.setTextDatum(textdatum_t::top_right);
  lcd.setTextSize(3);
  lcd.setTextColor(TEXT_WHITE);
  int percentage = map(ledBrightness, 0, 255, 0, 100);
  char percStr[10];
  sprintf(percStr, "%d%%", percentage);
  lcd.drawString(percStr, 660, 920);
  
  // Slider track position
  int sliderY = 970;
  int sliderX = 60;
  int sliderW = 600;
  int sliderH = 8;
  
  // Calculate new handle position
  int fillWidth = map(ledBrightness, 0, 255, 0, sliderW);
  int newHandleX = sliderX + fillWidth;
  
  // Only redraw if handle moved
  if(lastHandleX != newHandleX) {
    // Clear the entire slider area including handle space (bigger clear area)
    lcd.fillRect(sliderX - 25, sliderY - 25, sliderW + 50, 50, BG_COLOR);
    
    // Redraw slider track
    lcd.fillRoundRect(sliderX, sliderY, sliderW, sliderH, 4, SLIDER_BG);
    
    // Draw slider fill
    if(fillWidth > 0) {
      lcd.fillRoundRect(sliderX, sliderY, fillWidth, sliderH, 4, SLIDER_FILL);
    }
    
    // Draw new handle
    lcd.fillCircle(newHandleX, sliderY + sliderH/2, 18, SLIDER_FILL);
    lcd.drawCircle(newHandleX, sliderY + sliderH/2, 18, TEXT_WHITE);
    
    lastHandleX = newHandleX;
  }
}

// Draw the Off button
void drawOffButton() {
  auto& lcd = M5.Display;
  
  // Draw button background
  if(currentEffect == 4) {
    lcd.fillRoundRect(offButton.x, offButton.y, offButton.w, offButton.h, 20, 0x4228);
    lcd.drawRoundRect(offButton.x, offButton.y, offButton.w, offButton.h, 20, TEXT_WHITE);
  } else {
    lcd.fillRoundRect(offButton.x, offButton.y, offButton.w, offButton.h, 20, OFF_BUTTON_COLOR);
  }
  
  // Draw button text
  lcd.setTextDatum(textdatum_t::middle_center);
  lcd.setTextSize(4);
  lcd.setTextColor(TEXT_WHITE);
  lcd.drawString("Off", offButton.x + offButton.w/2, offButton.y + offButton.h/2);
}

// Update only the selection borders without redrawing everything
void updateButtonSelection(int oldSelection, int newSelection) {
  auto& lcd = M5.Display;
  
  // Clear old selection border for effect buttons (0-3)
  if(oldSelection >= 0 && oldSelection < 4) {
    // Draw over the old border with background color
    lcd.drawRoundRect(effectButtons[oldSelection].x - 2, effectButtons[oldSelection].y - 2, 
                     effectButtons[oldSelection].w + 4, effectButtons[oldSelection].h + 4, 15, BG_COLOR);
    lcd.drawRoundRect(effectButtons[oldSelection].x - 3, effectButtons[oldSelection].y - 3, 
                     effectButtons[oldSelection].w + 6, effectButtons[oldSelection].h + 6, 15, BG_COLOR);
  }
  
  // Draw new selection border for effect buttons (0-3)
  if(newSelection >= 0 && newSelection < 4) {
    lcd.drawRoundRect(effectButtons[newSelection].x - 2, effectButtons[newSelection].y - 2, 
                     effectButtons[newSelection].w + 4, effectButtons[newSelection].h + 4, 15, TEXT_WHITE);
    lcd.drawRoundRect(effectButtons[newSelection].x - 3, effectButtons[newSelection].y - 3, 
                     effectButtons[newSelection].w + 6, effectButtons[newSelection].h + 6, 15, TEXT_WHITE);
  }
  
  // Update Off button appearance if needed
  if((oldSelection == 4 && newSelection != 4) || (oldSelection != 4 && newSelection == 4)) {
    drawOffButton();
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

// Solid color effect - fill entire strip with selected color
void solidColorEffect() {
  for(int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, solidColor);
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
  lcd.fillScreen(BG_COLOR);

  // NeoPixel
  strip.begin();
  strip.clear();
  strip.setBrightness(ledBrightness);
  strip.show();
  
  // Draw initial UI
  drawInterface();
}

void loop() {
  M5.update();
  
  // Check for touch input
  if(M5.Touch.getCount()) {
    auto touch = M5.Touch.getDetail();
    
    // Check for button presses
    if(touch.wasPressed()) {
      // Check for color circle taps
      int circleY = 820;
      int startX = 80;
      int spacing = 75;
      int radius = 25;
      int hitRadius = 40;  // Larger hit radius for easier tapping
      
      for(int i = 0; i < 8; i++) {
        int circleX = startX + (i * spacing);
        int dx = touch.x - circleX;
        int dy = touch.y - circleY;
        int distance = sqrt(dx*dx + dy*dy);
        
        if(distance <= hitRadius) {
          // Calculate the color for this circle (same as in drawColorCircles)
          float hue = (float)i / 8;
          uint8_t r = (sin(hue * 6.28318 + 0) * 127 + 128);
          uint8_t g = (sin(hue * 6.28318 + 2.094395) * 127 + 128);
          uint8_t b = (sin(hue * 6.28318 + 4.18879) * 127 + 128);
          
          // Set solid color and switch to solid color effect
          solidColor = strip.Color(r, g, b);
          int oldEffect = currentEffect;
          currentEffect = 5;  // Solid color mode
          
          // Fill strip immediately
          for(int led = 0; led < NUM_LEDS; led++) {
            strip.setPixelColor(led, solidColor);
          }
          strip.show();
          
          updateButtonSelection(oldEffect, currentEffect);
          break;
        }
      }
      
      // Check which effect button was pressed
      for(int i = 0; i < 4; i++) {
        if(touch.x >= effectButtons[i].x && touch.x <= effectButtons[i].x + effectButtons[i].w &&
           touch.y >= effectButtons[i].y && touch.y <= effectButtons[i].y + effectButtons[i].h) {
          if(currentEffect != i) {  // Only update if selection changed
            int oldEffect = currentEffect;
            currentEffect = i;
            updateButtonSelection(oldEffect, currentEffect);  // Just update borders
          }
          break;
        }
      }
      
      // Check if Off button was pressed
      if(touch.x >= offButton.x && touch.x <= offButton.x + offButton.w &&
         touch.y >= offButton.y && touch.y <= offButton.y + offButton.h) {
        if(currentEffect != 4) {  // Only update if not already OFF
          int oldEffect = currentEffect;
          currentEffect = 4;  // Set to OFF mode
          strip.clear();
          strip.show();
          updateButtonSelection(oldEffect, currentEffect);  // Just update selection
        }
      }
    }
    
    // Check for slider interaction (both press and drag)
    if(touch.isPressed() || touch.wasPressed()) {
      int sliderX = 60;
      int sliderY = 920;
      int sliderW = 600;
      int sliderH = 60;  // Increased hit area for easier interaction (bigger touch radius)
      
      if(touch.y >= sliderY - 30 && touch.y <= sliderY + sliderH &&
         touch.x >= sliderX && touch.x <= sliderX + sliderW) {
        // Calculate new brightness based on touch position
        int newBrightness = map(touch.x, sliderX, sliderX + sliderW, 0, 255);
        newBrightness = constrain(newBrightness, 0, 255);
        
        if(newBrightness != ledBrightness) {
          ledBrightness = newBrightness;
          strip.setBrightness(ledBrightness);
          drawBrightnessSlider();  // Update slider display
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
    case 4:
      // OFF - do nothing, LEDs stay cleared
      break;
    case 5:
      solidColorEffect();
      break;
  }
  
  strip.show();
  delay(20);
}
