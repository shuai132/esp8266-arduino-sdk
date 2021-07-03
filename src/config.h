#pragma once

#define SSID_RE_DEFAULT     "901"
#define PASSWORD_DEFAULT    "11223344"
#define TRY_USE_EEPROM_INFO 1

#define CONFIG_AP_SSID      "CONFIG_AP"
#define CONFIG_AP_PASSWD    PASSWORD_DEFAULT

#define TCP_PORT            1234

#define PIN_RELAY           4
#define PIN_LED             2       // 不要使用它 会影响信号!!!

#define TEST_WITH_DESKTOP_SSID      "901"
#define TEST_WITH_DESKTOP_PASSWD    "11223344"
#define TEST_WITH_DESKTOP_IP        "192.168.1.4"
#define TEST_WITH_DESKTOP_PORT      TCP_PORT
