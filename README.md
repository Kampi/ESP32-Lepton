# ESP32-Lepton

High-performance ESP-IDF driver for [FLIR Lepton 3.x](https://www.flir.de/products/lepton/?vertical=lwir&segment=oem) thermal imaging cameras.

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v5.1+-blue.svg)](https://github.com/espressif/esp-idf)

## Table of Contents

- [ESP32-Lepton](#esp32-lepton)
  - [Table of Contents](#table-of-contents)
  - [Features](#features)
  - [Hardware Support](#hardware-support)
  - [Requirements](#requirements)
  - [Installation](#installation)
    - [Using ESP-IDF Component Manager](#using-esp-idf-component-manager)
    - [Manual Installation](#manual-installation)
  - [Quick Start](#quick-start)
  - [API Reference](#api-reference)
    - [Initialization](#initialization)
      - [`Lepton_Init()`](#lepton_init)
      - [`Lepton_Deinit()`](#lepton_deinit)
    - [Frame Capture](#frame-capture)
      - [`Lepton_CaptureFrameData()`](#lepton_captureframedata)
      - [`Lepton_CaptureFrameRGB()`](#lepton_captureframergb)
      - [`Lepton_GetCaptureTime()`](#lepton_getcapturetime)
    - [CCI Commands](#cci-commands)
      - [Temperature Measurement](#temperature-measurement)
      - [AGC Control](#agc-control)
      - [FFC Control](#ffc-control)
      - [Radiometry](#radiometry)
  - [Configuration](#configuration)
    - [Kconfig Options](#kconfig-options)
    - [Pin Configuration](#pin-configuration)
  - [Color Palettes](#color-palettes)
  - [Performance](#performance)
  - [Troubleshooting](#troubleshooting)
    - [Camera Not Detected](#camera-not-detected)
    - [Invalid Frames / Sync Issues](#invalid-frames--sync-issues)
    - [Slow Frame Rate](#slow-frame-rate)
    - [Memory Issues](#memory-issues)
  - [Examples](#examples)
  - [License](#license)
  - [Maintainer](#maintainer)

## Features

- Full support for **Lepton 3.5** (160x120 resolution)
- **VoSPI** (Video over SPI) high-speed frame capture
- **CCI** (Command & Control Interface) via I2C
- Dual-core optimization (ESP32-S3)
- AGC (Automatic Gain Control) support
- FFC (Flat Field Correction) control
- Radiometry support (temperature measurement)
- Multiple color palettes (Iron, Rainbow, Grayscale)
- Hardware abstraction for easy porting
- Comprehensive error handling
- Example applications included

## Hardware Support

| Camera Model | Resolution | Status |
| -------------- | ----------- | -------- |
| Lepton 3.0 | 160x120 | ⚠️ Not tested |
| Lepton 3.5 | 160x120 | ✅ Tested |
| Lepton 2.x | 80x60 | ⚠️ Not tested |

**Tested ESP32 Platforms:**

- ESP32-S3 (Recommended)
- ESP32 (Should work, not extensively tested)

## Requirements

- **ESP-IDF**: v5.1 or newer
- **Hardware**:
  - ESP32 or ESP32-S3 module
  - FLIR Lepton 3.x camera module
  - SPI interface (3-wire or 4-wire)
  - I2C interface for CCI
  - Adequate power supply (≥150mA @ 3.3V for Lepton)

## Installation

### Using ESP-IDF Component Manager

Add to your `main/idf_component.yml`:

```yaml
dependencies:
  esp32-lepton:
    git: https://github.com/Kampi/ESP32-Lepton.git
```

### Manual Installation

1. Clone into your project's `components` directory:

```bash
cd your_project/components
git clone https://github.com/Kampi/ESP32-Lepton.git
```

1. Include the header in your code:

```c
#include "lepton.h"
```

## Quick Start

```c
#include "lepton.h"

// Define configuration
Lepton_Conf_t config;
config = LEPTON_DEFAULT_CONF;
LEPTON_ASSIGN_FUNC(config, I2CM_Init, I2CM_Deinit, I2CM_Write, I2CM_Read);

// Do all the I2C initialization to get a handle for the Lepton CCI

LEPTON_ASSIGN_I2C_HANDLE(config, I2C_Handle);

// Initialize device
Lepton_t device;
Lepton_Error_t error = Lepton_Init(&device, &config);
if (error != LEPTON_ERR_OK) {
    ESP_LOGE(TAG, "Failed to initialize Lepton: %d", error);
    return;
}

// Allocate frame buffer
uint16_t *frame = (uint16_t*)heap_caps_malloc(
    LEPTON_IMAGE_SIZE_3 * sizeof(uint16_t),
    MALLOC_CAP_DMA
);

// Capture frame
error = Lepton_CaptureFrameData(&device, frame);
if (error == LEPTON_ERR_OK) {
    ESP_LOGI(TAG, "Frame captured successfully!");
    // Process frame data...
}

// Cleanup
free(frame);
Lepton_Deinit(&device);
```

## API Reference

### Initialization

#### `Lepton_Init()`

```c
Lepton_Error_t Lepton_Init(Lepton_t *p_Device, const Lepton_Conf_t *p_Config, Lepton_Result_t *p_Status);
```

Initialize Lepton camera with specified configuration.

**Parameters:**

- `p_Device`: Pointer to device structure
- `p_Config`: Pointer to configuration structure

**Returns:** `LEPTON_ERR_OK` on success, error code otherwise

---

#### `Lepton_Deinit()`

```c
void Lepton_Deinit(Lepton_t *p_Device);
```

Deinitialize Lepton camera and free resources.

### Frame Capture

#### `Lepton_CaptureFrameData()`

```c
Lepton_Error_t Lepton_CaptureFrameData(Lepton_t *p_Device, uint16_t *p_Buffer);
```

Capture raw 14-bit thermal frame data.

**Parameters:**

- `p_Device`: Pointer to initialized device
- `p_Buffer`: Buffer for frame data (must be DMA-capable)

**Returns:** `LEPTON_ERR_OK` on success

---

#### `Lepton_CaptureFrameRGB()`

```c
Lepton_Error_t Lepton_CaptureFrameRGB(Lepton_t *p_Device, uint8_t *p_Buffer, 
                                       const Lepton_RGB_t *p_Palette);
```

Capture frame and convert to RGB888 using specified palette.

**Parameters:**

- `p_Device`: Pointer to initialized device
- `p_Buffer`: Buffer for RGB data
- `p_Palette`: Color palette for conversion

---

#### `Lepton_GetCaptureTime()`

```c
uint32_t Lepton_GetCaptureTime(Lepton_t *p_Device);
```

Get last frame capture duration in milliseconds.

### CCI Commands

#### Temperature Measurement

```c
Lepton_Error_t Lepton_GetTemp(Lepton_t *p_Device, uint16_t *p_FPA, 
                                uint16_t *p_AUX, Lepton_Result_t *p_Status);
```

Read FPA (Focal Plane Array) and AUX (Housing) temperatures in Kelvin × 100.

#### AGC Control

```c
Lepton_Error_t Lepton_SetAGC(Lepton_t *p_Device, Lepton_AGC_Enable_t Mode);
Lepton_Error_t Lepton_GetAGC(Lepton_t *p_Device, Lepton_AGC_Enable_t *p_Mode);
```

#### FFC Control

```c
Lepton_Error_t Lepton_PerformFFC(Lepton_t *p_Device);
Lepton_Error_t Lepton_GetFFCMode(Lepton_t *p_Device, Lepton_FFC_Shutter_t *p_Mode);
```

#### Radiometry

```c
Lepton_Error_t Lepton_GetRadiometryMode(Lepton_t *p_Device, Lepton_Rad_Enable_t *p_Mode);
Lepton_Error_t Lepton_SetRadiometryMode(Lepton_t *p_Device, Lepton_Rad_Enable_t Mode);
```

## Configuration

### Kconfig Options

Configure via `idf.py menuconfig`:

```sh
Component config → ESP32-Lepton Configuration
```

**Available Options:**

- `LEPTON_USE_SEMAPHORE`: Enable thread-safe CCI access
- `LEPTON_CAPTURE_TIMEOUT`: Frame capture timeout (ms)
- `LEPTON_CCI_TIMEOUT`: CCI command timeout (ms)
- `LEPTON_MAX_RETRIES`: Maximum frame resync attempts

### Pin Configuration

Adjust GPIO pins in your `Lepton_Conf_t`:

```c
// Recommended ESP32-S3 pins
config.SPI.MISO = GPIO_NUM_13;
config.SPI.MOSI = GPIO_NUM_11;  // Optional (Lepton doesn't use MOSI)
config.SPI.SCK  = GPIO_NUM_12;
config.SPI.CS   = GPIO_NUM_10;

config.CCI.SDA  = GPIO_NUM_8;
config.CCI.SCL  = GPIO_NUM_9;
```

## Color Palettes

Built-in palettes for thermal visualization:

| Palette | Description | Use Case |
| --------- | ------------- | ---------- |
| **Iron** | Black → Red → Yellow → White | General thermal imaging |
| **Rainbow** | Blue → Green → Yellow → Red | High contrast visualization |
| **Grayscale** | Black → White | Simple temperature mapping |

**Usage:**

```c
#include "lepton_palette.h"

Lepton_CaptureFrameRGB(&device, rgb_buffer, &Lepton_Palette_Iron);
```

## Performance

**Typical Frame Rates:**

- Lepton 3.5: ~8.6 Hz (native)
- ESP32-S3 @ 240 MHz: ~9 Hz capture + RGB conversion

**Memory Requirements:**

- Raw frame buffer: 38.4 KB (160×120×2 bytes)
- RGB888 frame: 57.6 KB (160×120×3 bytes)
- Driver overhead: ~4 KB

**Optimization Tips:**

1. Use DMA-capable memory (`MALLOC_CAP_DMA`)
2. Run capture on dedicated core
3. Enable compiler optimizations (`-O2` or `-O3`)
4. Use hardware SPI controller (not bit-banging)

## Troubleshooting

### Camera Not Detected

**Symptoms:** CCI timeout, Status = 0x0000

**Solutions:**

1. Verify I2C wiring (SDA, SCL, GND)
2. Check pull-up resistors (2.2kΩ - 10kΩ)
3. Ensure camera has stable 3.3V power
4. Add 1-2 second boot delay after power-on

```c
// Give Lepton time to boot
vTaskDelay(pdMS_TO_TICKS(2000));
Lepton_Init(&device, &config);
```

### Invalid Frames / Sync Issues

**Symptoms:** Corrupted images, vertical lines

**Solutions:**

1. Reduce SPI clock (try 10 MHz instead of 20 MHz)
2. Check SPI wiring quality (short traces, proper grounding)
3. Increase `LEPTON_MAX_RETRIES` in Kconfig
4. Ensure CS line has proper pull-up

### Slow Frame Rate

**Solutions:**

1. Use DMA transfers for SPI
2. Optimize RGB conversion (use lookup tables)
3. Run on CPU core with higher priority
4. Disable watchdog during capture if needed

### Memory Issues

**Symptoms:** Allocation failures, heap corruption

**Solutions:**

1. Use `heap_caps_malloc()` with `MALLOC_CAP_DMA`
2. Check available heap: `esp_get_free_heap_size()`
3. Reduce frame buffer count
4. Enable PSRAM if available

## Examples

See the [`examples/`](examples/) directory:

- **basic_init**: Basic initialization of the Lepton camera

## License

This project is licensed under the **GNU General Public License v3.0**.

See [LICENSE](LICENSE) for full text.

## Maintainer

**Daniel Kampert**  
📧 [DanielKampert@kampis-elektroecke.de](mailto:DanielKampert@kampis-elektroecke.de)  
🌐 [www.kampis-elektroecke.de](https://www.kampis-elektroecke.de)

---

**Contributions Welcome!** Please open issues or pull requests on GitHub.
