# ESP32-Lepton Component Documentation

This directory contains the AsciiDoc documentation for the ESP32-Lepton component.

## Documentation Structure

The documentation is organized into separate modules:

- **index.adoc** - Main overview and getting started guide
- **lepton.adoc** - Main driver API documentation
- **lepton_capture.adoc** - Frame capture task implementation
- **lepton_cci.adoc** - High-level CCI commands
- **cci.adoc** - Low-level CCI protocol implementation
- **vospi.adoc** - VoSPI (Video over SPI) interface

## Building Documentation Locally

### Prerequisites

Install AsciiDoctor:

```bash
# Ubuntu/Debian
sudo apt-get install asciidoctor ruby-asciidoctor-pdf

# macOS
brew install asciidoctor

# Or via Ruby gems
gem install asciidoctor asciidoctor-pdf
```

### Build HTML Documentation

```bash
cd docs
asciidoctor index.adoc -o index.html
asciidoctor lepton.adoc -o lepton.html
asciidoctor lepton_capture.adoc -o lepton_capture.html
asciidoctor lepton_cci.adoc -o lepton_cci.html
asciidoctor cci.adoc -o cci.html
asciidoctor vospi.adoc -o vospi.html
```

### Build PDF Documentation

```bash
cd docs
asciidoctor-pdf index.adoc -o index.pdf
asciidoctor-pdf lepton.adoc -o lepton.pdf
asciidoctor-pdf lepton_capture.adoc -o lepton_capture.pdf
asciidoctor-pdf lepton_cci.adoc -o lepton_cci.pdf
asciidoctor-pdf cci.adoc -o cci.pdf
asciidoctor-pdf vospi.adoc -o vospi.pdf
```

### Build All Documentation

```bash
cd docs
for adoc in *.adoc; do
  asciidoctor "$adoc" -o "${adoc%.adoc}.html"
  asciidoctor-pdf "$adoc" -o "${adoc%.adoc}.pdf"
done
```

## Automated CI/CD

The documentation is automatically built and deployed via GitHub Actions:

- **Trigger**: Push to `main`/`master` branch or changes in `docs/` directory
- **Output**: HTML and PDF versions
- **Artifacts**: Uploaded as GitHub Actions artifacts
- **Releases**: Nightly documentation releases with tarball and individual PDFs

### Workflow File

See `.github/workflows/documentation.yml` for the complete CI/CD pipeline.

### Accessing Built Documentation

- **Artifacts**: Download from GitHub Actions run
- **Releases**: Check the "nightly" pre-release for latest documentation
- **Tarball**: `ESP32-Lepton-Docs-nightly.tar.gz` contains all HTML files
- **Individual PDFs**: Available in releases (e.g., `lepton-nightly.pdf`)

## Documentation Format

The documentation uses **AsciiDoc** format, which provides:

- Rich formatting capabilities
- Code syntax highlighting
- Cross-references between documents
- Professional PDF output
- Easy-to-read source format

### AsciiDoc Resources

- [AsciiDoc Quick Reference](https://docs.asciidoctor.org/asciidoc/latest/syntax-quick-reference/)
- [AsciiDoctor Documentation](https://docs.asciidoctor.org/)

## Contributing

When adding or modifying documentation:

1. Edit the relevant `.adoc` file
2. Build locally to verify formatting
3. Commit changes
4. Push to trigger automated build

### Documentation Style Guide

- Use clear, concise language
- Include code examples for all API functions
- Add usage examples for complex features
- Document error conditions and return values
- Cross-reference related modules using `link:module.html[Module Name]`

## Module Overview

### lepton.adoc

Core driver interface with initialization, configuration, and high-level control functions. This is the main entry point for users of the component.

### lepton_capture.adoc

Describes the FreeRTOS task that continuously captures thermal frames from the sensor. Includes VSync interrupt handling and frame buffer management.

### lepton_cci.adoc

High-level CCI commands for sensor configuration (AGC, emissivity, ROI, telemetry, etc.). Wraps low-level CCI functions with device state management.

### cci.adoc

Low-level I2C protocol implementation for the Lepton CCI interface. Handles command execution, register access, and status polling.

### vospi.adoc

SPI-based video interface for high-speed thermal data acquisition. Implements packet synchronization, frame assembly, and DMA transfers.

## License

This project is licensed under the **GNU General Public License v3.0**.

See [LICENSE](../LICENSE) for full text.

## Maintainer

**Daniel Kampert**  
📧 [DanielKampert@kampis-elektroecke.de](mailto:DanielKampert@kampis-elektroecke.de)  
🌐 [www.kampis-elektroecke.de](https://www.kampis-elektroecke.de)

---

**Contributions Welcome!** Please open issues or pull requests on GitHub.
