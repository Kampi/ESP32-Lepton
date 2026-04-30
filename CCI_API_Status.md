# Lepton CCI API — Implementation Status

Reference: FLIR Lepton Software Interface Description Document (Software IDD), Revision 200.

The document maps every CCI command from the IDD against the ESP32-Lepton driver
implementation.  Two layers are tracked separately:

- **CCI layer** — low-level functions in `src/CCI/cci.cpp` / `include/CCI/cci.h`
- **Lepton layer** — high-level public API in `src/lepton_cci.cpp` / `include/lepton.h`

Status legend:

| Symbol | Meaning |
| --- | --- |
| ✅ | Fully implemented |
| ⚙️ | Command register defined, no public API function |
| ❌ | Not implemented at all |

---

## Module Overview

| Module | IDD Chapter | Base Address | Total Commands | CCI Implemented | CCI Pending |
| --- | --- | --- | --- | --- | --- |
| AGC | 4.4 | `0x0100` | ~20 | 7 | ~13 |
| SYS | 4.5 | `0x0200` | ~20 | 14 | ~6 |
| VID | 4.6 | `0x0300` | ~14 | 6 | ~8 |
| OEM | 4.7 | `0x4800` | ~28 | 7 | ~21 |
| RAD | 4.8 | `0x4E00` | ~36 | 9 | ~27 |

---

## 4.4 AGC Module — Automatic Gain Control

### 4.4.2 AGC Enable State

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_AGC_ENABLE_STATE` GET | `CCI_GetAGCEnabled()` | `Lepton_EnableAGC()` | ✅ |
| `LEP_CID_AGC_ENABLE_STATE` SET | `CCI_SetAGCEnabled()` | `Lepton_EnableAGC()` | ✅ |

### 4.4.4 AGC Policy

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_AGC_POLICY` GET | `CCI_GetAGCPolicy()` | — | ⚙️ |
| `LEP_CID_AGC_POLICY` SET | `CCI_SetAGCPolicy()` | — | ⚙️ |

### 4.4.6 AGC ROI

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_AGC_ROI` GET | `CCI_GetAGCROI()` | `Lepton_GetAGCROI()` | ✅ |
| `LEP_CID_AGC_ROI` SET | `CCI_SetAGCROI()` | `Lepton_SetAGCROI()` | ✅ |

### 4.4.8 AGC Histogram Statistics

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_AGC_STATISTICS` GET | ⚙️ `CCI_CMD_AGC_GET_HISTOGRAM_STATISTICS` defined | — | ⚙️ |

> **Recommendation:** Useful — exposes min/max/mean pixel intensity and pixel count.
> Beneficial for custom tone-mapping and diagnostics.

### 4.4.10 AGC Linear Histogram Clip Percent

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_AGC_HISTOGRAM_CLIP_PERCENT` GET/SET | — | — | ❌ |

### 4.4.12 AGC Linear Histogram Tail Size

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_AGC_HISTOGRAM_TAIL_SIZE` GET/SET | — | — | ❌ |

### 4.4.14 AGC Linear Max Gain

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_AGC_LINEAR_MAX_GAIN` GET/SET | — | — | ❌ |

### 4.4.16 AGC Linear Midpoint

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_AGC_LINEAR_MIDPOINT` GET/SET | — | — | ❌ |

### 4.4.18 AGC Linear Dampening Factor

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_AGC_LINEAR_DAMPENING_FACTOR` GET/SET | — | — | ❌ |

### 4.4.20 AGC HEQ Dampening Factor

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_AGC_HEQ_DAMPENING_FACTOR` GET | ⚙️ `CCI_CMD_AGC_GET_HEQ_DAMPENING` defined | — | ⚙️ |
| `LEP_CID_AGC_HEQ_DAMPENING_FACTOR` SET | ⚙️ `CCI_CMD_AGC_SET_HEQ_DAMPENING` defined | — | ⚙️ |

> **Recommendation:** Useful when HEQ AGC policy is active — controls smoothness
> of histogram equalization between frames.

