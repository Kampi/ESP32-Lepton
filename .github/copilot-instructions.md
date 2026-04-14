# GitHub Copilot Instructions for ESP32-Lepton Component

## Project Overview

ESP32-Lepton is an ESP-IDF component providing a complete driver implementation for FLIR Lepton 3.x thermal imaging sensors. The component handles SPI-based video streaming (VoSPI), I2C command interface (CCI), and provides a high-level API for integration into ESP32 projects.

**Key Technologies:**
- Platform: ESP32, ESP32-S3 (ESP-IDF framework)
- Build System: CMake (ESP-IDF component)
- Communication: SPI (VoSPI), I2C (CCI)
- RTOS: FreeRTOS
- DMA: High-speed frame acquisition

---

## Code Style and Formatting

### General Formatting Rules

The project uses **Artistic Style (AStyle)** with a K&R-based configuration (`scripts/astyle.cfg`):

- **Style**: K&R (Kernighan & Ritchie)
- **Indentation**: 4 spaces (no tabs)
- **Line Length**: Maximum 120 characters
- **Braces**: K&R style (opening brace on same line for functions, control structures)
- **Operators**: Space padding around operators: `a = bar((b - c) * a, d--);`
- **Headers**: Space between header and bracket: `if (condition) {`
- **Pointers/References**: Stick to name: `char *pThing`, `char &thing`
- **Conditionals**: Always use braces, even for single-line blocks
- **Switch**: Indent case statements

**Example:**
```cpp
esp_err_t Lepton_Init(Lepton_Handle_t *p_Handle, const Lepton_Config_t *p_Config)
{
    if ((p_Handle == NULL) || (p_Config == NULL)) {
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t Error = initialize_spi_bus(p_Config);
    if (Error != ESP_OK) {
        return Error;
    }
    
    return ESP_OK;
}
```

### Naming Conventions

#### Functions
- **Public API**: `Lepton_FunctionName()` using PascalCase with module prefix
  - Examples: `Lepton_Init()`, `Lepton_CCI_ReadRegister()`, `VoSPI_StartCapture()`
- **Private/Static**: `snake_case` with descriptive names
  - Examples: `init_spi_device()`, `process_segment()`, `calculate_crc()`

#### Variables
- **Local variables**: `PascalCase` — **all** local variables, including booleans and loop-adjacent variables
  - Examples: `RetryCount`, `FrameBuffer`, `Error`, `IsInitialized`, `IsCapturing`
  - Simple single-letter loop counters (`i`, `x`, `y`) are the only exception
- **Pointers**: Prefix with `p_` (e.g., `p_Handle`, `p_Config`, `p_Data`)
- **Global/Static module state**: Prefix with underscore: `_Lepton_State`, `_VoSPI_Context`

#### Constants and Macros
- **All UPPERCASE** with underscores: `LEPTON_WIDTH`, `LEPTON_HEIGHT`, `VOSPI_PACKET_SIZE`
- Module-specific macros should include module prefix: `LEPTON_TELEMETRY_ENABLED`, `CCI_TIMEOUT_MS`

#### Types
- **Structs/Enums**: `Lepton_Description_t` or `ModuleName_Description_t` with `_t` suffix
  - Examples: `Lepton_Handle_t`, `Lepton_Config_t`, `VoSPI_Packet_t`, `CCI_Command_t`
- **Enums**: Use descriptive prefix for values
  - Example: `LEPTON_ERROR_NONE`, `LEPTON_ERROR_TIMEOUT`, `CCI_STATUS_BUSY`

#### File Names
- Header files: `moduleName.h` (e.g., `lepton.h`, `lepton_cci.h`, `vospi.h`)
- Implementation files: `moduleName.cpp` or `moduleName.c`
- Private headers: `Private/internalModule.h`

### Code Organization

#### Directory Structure
```text
ESP32-Lepton/
├── CMakeLists.txt             # Component build configuration
├── idf_component.yml          # Component manifest
├── Kconfig                    # Component configuration options
├── include/                   # Public API headers
│   ├── lepton.h               # Main driver API
│   ├── lepton_cci.h           # CCI high-level interface
│   ├── lepton_capture.h       # Capture task interface
│   ├── CCI/
│   │   └── cci.h              # CCI low-level protocol
│   ├── VoSPI/
│   │   └── vospi.h            # VoSPI interface
│   └── Definitions/           # Type definitions and constants
├── src/                       # Implementation files
│   ├── lepton.cpp
│   ├── lepton_cci.cpp
│   ├── lepton_capture.cpp
│   ├── CCI/
│   │   └── cci.cpp
│   └── VoSPI/
│       └── vospi.cpp
├── docs/                      # AsciiDoc documentation
└── examples/                  # Example applications
```

