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

set(COMPONENT_PRIV_REQUIRES freertos driver)

if((IDF_TARGET STREQUAL "esp32") OR (IDF_TARGET STREQUAL "esp32s2") OR (IDF_TARGET STREQUAL "esp32s3") OR (IDF_TARGET STREQUAL "esp32c2") OR (IDF_TARGET STREQUAL "esp32c3"))
	list(APPEND COMPONENT_PRIV_REQUIRES esp_timer)
endif()

register_component()