### 4.4.22 AGC HEQ Max Gain

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_AGC_HEQ_MAX_GAIN` GET/SET | — | — | ❌ |

### 4.4.24 AGC HEQ Clip Limit High

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_AGC_HEQ_CLIP_LIMIT_HIGH` GET | ⚙️ `CCI_CMD_AGC_GET_HEQ_CLIP_LIMIT_HIGH` defined | — | ⚙️ |
| `LEP_CID_AGC_HEQ_CLIP_LIMIT_HIGH` SET | ⚙️ `CCI_CMD_AGC_SET_HEQ_CLIP_LIMIT_HIGH` defined | — | ⚙️ |

### 4.4.26 AGC HEQ Clip Limit Low

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_AGC_HEQ_CLIP_LIMIT_LOW` GET | ⚙️ `CCI_CMD_AGC_GET_HEQ_CLIP_LIMIT_LOW` defined | — | ⚙️ |
| `LEP_CID_AGC_HEQ_CLIP_LIMIT_LOW` SET | ⚙️ `CCI_CMD_AGC_SET_HEQ_CLIP_LIMIT_LOW` defined | — | ⚙️ |

> **Recommendation (Clip Limits):** Useful for controlling the contrast of HEQ output.
> Clip-limit high prevents over-brightening of hot pixels; clip-limit low prevents
> clipping of cold regions.

### 4.4.28 AGC HEQ Bin Extension

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_AGC_HEQ_BIN_EXTENSION` GET/SET | — | — | ❌ |

### 4.4.30 AGC HEQ Midpoint

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_AGC_HEQ_MIDPOINT` GET/SET | — | — | ❌ |

### 4.4.32 AGC HEQ Empty Counts

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_AGC_HEQ_EMPTY_COUNTS` GET | ⚙️ `CCI_CMD_AGC_GET_HEQ_EMPTY_COUNTS` defined | — | ⚙️ |
| `LEP_CID_AGC_HEQ_EMPTY_COUNTS` SET | ⚙️ `CCI_CMD_AGC_SET_HEQ_EMPTY_COUNTS` defined | — | ⚙️ |

### 4.4.34 AGC HEQ Normalization Factor

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_AGC_HEQ_NORMALIZATION_FACTOR` GET/SET | — | — | ❌ |

### 4.4.36 AGC HEQ Scale Factor (Output Scale)

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_AGC_HEQ_SCALE_FACTOR` GET | ⚙️ `CCI_CMD_AGC_GET_HEQ_OUTPUT_SCALE` defined | — | ⚙️ |
| `LEP_CID_AGC_HEQ_SCALE_FACTOR` SET | ⚙️ `CCI_CMD_AGC_SET_HEQ_OUTPUT_SCALE` defined | — | ⚙️ |

> **Recommendation:** Useful — controls whether the AGC output is scaled to 8-bit
> or 14-bit. Must match the expected downstream bit-depth.

### 4.4.38 AGC Calculation Enable State

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_AGC_CALC_ENABLE_STATE` GET | `CCI_GetAGCCalc()` | — | ⚙️ |
| `LEP_CID_AGC_CALC_ENABLE_STATE` SET | `CCI_SetAGCCalc()` | — | ⚙️ |

### 4.4.40 AGC HEQ Linear Percent

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_AGC_HEQ_LINEAR_PERCENT` GET | ⚙️ `CCI_CMD_AGC_GET_HEQ_LINEAR` defined | — | ⚙️ |
| `LEP_CID_AGC_HEQ_LINEAR_PERCENT` SET | ⚙️ `CCI_CMD_AGC_SET_HEQ_LINEAR` defined | — | ⚙️ |

---

## 4.5 SYS Module — System

### 4.5.2 Ping

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_SYS_PING` RUN | used internally in `CCI_WaitForBoot()` | `Lepton_Init()` | ✅ |

### 4.5.4 Camera Status

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_SYS_CAM_STATUS` GET | — | — | ❌ |

> **Recommendation:** Useful — returns the camera's current operational state
> (`READY`, `INITIALIZING`, `IN_LOW_POWER_MODE`, `FLAT_FIELD_IN_PROCESS`, etc.).
> Valuable for displaying camera state in the UI and handling FFC in progress.

