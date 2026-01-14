# ESP32-Lepton

High-performance ESP-IDF driver for [FLIR Lepton 3.5](https://oem.flir.com/de-de/products/lepton/?vertical=microcam&segment=oem) thermal imaging cameras.

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg?logo=opensourceinitiative)](https://www.gnu.org/licenses/gpl-3.0)
[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v5.1+-blue.svg)](https://github.com/espressif/esp-idf)
[![Documentation](https://img.shields.io/badge/User%20Guide-PDF-007ec6?longCache=true&style=flat&logo=asciidoctor&colorA=555555)](https://kampi.github.io/ESP32-Lepton/)

## Table of Contents

- [ESP32-Lepton](#esp32-lepton)
  - [Table of Contents](#table-of-contents)
  - [Features](#features)
  - [Hardware Support](#hardware-support)
  - [Requirements](#requirements)
  - [Installation](#installation)
    - [Using ESP-IDF Component Manager](#using-esp-idf-component-manager)
    - [Manual Installation](#manual-installation)
  - [Kconfig Options](#kconfig-options)
  - [Color Palettes (TBD)](#color-palettes-tbd)
  - [Performance](#performance)
  - [Examples](#examples)
  - [License](#license)
  - [Maintainer](#maintainer)

## Features

- Full support for **Lepton 3.5** (160x120 resolution)
- VoSPI (Video over SPI) high-speed frame capture
- CCI (Command & Control Interface) via I2C
- AGC (Automatic Gain Control) support
- FFC (Flat Field Correction) control
- Radiometry support (temperature measurement)
- Multiple color palettes (Iron, Rainbow, Grayscale)

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

- Clone into your project's `components` directory:

```bash
cd your_project/components
git clone https://github.com/Kampi/ESP32-Lepton.git
```

## Kconfig Options

Configure via `idf.py menuconfig`:

```sh
Component config → ESP32-Lepton
```

## Color Palettes (TBD)

Built-in palettes for thermal visualization:

| Palette | Description | Use Case |
| --------- | ------------- | ---------- |
| **Iron** | Black → Red → Yellow → White | General thermal imaging |

## Performance

**Typical Frame Rates:**

- Lepton 3.5: ~8.6 Hz (native)
- ESP32-S3 @ 240 MHz: ~9 Hz capture + RGB conversion

## Examples

See the [`examples/`](examples/) directory:

- **basic_init**: Basic initialization of the Lepton camera
- **basic_capture**: Basic capture example

## License

This project is licensed under the **GNU General Public License v3.0**.

See [LICENSE](LICENSE) for full text.

## Maintainer

**Daniel Kampert**  
📧 [DanielKampert@kampis-elektroecke.de](mailto:DanielKampert@kampis-elektroecke.de)  
🌐 [www.kampis-elektroecke.de](https://www.kampis-elektroecke.de)

---

**Contributions Welcome!** Please open issues or pull requests on GitHub.
