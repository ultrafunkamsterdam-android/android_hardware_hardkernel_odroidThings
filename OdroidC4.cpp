#include "OdroidC4.h"

OdroidC4::OdroidC4() {
    pinList = {
        {"", -1, 0},
        {"3.3V", -1, PIN_PWR}, {"5V", -1, PIN_PWR},
        {"3", 8, PIN_GPIO|PIN_I2C}, {"5V", -1, PIN_PWR},
        {"5", 9, PIN_GPIO|PIN_I2C}, {"GND", -1, PIN_GND},
        {"7", 7, PIN_GPIO|PIN_PWM}, {"8", 15, PIN_GPIO|PIN_UART},
        {"GND", -1, PIN_GND}, {"10", 16, PIN_GPIO|PIN_UART},
        {"11", 0, PIN_GPIO|PIN_PWM}, {"12", 1, PIN_GPIO|PIN_PWM},
        {"13", 2, PIN_GPIO}, {"GND", -1, PIN_GND},
        {"15", 3, PIN_GPIO|PIN_PWM|PIN_UART}, {"16", 4, PIN_GPIO},
        {"3.3V", -1, PIN_PWR}, {"18", 5, PIN_GPIO},
        {"19", 12, PIN_GPIO|PIN_SPI}, {"GND", -1, PIN_GND},
        {"21", 13, PIN_GPIO|PIN_SPI}, {"22", 6, PIN_GPIO},
        {"23", 14, PIN_GPIO|PIN_SPI}, {"24", 10, PIN_GPIO|PIN_SPI},
        {"25", -1, PIN_GND}, {"26", 11, PIN_GPIO|PIN_UART},
        {"27", 30, PIN_GPIO|PIN_I2C}, {"28", 31, PIN_GPIO|PIN_I2C},
        {"29", 21, PIN_GPIO}, {"GND", -1, PIN_GND},
        {"31", 22, PIN_GPIO}, {"32", 26, PIN_GPIO|PIN_UART},
        {"33", 23, PIN_GPIO|PIN_PWM|PIN_UART}, {"GND", -1, PIN_GND},
        {"35", 24, PIN_GPIO|PIN_PWM}, {"36", 27, PIN_GPIO},
        {"AIN3", 25, PIN_AIN}, {"1.8V", 28, PIN_PWR},
    };

    i2cList = {
        {"I2C-1", "/dev/i2c-0", 8, 9}, // Pin #3, #5
        {"I2C-2", "/dev/i2c-1", 30, 31}, // Pin #27, #28
    };

    pwmList = {
        { // Pin #7
            .index = 7,
            .path = "/sys/devices/platform/soc/ffd00000.cbus/ffd1a000.pwm",
            .line = 0
        },
        { // Pin #11
            .index = 0,
            .path = "/sys/devices/platform/soc/ffd00000.cbus/ffd1a000.pwm",
            .line = 1
        },
        { // Pin #12
            .index = 1,
            .path = "/sys/devices/platform/soc/ffd00000.cbus/ffd19000.pwm",
            .line = 0
        },
        { // Pin #15
            .index = 3,
            .path = "/sys/devices/platform/soc/ffd00000.cbus/ffd19000.pwm",
            .line = 1
        },
        { // Pin #33
            .index = 23,
            .path =  "/sys/devices/platform/soc/ffd00000.cbus/ffd1b000.pwm",
            .line = 0
        },
        { // Pin #35
            .index = 24,
            .path = "/sys/devices/platform/soc/ffd00000.cbus/ffd1b000.pwm",
            .line = 1
        },
    };

    uartList = {
        {"UART-1", "/dev/ttyS1", 16, 15}, // Pin #10, #8
        {"UART-2", "/dev/ttyS2", 3, 23}, // Pin #15, #35
        {"UART-3", "/dev/ttyS3", 11, 26}, // Pin #26, #32
        {"UART-4", "/dev/ttyS4", -1, -1},
        {"UART-USB-0", "/dev/ttyUSB0", -1, -1},
        {"UART-USB-1", "/dev/ttyUSB1", -1, -1},
        {"UART-ACM-0", "/dev/ttyACM0", -1, -1},
        {"UART-ACM-1", "/dev/ttyACM1", -1, -1},
    };

    spiList = { // Pin #19, #21, #23
        {"SPI0.0", "/dev/spidev0.0", 14, 12, 13, 10}, // Pin #24
        {"SPI0.1", "/dev/spidev0.1", 14, 12 ,13, 11}, // Pin #26
    };

    Board("odroidc4");
}