### 4.5.6 FLIR Serial Number

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_SYS_FLIR_SERIAL_NUMBER` GET | `CCI_GetSerialNumber()` | `Lepton_Init()` (auto) | ✅ |

### 4.5.8 Customer Serial Number

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_SYS_CUST_SERIAL_NUMBER` GET | — | — | ❌ |

> **Recommendation:** Low priority — same device; customer serial is typically
> identical to or derived from the FLIR serial number.

### 4.5.10 Camera Uptime

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_SYS_CAM_UPTIME` GET | `CCI_GetUptime()` | `Lepton_GetUptime()` | ✅ |

### 4.5.12 AUX Temperature (Kelvin)

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_SYS_AUX_TEMPERATURE_KELVIN` GET | `CCI_GetAuxTemp()` | `Lepton_GetTemperature()` | ✅ |

### 4.5.14 FPA Temperature (Kelvin)

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_SYS_FPA_TEMPERATURE_KELVIN` GET | `CCI_GetFPATemp()` | `Lepton_GetTemperature()` | ✅ |

### 4.5.16 Telemetry Enable State

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_SYS_TELEMETRY_ENABLE_STATE` GET | `CCI_GetTelemetry()` | — | ⚙️ |
| `LEP_CID_SYS_TELEMETRY_ENABLE_STATE` SET | `CCI_SetTelemetry()` | `Lepton_EnableTelemetry()` | ✅ |

### 4.5.18 Telemetry Location

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_SYS_TELEMETRY_LOCATION` GET | `CCI_GetTelemetryPosition()` | — | ⚙️ |
| `LEP_CID_SYS_TELEMETRY_LOCATION` SET | `CCI_SetTelemetryPosition()` | — | ⚙️ |

### 4.5.20 Execute Frame Average (Run)

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_SYS_EXECTUE_FRAME_AVERAGE` RUN | — | — | ❌ |

> **Recommendation:** Useful — triggers on-chip frame averaging which reduces
> temporal noise. Practical for still-image captures (e.g., saving an image to SD).

### 4.5.22 Number of Frames to Average

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_SYS_NUM_FRAMES_TO_AVERAGE` GET/SET | — | — | ❌ |

> **Recommendation:** Goes together with Frame Average above.
> Values: 1, 2, 4, 8, 16, 32, 64, 128 frames.

### 4.5.24 Scene Statistics

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_SYS_SCENE_STATISTICS` GET | `CCI_GetSceneStatistics()` | `Lepton_GetSceneStatistics()` | ✅ |

### 4.5.26 Scene ROI

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_SYS_SCENE_ROI` GET | `CCI_GetSceneROI()` | `Lepton_GetSceneROI()` | ✅ |
| `LEP_CID_SYS_SCENE_ROI` SET | `CCI_SetSceneROI()` | `Lepton_SetSceneROI()` | ✅ |

### 4.5.28 Thermal Shutdown Count

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_SYS_THERMAL_SHUTDOWN_COUNT` GET | — | — | ❌ |

> **Recommendation:** Useful for diagnostics — counts how often the camera reached
> its over-temperature protection threshold.

### 4.5.30 Shutter Position

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_SYS_SHUTTER_POSITION` GET | `CCI_GetShutterPosition()` | — | ⚙️ |
| `LEP_CID_SYS_SHUTTER_POSITION` SET | `CCI_SetShutterPosition()` | — | ⚙️ |

### 4.5.32 FFC Shutter Mode Object

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_SYS_FFC_SHUTTER_MODE_OBJ` GET/SET | — | — | ❌ |

> **Recommendation:** Highly recommended — controls whether FFC is triggered
> manually, automatically (by the camera), or externally. Also configures the
> FFC period (time between automatic FFCs) and temperature delta threshold.
> Currently FFC can only be run manually via `Lepton_RunCCI()`.

### 4.5.34 Run FFC Normalization

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `FLR_CID_SYS_RUN_FFC` RUN | `CCI_RunFFC()` | `Lepton_RunCCI()` | ✅ |

### 4.5.36 FFC Status

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_SYS_FFC_STATUS` GET | — | — | ❌ |

> **Recommendation:** Useful — lets the host poll whether an FFC is still in progress
> (`BUSY`) or completed. Helpful for blocking the UI during FFC.

### 4.5.38 Gain Mode

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_SYS_GAIN_MODE` GET | `CCI_GetGainMode()` | — | ⚙️ |
| `LEP_CID_SYS_GAIN_MODE` SET | `CCI_SetGainMode()` | — | ⚙️ |

