# CHANGELOG

## [Unreleased]

**Changed:**

- Change min. esp-idf version to 5.5.3 to make use of the direct PSRAM / SPI DMA transaction

**Added:**

- Add `MALLOC_CAP_SIMD` for image buffers
- Add dynamic memory caps for VoSPI transaction to allow the usage without PSRAM
- Add SIMD support

**Fixed:**

- Fix memory leaks during driver deinitialization

**Removed:**

- Remove ESP from supported devices list because it is untested

## [0.0.2] - 2026-01-26

**Changed:**

- Set component version in CMakeLists.txt and idf_component.txt from CI/CD (#7)
- Set component version in Library via CMakeLists (#6)

## [0.0.1] - 28.12.2025

**Added:**

Initial release

[Unreleased]: https://github.com/Kampi/ESP32-Lepton/compare/0.0.2...HEAD
[0.0.2]: https://github.com/Kampi/ESP32-Lepton/compare/0.0.1...0.0.2