---

## License and Copyright

### File Headers

**Every source file** (`.cpp`, `.h`, `.c`) must include the following header:

```cpp
/*
 * filename.cpp
 *
 *  Copyright (C) Daniel Kampert, 2026
 *  Website: www.kampis-elektroecke.de
 *  File info: Brief description of the file's purpose.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * Errors and commissions should be reported to DanielKampert@kampis-elektroecke.de
 */
```

**License**: GNU General Public License v3.0  
**Copyright Holder**: Daniel Kampert  
**Contact**: DanielKampert@kampis-elektroecke.de  
**Website**: www.kampis-elektroecke.de

---

## Documentation Standards

### Function Documentation Requirements

**CRITICAL**: Every function declaration in header files MUST include complete Doxygen documentation.

#### Documentation Placement Rule

Function documentation belongs exclusively in the **header file** (`.h`) alongside the declaration - not in the implementation file.

**Exception:** `static` functions that are only defined in a `.cpp`/`.c` file and have no header declaration. For these, the Doxygen comment is placed directly above the function definition in the implementation file.

| Function type | Documentation location |
|--------------|------------------------|
| Public API (declared in `.h`) | Header file (`.h`) only |
| `static` (defined in `.cpp`/`.c` only) | Implementation file (`.cpp`/`.c`) |

```cpp
// ✅ CORRECT: Public API → documented in header (.h)
// lepton.h:
/** @brief          Initialize the Lepton driver.
 *  @return         LEPTON_ERR_OK on success
 */
Lepton_Error_t Lepton_Init(Lepton_Config_t *p_Config);

// lepton.cpp: NO Doxygen comment above the definition
Lepton_Error_t Lepton_Init(Lepton_Config_t *p_Config)
{
    // ...
}

// ✅ CORRECT: static function → documented in .cpp only
// lepton.cpp:
/** @brief          Reset the VoSPI synchronization state.
 *  @param p_Config Pointer to Lepton configuration
 */
static void resync_vospi(Lepton_Config_t *p_Config)
{
    // ...
}
```

```cpp
// ❌ INCORRECT: Doxygen comment in .cpp for a public function
// lepton.cpp:
/** @brief Initialize the Lepton driver. */  // ← wrong place
Lepton_Error_t Lepton_Init(Lepton_Config_t *p_Config)
{
    // ...
}
```

#### Doxygen-Style Documentation for ALL Functions

Use Doxygen comments for **ALL** public API functions and structures:

```cpp
/** @brief          Initialize the Lepton thermal camera driver.
 *  
 *  This function initializes the SPI and I2C interfaces, configures the
 *  Lepton sensor, and prepares the driver for frame capture operations.
 *  
 *  @param p_Handle Pointer to handle structure that will be initialized
 *  @param p_Config Pointer to configuration structure with SPI/I2C settings
 *  
 *  @return         ESP_OK on success
 *  @return         ESP_ERR_INVALID_ARG if p_Handle or p_Config is NULL
 *  @return         ESP_ERR_NO_MEM if memory allocation fails
 *  @return         ESP_ERR_TIMEOUT if sensor initialization times out
 *  
 *  @note           This function must be called before any other Lepton API calls.
 *  @warning        Ensure SPI and I2C pins are not used by other peripherals.
 */
esp_err_t Lepton_Init(Lepton_Handle_t *p_Handle, const Lepton_Config_t *p_Config);
```

**Mandatory documentation elements:**
- `@brief` - Short one-line description
- `@param` - Description for EACH parameter (include direction: in/out/inout if relevant)
- `@return` - Document ALL possible return values (one `@return` per value)
- `@note` - Important usage notes (optional but recommended)
- `@warning` - Critical warnings about misuse or side effects (if applicable)

#### Inline Comments
- Use `//` for single-line comments
- Use `/* */` for block comments
- Add explanatory comments for complex logic
- Comment "why", not "what" (the code shows what)

**Example:**
```cpp
/* Calculate CRC-16 for data integrity verification as per Lepton datasheet section 4.2.3 */
uint16_t Crc = calculate_crc16(p_Data, Length);
```

### Structure Documentation