### 4.5.40 FFC States

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_SYS_FFC_STATE` GET | — | — | ❌ |

> **Recommendation:** Medium priority — similar to FFC Status but returns the
> detailed FFC state machine state.

### 4.5.42 Gain Mode Object

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_SYS_GAIN_MODE_OBJ` GET/SET | — | — | ❌ |

> **Recommendation:** Medium priority — the full Gain Mode Object configures
> thresholds for automatic switching between High/Low gain modes, which is
> useful for wide-dynamic-range scenarios.

### 4.5.44 Gain Mode Desired Flag

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_SYS_GAIN_MODE_DESIRED_FLAG` GET | — | — | ❌ |

---

## 4.6 VID Module — Video Processing

### 4.6.2 Video Polarity Select

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_VID_POLARITY_SELECT` GET/SET | — | — | ❌ |

> **Recommendation:** Highly recommended — switches between white-hot and
> black-hot display polarity. This is a very common user-facing setting in
> thermal cameras.

### 4.6.4 Pseudo-Color LUT Select

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_VID_LUT_SELECT` GET | ⚙️ `CCI_CMD_VID_GET_VIDEO_LOOKUP` defined | — | ⚙️ |
| `LEP_CID_VID_LUT_SELECT` SET | ⚙️ `CCI_CMD_VID_SET_VIDEO_LOOKUP` defined | — | ⚙️ |

> **Recommendation:** Useful when operating in RGB888 mode — selects the
> on-chip hardware palette (Wheel6, Fusion, Rainbow, Globow, Sepia, Color,
> Ice-Fire, Rain, User-Defined). Note: the driver currently uses a software
> palette (`Lepton_Raw14ToRGB()`); this command controls the hardware palette
> used in RGB888 mode.

### 4.6.6 Custom LUT (User-Defined)

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_VID_LUT_TRANSFER` GET | ⚙️ `CCI_CMD_VID_GET_VIDEO_CUSTOM_LOOKUP` defined | — | ⚙️ |
| `LEP_CID_VID_LUT_TRANSFER` SET | ⚙️ `CCI_CMD_VID_SET_VIDEO_CUSTOM_LOOKUP` defined | — | ⚙️ |

> **Recommendation:** Useful companion to the hardware LUT select. Allows
> uploading a fully custom 256-entry RGBA lookup table to the camera.

### 4.6.8 Video Focus Calculation Enable

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_VID_FOCUS_CALC_ENABLE` GET | ⚙️ `CCI_CMD_VID_GET_VIDEO_FOCUS_CALC_ENABLE` defined | — | ⚙️ |
| `LEP_CID_VID_FOCUS_CALC_ENABLE` SET | ⚙️ `CCI_CMD_VID_SET_VIDEO_FOCUS_CALC_ENABLE` defined | — | ⚙️ |

> **Recommendation:** Low priority for this application — focus metric is more
> relevant to visible-light/paired-sensor designs.

### 4.6.10 Video Focus ROI

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_VID_FOCUS_ROI` GET | `CCI_GetVideoFocusROI()` | `Lepton_GetVideoFocusROI()` | ✅ |
| `LEP_CID_VID_FOCUS_ROI` SET | `CCI_SetVideoFocusROI()` | `Lepton_SetVideoFocusROI()` | ✅ |

### 4.6.12 Video Focus Metric Threshold

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_VID_FOCUS_THRESHOLD` GET | ⚙️ `CCI_CMD_VID_GET_VIDEO_FOCUS_THRESHOLD` defined | — | ⚙️ |
| `LEP_CID_VID_FOCUS_THRESHOLD` SET | ⚙️ `CCI_CMD_VID_SET_VIDEO_FOCUS_THRESHOLD` defined | — | ⚙️ |

### 4.6.14 Video Focus Metric (Read-only)

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_VID_FOCUS_METRIC` GET | ⚙️ `CCI_CMD_VID_GET_VIDEO_FOCUS_METRIC` defined | — | ⚙️ |

