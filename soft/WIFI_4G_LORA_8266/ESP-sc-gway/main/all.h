//#include "ESP-sc-gway.h"					    
//#include <Esp.h>
//#include <string.h>
//#include <stdio.h>
//#include <sys/types.h>
//#include <unistd.h>
//#include <fcntl.h>
//#include <cstdlib>
//#include <sys/time.h>
//#include <cstring>
//#include <string>							

//#include <TimeLib.h>						
//#include <DNSServer.h>						
//#include <ArduinoJson.h>

//#include <ESP8266WiFi.h>					
//						
//#include "FS.h"
//#include <WiFiUdp.h>
//#include <pins_arduino.h>

//#include <gBase64.h>						
//#include <ESP8266mDNS.h>

#include <WiFiManager.h>						
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <spi_flash.h>
#include <SPI.h>
#include <lora_1278.h>
#include "lora_server.h"
#include "wps_www_server.h"
#include <ESP8266WiFi.h>
#include "qc_telnet.h"
#include "led_show.h"
#include "AT_4G.h"
#include "sn_deal.h"
#include "ota_fw.h"
extern "C" {
#include "user_interface.h"
#include "lwip/err.h"
#include "lwip/dns.h"
#include "c_types.h"
}
#define USE_SERIAL Serial
#if !defined(CFG_noassert)
#define ASSERT(cond)
#else
#define ASSERT(cond) /**/
#endif

