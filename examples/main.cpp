#ifndef CONFIG_LEPTON_I2C_INTERFACE
    #define CONFIG_LEPTON_I2C_INTERFACE             1
#endif

#ifndef CONFIG_LEPTON_I2C_SDA
    #define CONFIG_LEPTON_I2C_SDA                   23
#endif

#ifndef CONFIG_LEPTON_I2C_SCL
    #define CONFIG_LEPTON_I2C_SCL                   22
#endif

#ifndef CONFIG_LEPTON_I2C_CLOCK
    #define CONFIG_LEPTON_I2C_CLOCK                 100000
#endif

static Lepton_Conf_t _Conf = LEPTON_DEFAULT_CONF;

extern "C" void app_main(void)
{

    _Conf.CCI.Host = (i2c_port_t)CONFIG_LEPTON_I2C_INTERFACE;
}