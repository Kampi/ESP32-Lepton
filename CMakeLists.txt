set(COMPONENT_SRCS
    "src/lepton.cpp"
    "src/lepton_cci.cpp"
    "src/lepton_capture.cpp"
    "src/VoSPI/vospi.cpp"
    "src/CCI/cci.cpp"
    )

set(COMPONENT_ADD_INCLUDEDIRS
	"include"
    "include/Definitions"
    "include/CCI"
    "include/VoSPI"
	)

set(COMPONENT_PRIV_INCLUDEDIRS
    "include/CCI"
    "include/VoSPI"
    )

set(COMPONENT_PRIV_REQUIRES freertos driver espressif__esp-dsp)

if((IDF_TARGET STREQUAL "esp32s3"))
	list(APPEND COMPONENT_PRIV_REQUIRES esp_timer)
endif()

# Library version
set(ESP32_LEPTON_LIB_MAJOR 0)
set(ESP32_LEPTON_LIB_MINOR 0)
set(ESP32_LEPTON_LIB_BUILD 3)

register_component()

# Add version definitions to the component
target_compile_definitions(${COMPONENT_LIB} PUBLIC
    ESP32_LEPTON_LIB_MAJOR=${ESP32_LEPTON_LIB_MAJOR}
    ESP32_LEPTON_LIB_MINOR=${ESP32_LEPTON_LIB_MINOR}
    ESP32_LEPTON_LIB_BUILD=${ESP32_LEPTON_LIB_BUILD}
)