### 4.6.16 Scene-Based NUC Enable

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_VID_SBNUC_ENABLE` GET/SET | — | — | ❌ |

> **Recommendation:** Medium priority — SBNUC (Scene-Based Non-Uniformity
> Correction) improves image quality by correcting non-uniformities without
> a shutter event. Beneficial in applications where the shutter click is
> undesirable.

### 4.6.18 Gamma Select

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_VID_GAMMA_SELECT` GET/SET | — | — | ❌ |

> **Recommendation:** Low priority — gamma correction for RGB888 output.
> Not relevant when using the RAW14 + software palette path.

### 4.6.20 Video Freeze Enable

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_VID_FREEZE_ENABLE` GET | `CCI_GetVideoFreeze()` | — | ⚙️ |
| `LEP_CID_VID_FREEZE_ENABLE` SET | `CCI_SetVideoFreeze()` | `Lepton_FreezeVideo()` | ✅ |

### 4.6.22 Video Output Format

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_VID_VIDEO_OUTPUT_FORMAT` GET | `CCI_GetVideoFormat()` | `Lepton_GetVideoFormat()` | ✅ |
| `LEP_CID_VID_VIDEO_OUTPUT_FORMAT` SET | `CCI_SetVideoFormat()` | `Lepton_SetVideoFormat()` | ✅ |

### 4.6.24 Low-Gain Color LUT Select

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_VID_LOW_GAIN_COLOR_LUT` GET | ⚙️ `CCI_CMD_VID_GET_VIDEO_PSEUDO_COLOR_SELECT` defined | — | ⚙️ |
| `LEP_CID_VID_LOW_GAIN_COLOR_LUT` SET | ⚙️ `CCI_CMD_VID_SET_VIDEO_PSEUDO_COLOR_SELECT` defined | — | ⚙️ |

> **Recommendation:** Useful when dual-gain mode (High/Low) is active — allows
> a different color palette for low-gain imagery.

### 4.6.26 Boresight Calculation Enable (VID)

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_VID_BORESIGHT_CALC_ENABLE` GET/SET | — | — | ❌ |

> **Recommendation:** Not relevant for this application.

### 4.6.28 Boresight Coordinates (VID)

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_VID_BORESIGHT_COORDINATES` GET | — | — | ❌ |

> **Recommendation:** Not relevant for this application.

---

## 4.7 OEM Module — OEM (Protected Commands)

All OEM commands require the OEM protection bit (`0x4000`) in the command word.

### 4.7.2 Power Down

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_OEM_POWER_DOWN` RUN | — | — | ❌ |

> **Recommendation:** Medium priority — enables software-controlled power-down.
> Useful for battery-powered designs or when the application needs to
> conserve power without a hardware `PWRDN` pin.

### 4.7.4 Standby

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_OEM_STANDBY` RUN | — | — | ❌ |

### 4.7.6 Low Power Mode 1 / 2

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_OEM_LOW_POWER_MODE_1/2` RUN | — | — | ❌ |

### 4.7.8 BIT Test

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_OEM_BIT_TEST` RUN | — | — | ❌ |

> **Recommendation:** Low priority — Built-In Test for factory or diagnostic use.

### 4.7.10 Mask Revision

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_OEM_MASK_REVISION` GET | — | — | ❌ |

> **Recommendation:** Low priority — silicon revision identifier.

### 4.7.12 FLIR Part Number

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_OEM_FLIR_PART_NUMBER` GET | `CCI_GetPartNumber()` | `Lepton_Init()` (auto) | ✅ |

### 4.7.14 Software Version

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_OEM_SOFTWARE_VERSION` GET | `CCI_GetSoftwareVersion()` | `Lepton_Init()` (auto) | ✅ |

### 4.7.16 Video Output Enable

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_OEM_VIDEO_OUTPUT_ENABLE` GET/SET | — | — | ❌ |

> **Recommendation:** Medium priority — can disable the VoSPI output entirely,
> useful when the host does not want to process frames (saves bus bandwidth).

### 4.7.18 Video Output Format (OEM)

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_OEM_VIDEO_OUTPUT_FORMAT` GET/SET | — | — | ❌ |

