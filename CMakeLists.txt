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

if((IDF_TARGET STREQUAL "esp32") OR (IDF_TARGET STREQUAL "esp32s2") OR (IDF_TARGET STREQUAL "esp32s3") OR (IDF_TARGET STREQUAL "esp32c2") OR (IDF_TARGET STREQUAL "esp32c3") OR (IDF_TARGET STREQUAL "esp32c6"))
	list(APPEND COMPONENT_PRIV_REQUIRES esp_timer)
endif()

idf_component_register(SRCS ${COMPONENT_SRCS}
                       INCLUDE_DIRS ${COMPONENT_ADD_INCLUDEDIRS}
                       PRIV_INCLUDE_DIRS ${COMPONENT_PRIV_INCLUDEDIRS}
                       PRIV_REQUIRES ${COMPONENT_PRIV_REQUIRES}
                       )

target_compile_definitions(${COMPONENT_LIB} PUBLIC LEPTON_LIB_MAJOR=1)
target_compile_definitions(${COMPONENT_LIB} PUBLIC LEPTON_LIB_MINOR=0)
target_compile_definitions(${COMPONENT_LIB} PUBLIC LEPTON_LIB_BUILD=0)