Document struct members inline with Doxygen format:

```cpp
/** @brief Lepton camera configuration structure.
 */
typedef struct {
    gpio_num_t CS_Pin;          /**< SPI chip select GPIO pin. */
    gpio_num_t VSync_Pin;       /**< VSync signal GPIO pin (optional, set to -1 to disable). */
    uint32_t SPI_Clock_Hz;      /**< SPI clock frequency in Hz (max 20 MHz for Lepton 3.x). */
    i2c_port_t I2C_Port;        /**< I2C port number for CCI communication. */
    uint8_t I2C_Address;        /**< I2C device address (typically 0x2A). */
} Lepton_Config_t;
```

### Enum Documentation

```cpp
/** @brief Lepton sensor status codes.
 */
typedef enum {
    LEPTON_STATUS_READY = 0,    /**< Sensor ready for capture. */
    LEPTON_STATUS_BUSY,         /**< Sensor busy processing command. */
    LEPTON_STATUS_ERROR,        /**< Sensor error state. */
    LEPTON_STATUS_BOOTING       /**< Sensor booting up (after power-on). */
} Lepton_Status_t;
```

---

## Code Quality and Validation

### Mandatory Checks After Code Changes

**CRITICAL**: After making ANY code changes, you MUST:

1. **Syntax Validation**
   - Verify code compiles without errors
   - Check for correct bracket matching, semicolons, and syntax
   - Validate all include statements and dependencies

2. **Error Checking**
   - Run static analysis if available
   - Check for compiler warnings and address them
   - Verify all error paths are handled correctly
   - Ensure all ESP-IDF return codes are checked

3. **Documentation Sync**
   - Update function documentation if signatures change
   - Update parameter descriptions if behavior changes
   - Verify examples in documentation still compile

**Example validation checklist for each change:**
```text
☐ Code compiles without errors
☐ No compiler warnings introduced
☐ All function declarations have complete Doxygen documentation
☐ All return values documented
☐ Error handling implemented for all ESP-IDF calls
☐ Related documentation (.adoc files) updated
☐ Code formatted with AStyle
```

---

## ESP-IDF Specific Conventions

### Error Handling

- **Always check return values** from ESP-IDF functions
- Use `ESP_ERROR_CHECK()` only for critical initialization that should abort
- Use manual error handling for all other cases:

```cpp
esp_err_t Error = spi_bus_initialize(SPI_Bus, &Config, SPI_DMA_CH_AUTO);
if (Error != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(Error));
    return Error;
}
```

### Logging

Use ESP-IDF logging macros with appropriate levels:
- `ESP_LOGE(TAG, ...)` - Errors
- `ESP_LOGW(TAG, ...)` - Warnings  
- `ESP_LOGI(TAG, ...)` - Information
- `ESP_LOGD(TAG, ...)` - Debug

Define TAG at the top of each file:
```cpp
static const char *TAG = "Lepton";
```

### Component Configuration (Kconfig)

- Add configuration options in `Kconfig` for user-configurable parameters
- Use appropriate config types (bool, int, choice, string)
- Provide sensible defaults
- Document each option clearly

**Example:**
```kconfig
menu "Lepton Configuration"
    config LEPTON_ENABLE_TELEMETRY
        bool "Enable telemetry data extraction"
        default y
        help
            Enable extraction and parsing of telemetry data from Lepton frames.

    config LEPTON_SPI_MAX_TRANSFER_SIZE
        int "Maximum SPI transfer size"
        default 4092
        range 1024 4092
        help
            Maximum size for single SPI transfer in bytes.
endmenu
```

---

## Component Design Patterns

### Public API Pattern

Public APIs follow a consistent pattern:

```cpp
// Initialization and cleanup
esp_err_t Lepton_Init(Lepton_Handle_t *p_Handle, const Lepton_Config_t *p_Config);
esp_err_t Lepton_Deinit(Lepton_Handle_t Handle);

// Configuration
esp_err_t Lepton_GetConfig(Lepton_Handle_t Handle, Lepton_Config_t *p_Config);
esp_err_t Lepton_SetConfig(Lepton_Handle_t Handle, const Lepton_Config_t *p_Config);

// Operations
esp_err_t Lepton_CaptureFrame(Lepton_Handle_t Handle, uint16_t *p_Buffer, size_t Size);
esp_err_t Lepton_StartContinuousCapture(Lepton_Handle_t Handle, Lepton_FrameCallback_t Callback);
esp_err_t Lepton_StopContinuousCapture(Lepton_Handle_t Handle);
```