> **Note:** This is a separate OEM-layer command from the VID module `LEP_CID_VID_VIDEO_OUTPUT_FORMAT`.
> The driver uses the VID module command, which is sufficient for normal operation.

### 4.7.20 Video Output Source

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_OEM_VIDEO_OUTPUT_SOURCE` GET | `CCI_GetVideoSource()` | `Lepton_GetVideoSource()` | ✅ |
| `LEP_CID_OEM_VIDEO_OUTPUT_SOURCE` SET | `CCI_SetVideoSource()` | `Lepton_SetVideoSource()` | ✅ |

### 4.7.22 Video Output Source Constant

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_OEM_VIDEO_OUTPUT_CONSTANT` GET/SET | used internally in `CCI_SetVideoSource()` | — | ⚙️ |

### 4.7.24 Video Output Channel

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_OEM_VIDEO_OUTPUT_CHANNEL` GET/SET | — | — | ❌ |

> **Recommendation:** Low priority — selects SPI/I2C output channel; SPI (VoSPI)
> is always used.

### 4.7.26 Video Gamma Enable

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_OEM_VIDEO_GAMMA_ENABLE` GET/SET | — | — | ❌ |

> **Recommendation:** Low priority — only relevant for RGB888 hardware output.

### 4.7.28 Customer Part Number

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_OEM_CUST_PART_NUMBER` GET | — | — | ❌ |

> **Recommendation:** Low priority — user-defined part number stored in OTP.

### 4.7.30 FFC Normalization Target

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_OEM_FFC_NORMALIZATION_TARGET` GET/SET | — | — | ❌ |

> **Recommendation:** Low priority — factory-level calibration, rarely changed
> in the field.

### 4.7.32 OEM Status (Cal Status)

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_OEM_STATUS` GET | — | — | ❌ |

### 4.7.34 Scene Mean Value

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_OEM_SCENE_MEAN_VALUE` GET | — | — | ❌ |

### 4.7.36 Power Mode

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_OEM_POWER_MODE` GET/SET | — | — | ❌ |

### 4.7.38 GPIO Mode Select

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_OEM_GPIO_MODE_SELECT` GET | `CCI_GetGPIOMode()` | — | ⚙️ |
| `LEP_CID_OEM_GPIO_MODE_SELECT` SET | `CCI_SetGPIOMode()` | — | ⚙️ |

### 4.7.40 GPIO VSYNC Phase Delay

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_OEM_GPIO_VSYNC_PHASE_DELAY` GET/SET | — | — | ❌ |

> **Recommendation:** Useful — allows fine-tuning the delay between the VSYNC
> signal and the actual start of frame data on VoSPI. Useful when the host
> GPIO ISR introduces non-negligible latency.

### 4.7.42 User Defaults State

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_OEM_USER_DEFAULTS` GET | — | — | ❌ |

### 4.7.44 User Defaults Restore

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_OEM_USER_DEFAULTS_RESTORE` RUN | — | — | ❌ |

> **Recommendation:** Medium priority — restores all OEM user-configurable
> settings to factory defaults. Useful as a "factory reset" feature in the UI.

### 4.7.46 Shutter Profile Object

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_OEM_SHUTTER_PROFILE_OBJ` GET/SET | — | — | ❌ |

> **Recommendation:** Medium priority — configures the FFC shutter timing
> profile (open/close delays, dwell time). Important for ensuring correct
> FFC behaviour in the mechanical shutter.

### 4.7.48 Thermal Shutdown Enable

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_OEM_THERMAL_SHUTDOWN_ENABLE_STATE` GET/SET | — | — | ❌ |

> **Recommendation:** Medium priority — enables or disables the hardware
> over-temperature shutdown. Should be exposed to allow advanced configuration.

### 4.7.50 Bad Pixel Replace Control

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_OEM_BAD_PIXEL_REPLACE_CONTROL` GET/SET | — | — | ❌ |

> **Recommendation:** Useful — enables on-chip bad pixel replacement
> (interpolation of defective pixels). Directly improves image quality.

