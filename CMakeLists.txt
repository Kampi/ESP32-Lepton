set(COMPONENT_SRCS
    "src/lepton.cpp"
    )

set(COMPONENT_ADD_INCLUDEDIRS
	"include"
	)

if((IDF_TARGET STREQUAL "esp32") OR (IDF_TARGET STREQUAL "esp32c2") OR (IDF_TARGET STREQUAL "esp32c3") OR (IDF_TARGET STREQUAL "esp32s2") OR (IDF_TARGET STREQUAL "esp32s3"))
	list(APPEND COMPONENT_PRIV_REQUIRES esp_timer)
	list(APPEND COMPONENT_SRCS "src/Arch/SPI/lepton_spi.cpp")
	list(APPEND COMPONENT_SRCS "src/Arch/I2C/lepton_i2c.cpp")
	list(APPEND COMPONENT_SRCS "src/Arch/GPIO/lepton_gpio.cpp")
endif()

idf_component_register(SRCS ${COMPONENT_SRCS}
                       INCLUDE_DIRS ${COMPONENT_ADD_INCLUDEDIRS}

                       # Only public requires, because the header files includes them
                       REQUIRES freertos driver

                       # The private requires are only used by the driver
                       PRIV_REQUIRES ${COMPONENT_PRIV_REQUIRES}
                       )

target_compile_definitions(${COMPONENT_LIB} PUBLIC LEPTON_LIB_MAJOR=1)
target_compile_definitions(${COMPONENT_LIB} PUBLIC LEPTON_LIB_MINOR=0)
target_compile_definitions(${COMPONENT_LIB} PUBLIC LEPTON_LIB_BUILD=0)