### Handle-Based Design

Use opaque handles for encapsulation:

```cpp
// Public (in header)
typedef void* Lepton_Handle_t;

// Private (in implementation)
typedef struct {
    spi_device_handle_t SPI_Handle;
    i2c_port_t I2C_Port;
    gpio_num_t VSync_Pin;
    SemaphoreHandle_t Mutex;
    bool IsInitialized;
} Lepton_Context_t;
```

### Thread Safety

- Use mutexes for shared state access
- Document thread-safety guarantees in function documentation
- Use `@threadsafe` or `@note Thread-safe` tags

```cpp
/** @brief          Get current sensor temperature.
 *  @param Handle   Lepton handle
 *  @param p_Temp   Pointer to store temperature value
 *  @return         ESP_OK on success, error code otherwise
 *  @note           This function is thread-safe.
 */
esp_err_t Lepton_GetTemperature(Lepton_Handle_t Handle, float *p_Temp);
```

---

## Hardware Interface Guidelines

### SPI (VoSPI) Interface

- Support both software and hardware CS control
- Use DMA for frame transfers when available
- Implement packet validation (CRC, ID, sequence)
- Handle discard frames and resynchronization

### I2C (CCI) Interface

- Implement proper I2C timing (wait states)
- Handle busy status polling
- Support register read/write operations
- Implement retry logic for transient failures

### GPIO (VSync)

- Support optional VSync for frame synchronization
- Use interrupts for low-latency frame detection
- Provide callback mechanism for applications

---

## Documentation Maintenance

### AsciiDoc Documentation

**When modifying code, always update the corresponding `.adoc` documentation.**

#### Documentation Structure

```text
docs/
├── index.adoc              # Component overview
├── lepton.adoc             # Main driver API
├── lepton_cci.adoc         # CCI interface
├── lepton_capture.adoc     # Capture task
├── cci.adoc                # Low-level CCI
└── vospi.adoc              # VoSPI interface
```

#### When to Update Documentation

Update documentation when:
- Adding new public API functions
- Modifying function signatures or behavior
- Changing configuration options (Kconfig)
- Adding new features or modes
- Fixing bugs that affect documented behavior

---

## Testing and Debugging

### Debugging

- Use `ESP_LOGD` for verbose debug output
- Add conditional compilation for debug code: `#if CONFIG_LOG_DEFAULT_LEVEL >= ESP_LOG_DEBUG`
- Provide diagnostic functions for troubleshooting

### Error Reporting

- Log errors with context: `ESP_LOGE(TAG, "Failed to read register 0x%04X: %s", reg, esp_err_to_name(err));`
- Use esp_err_to_name() for readable error messages
- Include relevant parameters and state information

---

## Best Practices

### Memory Management

- Check malloc/heap allocation success
- Free allocated memory in error paths
- Use stack allocation for small buffers
- Consider PSRAM for large frame buffers

### Performance

- Use DMA for SPI transfers
- Minimize I2C transactions (batch reads/writes where possible)
- Profile critical paths (frame acquisition loop)
- Optimize hot paths identified by profiling

### Portability

- Support ESP32 and ESP32-S3 (check in CMakeLists.txt)
- Use ESP-IDF abstraction layers (no direct register access)
- Test on different ESP-IDF versions (5.x+)
- Document hardware requirements and limitations

---

## Common Pitfalls to Avoid

❌ **Don't:**
- Mix tabs and spaces
- Exceed 120 character line length
- Forget error checking on ESP-IDF calls
- Omit function documentation
- Skip syntax validation after changes
- Ignore compiler warnings
- Use blocking operations without timeouts
- Access hardware registers directly

✅ **Do:**
- Use consistent naming conventions
- **Document ALL public functions with complete Doxygen comments**
- Include license headers in all files
- **Validate syntax and check for errors after every code change**
- Test error paths and edge cases
- Use appropriate log levels
- Follow ESP-IDF coding standards
- Update corresponding documentation when changing code

---

## Additional Resources

- **ESP-IDF Documentation**: https://docs.espressif.com/projects/esp-idf/
- **FLIR Lepton Datasheet**: Refer to official FLIR documentation
- **Component Repository**: https://github.com/Kampi/ESP32-Lepton

---

**Last Updated**: January 26, 2026  
**Maintainer**: Daniel Kampert (DanielKampert@kampis-elektroecke.de)