### 4.7.52 Temporal Filter Control

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_OEM_TEMPORAL_FILTER_CONTROL` GET/SET | — | — | ❌ |

> **Recommendation:** Useful — enables the temporal noise filter. Reduces
> frame-to-frame noise at the cost of slight motion blur.

### 4.7.54 Column Noise Estimate Control

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_OEM_COLUMN_NOISE_ESTIMATE_CONTROL` GET/SET | — | — | ❌ |

> **Recommendation:** Useful — removes fixed-pattern column noise artifacts.

### 4.7.56 Pixel Noise Estimate Control

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_OEM_PIXEL_NOISE_ESTIMATE_CONTROL` GET/SET | — | — | ❌ |

> **Recommendation:** Useful — removes fixed-pattern pixel noise artifacts
> (complements the column noise filter).

### 4.7.58 Reboot

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_OEM_REBOOT` RUN | `CCI_RebootCamera()` | — | ⚙️ |

---

## 4.8 RAD Module — Radiometry (Protected Commands)

All RAD commands require the OEM protection bit (`0x4000`) in the command word.

### 4.8.2 RBFO Parameters — Internal High Gain

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_RBFO_INTERNAL` GET/SET | — | — | ❌ |

> **Recommendation:** Not recommended for field implementation — RBFO
> parameters are factory-calibrated and should not be modified by end users.

### 4.8.4 RBFO Parameters — External High Gain

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_RBFO_EXTERNAL` GET/SET | — | — | ❌ |

> **Recommendation:** Medium priority — allows calibrating for an external
> window or optic in front of the sensor. Useful if the device is mounted
> behind glass.

### 4.8.6 TShutter Mode

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_TSHUTTER_MODE` GET/SET | — | — | ❌ |

> **Recommendation:** Medium priority — selects whether the shutter temperature
> is determined by the user, by calibration, or fixed. Relevant for
> achieving accurate temperature measurements.

### 4.8.8 TShutter Value

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_TSHUTTER` GET/SET | — | — | ❌ |

### 4.8.10 Run RAD FFC

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_RUN_FFC` RUN | — | — | ❌ |

> **Note:** The generic FFC (`CCI_RunFFC` / `FLR_CID_SYS_RUN_FFC`) covers this
> use case. The RAD FFC is a radiometry-specific variant.

### 4.8.12 RAD Run Status

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_RUN_STATUS` GET | — | — | ❌ |

### 4.8.14 RAD Enable State

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_ENABLE_STATE` GET | `CCI_GetRadiometry()` | — | ⚙️ |
| `LEP_CID_RAD_ENABLE_STATE` SET | `CCI_SetRadiometry()` | — | ⚙️ |

### 4.8.16 Global Offset

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_GLOBAL_OFFSET` GET/SET | — | — | ❌ |

> **Recommendation:** Not recommended for field use — global flux offset is
> a factory calibration parameter.

### 4.8.18 FPA CTS Mode

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_TFPA_CTS_MODE` GET/SET | — | — | ❌ |

### 4.8.20 FPA CTS (temperature counts)

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_TFPA_CTS` GET/SET | — | — | ❌ |

### 4.8.22 TEQ Shutter LUT

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_TEQ_SHUTTER_LUT` GET/SET | — | — | ❌ |

> **Recommendation:** Not recommended — factory calibration data.

### 4.8.24 Global Gain

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_GLOBAL_GAIN` GET/SET | — | — | ❌ |

### 4.8.26 Radiometry Filter

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_RADIOMETRY_FILTER` GET/SET | — | — | ❌ |

### 4.8.28 TFpa LUT

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_TFPA_LUT` GET/SET | — | — | ❌ |

> **Recommendation:** Not recommended — factory calibration data.

### 4.8.30 TAux LUT

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_TAUX_LUT` GET/SET | — | — | ❌ |

> **Recommendation:** Not recommended — factory calibration data.

### 4.8.32 AUX CTS Mode

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_TAUX_CTS_MODE` GET/SET | — | — | ❌ |

### 4.8.34 TEQ Shutter Flux

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_TEQ_SHUTTER_FLUX` GET/SET | — | — | ❌ |

### 4.8.36 Frame Median Value

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_FRAME_MEDIAN_VALUE` GET | — | — | ❌ |

> **Recommendation:** Useful for diagnostics — median pixel value of the
> current frame before radiometry processing.

