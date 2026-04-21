# ESP32-Lepton

High-performance ESP-IDF driver for [FLIR Lepton 3.5](https://oem.flir.com/de-de/products/lepton/?vertical=microcam&segment=oem) thermal imaging cameras.

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg?logo=opensourceinitiative)](https://www.gnu.org/licenses/gpl-3.0)
[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v5.1+-blue.svg)](https://github.com/espressif/esp-idf)
[![Documentation](https://img.shields.io/badge/User%20Guide-PDF-007ec6?longCache=true&style=flat&logo=asciidoctor&colorA=555555)](https://kampi.github.io/ESP32-Lepton/)
[![ESP Registry](https://components.espressif.com/components/kampi/esp32-lepton/badge.svg)](https://components.espressif.com/components/kampi/esp32-lepton/)

## Table of Contents

- [ESP32-Lepton](#esp32-lepton)
  - [Table of Contents](#table-of-contents)
  - [Features](#features)
  - [Hardware Support](#hardware-support)
  - [Requirements](#requirements)
  - [Installation](#installation)
    - [Using ESP-IDF Component Manager](#using-esp-idf-component-manager)
    - [Manual Installation](#manual-installation)
  - [Supported commands](#supported-commands)
  - [Documentation](#documentation)
    - [Requirements](#requirements-1)
    - [Structure](#structure)
    - [Format](#format)
    - [Style Guide](#style-guide)
    - [Building Locally](#building-locally)
      - [Build HTML Documentation](#build-html-documentation)
      - [Build PDF Documentation](#build-pdf-documentation)
      - [Build All Documentation](#build-all-documentation)
    - [Automated CI/CD](#automated-cicd)
    - [Accessing Built Documentation](#accessing-built-documentation)
    - [Module Overview](#module-overview)
      - [lepton.adoc](#leptonadoc)
      - [lepton\_capture.adoc](#lepton_captureadoc)
      - [lepton\_cci.adoc](#lepton_cciadoc)
      - [cci.adoc](#cciadoc)
      - [vospi.adoc](#vospiadoc)
  - [Kconfig Options](#kconfig-options)
  - [Color Palettes](#color-palettes)
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
- Multiple color palettes (Iron, Rainbow, Whitehot, Blackhot, Amber, Arctic, Lava)

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

## Supported commands

See [CCI API Status](CCI_API_Status.md).

## Documentation

### Requirements

```bash
# Ubuntu/Debian
sudo apt-get install asciidoctor ruby-asciidoctor-pdf

# macOS
brew install asciidoctor

# Or via Ruby gems
gem install asciidoctor asciidoctor-pdf
```

### Structure

The documentation is organized into separate modules:

- **index.adoc** - Main overview and getting started guide
- **lepton.adoc** - Main driver API documentation
- **lepton_capture.adoc** - Frame capture task implementation
- **lepton_cci.adoc** - High-level CCI commands
- **cci.adoc** - Low-level CCI protocol implementation
- **vospi.adoc** - VoSPI (Video over SPI) interface

### Format

The documentation uses **AsciiDoc** format, which provides:

- Rich formatting capabilities
- Code syntax highlighting
- Cross-references between documents
- Professional PDF output
- Easy-to-read source format

### Style Guide

- Use clear, concise language
- Include code examples for all API functions
- Add usage examples for complex features
- Document error conditions and return values
- Cross-reference related modules using `link:module.html[Module Name]`

### Building Locally

#### Build HTML Documentation

```bash
cd docs
asciidoctor index.adoc -o index.html
asciidoctor lepton.adoc -o lepton.html
asciidoctor lepton_capture.adoc -o lepton_capture.html
asciidoctor lepton_cci.adoc -o lepton_cci.html
asciidoctor cci.adoc -o cci.html
asciidoctor vospi.adoc -o vospi.html
```

#### Build PDF Documentation

```bash
cd docs
asciidoctor-pdf index.adoc -o index.pdf
asciidoctor-pdf lepton.adoc -o lepton.pdf
asciidoctor-pdf lepton_capture.adoc -o lepton_capture.pdf
asciidoctor-pdf lepton_cci.adoc -o lepton_cci.pdf
asciidoctor-pdf cci.adoc -o cci.pdf
asciidoctor-pdf vospi.adoc -o vospi.pdf
```

#### Build All Documentation

```bash
cd docs
for adoc in *.adoc; do
  asciidoctor "$adoc" -o "${adoc%.adoc}.html"
  asciidoctor-pdf "$adoc" -o "${adoc%.adoc}.pdf"
done
```

### Automated CI/CD

The documentation is automatically built and deployed via GitHub Actions:

- **Trigger**: Push to `main`/`master` branch or changes in `docs/` directory
- **Output**: HTML and PDF versions
- **Artifacts**: Uploaded as GitHub Actions artifacts
- **Releases**: Nightly documentation releases with tarball and individual PDFs

### Accessing Built Documentation

- **Artifacts**: Download from GitHub Actions run
- **Releases**: Check the "nightly" pre-release for latest documentation
- **Tarball**: `ESP32-Lepton-Docs-nightly.tar.gz` contains all HTML files
- **Individual PDFs**: Available in releases (e.g., `lepton-nightly.pdf`)

### Module Overview

#### lepton.adoc

Core driver interface with initialization, configuration, and high-level control functions. This is the main entry point for users of the component.

#### lepton_capture.adoc

Describes the FreeRTOS task that continuously captures thermal frames from the sensor. Includes VSync interrupt handling and frame buffer management.

#### lepton_cci.adoc

High-level CCI commands for sensor configuration (AGC, emissivity, ROI, telemetry, etc.). Wraps low-level CCI functions with device state management.

#### cci.adoc

Low-level I2C protocol implementation for the Lepton CCI interface. Handles command execution, register access, and status polling.

#### vospi.adoc

SPI-based video interface for high-speed thermal data acquisition. Implements packet synchronization, frame assembly, and DMA transfers.

## Kconfig Options

Configure via `idf.py menuconfig`:

```sh
Component config → ESP32-Lepton
```

## Color Palettes

Built-in palettes for thermal visualization:

| Palette | Description | Use Case |
| --------- | ------------- | ---------- |
| **Iron** | Black → Red → Yellow → White | General thermal imaging |
| **Rainbow** | Full spectrum blue → cyan → green → yellow → red | High contrast visualization |
| **Whitehot** | Black → Gray → White (cold to hot) | Standard grayscale imaging |
| **Blackhot** | White → Gray → Black (cold to hot) | Inverted grayscale imaging |
| **Amber** | Black → Brown → Orange → Yellow → White | Night vision / security |
| **Arctic** | Dark blue → Cyan → White | Cold-scene visualization |
| **Lava** | Black → Purple → Red → Orange → Yellow | Volcanic / high-temp imaging |

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
