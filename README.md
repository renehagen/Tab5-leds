# Tab5 LED Strip Controller

An interactive LED strip controller for the **M5Stack Tab5** (ESP32-P4) with a beautiful touch interface and multiple spectacular lighting effects.

## Features

- **Touch Interface**: 5-inch 1280×720 touchscreen display in portrait mode
- **5 Lighting Effects**:
  - 🌈 **Rainbow**: Smooth moving rainbow gradient
  - 🔥 **Fire**: Realistic flickering flame effect with heat simulation
  - ☄️ **Meteor**: Colorful meteor shower with glowing trails
  - 🚗 **KITT**: Classic Knight Rider scanner effect
  - ⚫ **OFF**: Turn off all LEDs
- **Brightness Control**: Interactive slider to adjust LED brightness (1-255)
- **144 LEDs**: Optimized for WS2812B/NeoPixel LED strips with 144 LEDs

## Hardware Requirements

- M5Stack Tab5 (ESP32-P4 Function EV Board)
- WS2812B/NeoPixel LED strip (144 LEDs)
- LED strip connected to GPIO 54

## Software Requirements

- PlatformIO
- Libraries (automatically installed):
  - M5Unified
  - M5GFX
  - Adafruit NeoPixel

## Installation

1. Clone this repository:
   ```bash
   git clone <your-repo-url>
   cd Tab5-leds
   ```

2. Open the project in VS Code with PlatformIO extension

3. Build and upload:
   ```bash
   pio run --target upload
   ```

## Usage

1. **Select an Effect**: Tap any of the effect buttons (Rainbow, Fire, Meteor, KITT, or OFF)
2. **Adjust Brightness**: Drag the brightness slider at the bottom to control LED intensity
3. The selected effect button will turn green to show which effect is active

## Configuration

You can modify these settings in `src/main.cpp`:

- `NUM_LEDS`: Change the number of LEDs in your strip (default: 144)
- `LED_PIN`: Change the GPIO pin connected to your LED strip (default: 54)
- Effect speeds and colors can be customized in each effect function

## Pin Configuration

| Component | GPIO Pin |
|-----------|----------|
| LED Strip Data | 54 |

## Platform Configuration

This project uses the **pioarduino** ESP32 platform for ESP32-P4 support:
- Platform: `https://github.com/pioarduino/platform-espressif32.git#54.03.20`
- Board: esp32-p4-evboard
- Display: 5 Inch IPS TFT (1280×720)
- Touch: ST7123 Touch Controller

## License

MIT License - Feel free to use and modify for your projects!

## Contributing

Contributions are welcome! Feel free to submit issues or pull requests.

## Keywords

`ESP32-P4`, `M5Stack Tab5`, `LED Controller`, `WS2812B`, `NeoPixel`, `LED Effects`, `Touch Interface`, `Arduino`, `PlatformIO`