### 4.8.38 Housing TCP

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_HOUSING_TCP` GET/SET | — | — | ❌ |

### 4.8.40 Shutter TCP

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_SHUTTER_TCP` GET/SET | — | — | ❌ |

### 4.8.42 Lens TCP

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_LENS_TCP` GET/SET | — | — | ❌ |

### 4.8.44 Arbitrary Offset

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_ARBITRARY_OFFSET` GET/SET | — | — | ❌ |

> **Recommendation:** Useful — a per-device temperature offset correction that
> compensates for systematic measurement offsets in a specific installation
> (e.g., a window in front of the sensor). Differs from the flux-linear
> window parameters in that it is a direct additive offset in Kelvin.

### 4.8.46 Flux Linear Parameters

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_FLUX_LINEAR_PARAMS` GET | `CCI_GetRadiometryFluxLinearParams()` | `Lepton_GetFluxLinearParameters()` | ✅ |
| `LEP_CID_RAD_FLUX_LINEAR_PARAMS` SET | `CCI_SetRadiometryFluxLinearParams()` | `Lepton_SetFluxLinearParameters()`, `Lepton_SetEmissivity()` | ✅ |

### 4.8.48 TLinear Enable State

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_TLINEAR_ENABLE_STATE` GET | `CCI_GetTLinearEnabled()` | — | ⚙️ |
| `LEP_CID_RAD_TLINEAR_ENABLE_STATE` SET | `CCI_SetTLinearEnabled()` | — | ⚙️ |

### 4.8.50 TLinear Resolution

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_TLINEAR_RESOLUTION` GET | `CCI_GetTLinearResolution()` | `Lepton_GetTLinearResolution()` | ✅ |
| `LEP_CID_RAD_TLINEAR_RESOLUTION` SET | `CCI_SetTLinearResolution()` | `Lepton_SetTLinearResolution()` | ✅ |

### 4.8.52 TLinear Auto Resolution

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_TLINEAR_AUTO_RESOLUTION` GET | `CCI_GetRadiometryTLinearAutoRes()` | — | ⚙️ |
| `LEP_CID_RAD_TLINEAR_AUTO_RESOLUTION` SET | `CCI_SetRadiometryTLinearAutoRes()` | — | ⚙️ |

### 4.8.54 Spotmeter ROI

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_SPOTMETER_ROI` GET | `CCI_GetSpotmeterROI()` | `Lepton_GetSpotmeterROI()` | ✅ |
| `LEP_CID_RAD_SPOTMETER_ROI` SET | `CCI_SetSpotmeterROI()` | `Lepton_SetSpotmeterROI()` | ✅ |

### 4.8.56 Spotmeter Value (Kelvin)

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_SPOTMETER_OBJ_KELVIN` GET | `CCI_GetSpotmeter()` | `Lepton_GetSpotmeter()` | ✅ |

### 4.8.58 RBFO Internal — Low Gain

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_RBFO_INTERNAL_LG` GET/SET | — | — | ❌ |

> **Recommendation:** Not recommended — factory calibration data.

### 4.8.60 RBFO External — Low Gain

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_RBFO_EXTERNAL_LG` GET/SET | — | — | ❌ |

### 4.8.62 Arbitrary Offset Mode

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_ARBITRARY_OFFSET_MODE` GET/SET | — | — | ❌ |

### 4.8.64 Arbitrary Offset Parameters

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_ARBITRARY_OFFSET_PARAMS` GET/SET | — | — | ❌ |

> **Recommendation (Arbitrary Offset Mode + Params):** Together with
> `LEP_CID_RAD_ARBITRARY_OFFSET`, these allow auto-applying a decay-based
> temperature correction offset. Useful for long-duration measurement accuracy.

### 4.8.66 Radio Cal Values

| IDD Command | CCI Function | Lepton API | Status |
| --- | --- | --- | --- |
| `LEP_CID_RAD_RADIO_CAL_VALUES` GET/SET | — | — | ❌ |

> **Recommendation:** Read-only useful — returns the current FPA/AUX temperature
> counts and Kelvin values used internally by the radiometry engine, without
> requiring individual `SYS_GET_FPA_TEMP` + `SYS_GET_AUX_TEMP` calls.
