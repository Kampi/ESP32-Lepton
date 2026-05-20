# CHANGELOG

## [Unreleased]

## [0.0.3] - 2026-04-21

**Changed:**

- Change min. esp-idf version to 5.5.3 to make use of the direct PSRAM / SPI DMA transaction

**Added:**

- Add support for esp-idf 6
- Add `MALLOC_CAP_SIMD` for image buffers
- Add dynamic memory caps for VoSPI transaction to allow the usage without PSRAM
- Add SIMD support
- Add additional color paletts
- Add API to trigger camera FFC

**Fixed:**

- Fix memory leaks during driver deinitialization

**Removed:**

- Remove ESP32 from supported devices list because it is untested

## [0.0.2] - 2026-01-26

**Changed:**

- Set component version in CMakeLists.txt and idf_component.txt from CI/CD (#7)
- Set component version in Library via CMakeLists (#6)

## [0.0.1] - 28.12.2025

**Added:**

Initial release

[Unreleased]: https://github.com/Kampi/ESP32-Lepton/compare/0.0.3...HEAD
[0.0.2]: https://github.com/Kampi/ESP32-Lepton/compare/0.0.1...0.0.2
[0.0.3]: https://github.com/Kampi/ESP32-Lepton/compare/0.0.2...0.0.3
