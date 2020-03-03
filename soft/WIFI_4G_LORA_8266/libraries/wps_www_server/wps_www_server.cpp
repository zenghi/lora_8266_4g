#include "wps_www_server.h"
#include "led_show.h"
//#include "lora_server.h"
//#include "lora_1278.h"
#define USE_SERIAL Serial


int wps_server::writeConfig(const char *fn, struct gw_config *c) 
{
	if (!SPIFFS.exists(fn)) {
		USE_SERIAL.println("WARNING:: writeConfig, file not exists, formatting ");
//		SPIFFS.format();
		USE_SERIAL.println("no save");
	}
	File f = SPIFFS.open(fn, "w");
	if (!f) {
		USE_SERIAL.println("ERROR:: writeConfig, open file=");
		USE_SERIAL.println(fn);
		USE_SERIAL.println();
		USE_SERIAL.println("save  error");
		return(-1);
	}
	
	f.print("SSID"); f.print('='); f.print((*c).ssid); f.print('\n'); 
	f.print("PASS"); f.print('='); f.print((*c).pass); f.print('\n');
	f.print("SEVERIP");  f.print('='); f.print((*c).severip); f.print('\n');
	f.print("PORT");  f.print('='); f.print((*c).port); f.print('\n');
	f.print("LAT");  f.print('='); f.print((*c).lat); f.print('\n');
	f.print("LON");  f.print('='); f.print((*c).lon); f.print('\n');
	f.print("MYSF");  f.print('='); f.print((*c).lora_sf); f.print('\n');
	f.print("MYFREQ");  f.print('='); f.print((*c).lora_freq); f.print('\n');
	f.print("TX_DEALY");  f.print('='); f.print((*c).tx_dealy); f.print('\n');
	f.print("TX_FREQ");  f.print('='); f.print((*c).lora_tx_freq); f.print('\n');
	f.print("MIFI_FLG");  f.print('='); f.print((*c).mifi_flg); f.print('\n');
	f.close();
	
	USE_SERIAL.println("save  ok");
	return(1);
}
int wps_server::readConfig(const char *fn, struct gw_config *c) {
	int tries = 0;
	USE_SERIAL.printf("readgw config\r\n");
	if (!SPIFFS.exists(fn)) {
		USE_SERIAL.println(F("ERROR:: readConfig="));
//		SPIFFS.format();
		USE_SERIAL.println("no  save");
		File f = SPIFFS.open(fn, "w");
		if (!f) {
			USE_SERIAL.println("save  error");
			SPIFFS.format();
			return(-1);
		}
		writeConfig(CONFIGFILE, &gw);
		f.close();
		return(-1);
	}

	File f = SPIFFS.open(fn, "r");
	if (!f) {
		USE_SERIAL.println("save  error");
		return(-1);
	}

	while (f.available()) {
		
		if (tries >= 10) {
			f.close();
			DBG_PRINTF(F("Formatting"));
//			SPIFFS.format();
			f = SPIFFS.open(fn, "r");
			tries = 0;
		}
		
		String id =f.readStringUntil('=');						// C++ thing
		String val=f.readStringUntil('\n');
		if (id == "SSID") {										// WiFi SSID
			DBG_PRINTF(F("SSID=")); DBG_PRINTF(val);
			(*c).ssid = val;									// val contains ssid, we do NO check
		}
		else if (id == "PASS") { 								// WiFi Password
			DBG_PRINTF(F("PASS=")); DBG_PRINTF(val); 
			(*c).pass = val;
		}
		else if (id == "SEVERIP") {								// FILEREC setting
			DBG_PRINTF(F("SEVERIP=")); DBG_PRINTF(val);
			(*c).severip = val;									// val contains ssid, we do NO check
		}
		else if (id == "PORT") {								// FILEREC setting
			DBG_PRINTF(F("PORT=")); DBG_PRINTF(val);
			(*c).port = (int) val.toInt();
		}
		else if (id == "LAT") {								// FILEREC setting
			DBG_PRINTF(F("LAT=")); DBG_PRINTF(val);
			(*c).lat = (float) val.toFloat();
		DBG_PRINTF((*c).lat);
		}
		else if (id == "LON") {								// FILEREC setting
			DBG_PRINTF(F("LON=")); DBG_PRINTF(val);
			(*c).lon = (float) val.toFloat();
		
			DBG_PRINTF("123@@@");
			DBG_PRINTF((*c).lon);
		}
		else if (id == "MYSF") {								// FILEREC setting
			DBG_PRINTF(F("MYSF=")); DBG_PRINTF(val);
			(*c).lora_sf = (sf_t) val.toInt();
		}
		else if (id == "MYFREQ") {								// FILEREC setting
			DBG_PRINTF(F("MYFREQ=")); DBG_PRINTF(val);
			(*c).lora_freq = (int) val.toInt();
		}
		else if (id == "TX_DEALY") {								// FILEREC setting
			DBG_PRINTF(F("TX_DEALY=")); DBG_PRINTF(val);
			(*c).tx_dealy = (int) val.toInt();
		}
		else if (id == "TX_FREQ") {								// FILEREC setting
			DBG_PRINTF(F("TX_FREQ=")); DBG_PRINTF(val);
			(*c).lora_tx_freq = (int) val.toInt();
		}
		else if (id == "MIFI_FLG") {								// FILEREC setting
			DBG_PRINTF(F("MIFI_FLG=")); DBG_PRINTF(val);
			(*c).mifi_flg = (int) val.toInt();
		}
		else {
			tries++;
		}
	}
	f.close();
	return(1);
}

int wps_server::read_gw() 
{
	if (SPIFFS.begin()) 
	{
		USE_SERIAL.printf("iffs ok\r\n");
		Dir dir = SPIFFS.openDir("/data");
		  while (dir.next()) {
			USE_SERIAL.print(dir.fileName());
			USE_SERIAL.print(" - ");
			USE_SERIAL.println(dir.fileSize());
		  }
	}
	else 
	{
		USE_SERIAL.printf("iffs init error\r\n");
	}
#if 1
	readConfig( CONFIGFILE, &gw);
#endif// endif 0
}

void  wps_server::write_ttn_bak()
{
#if 1
	USE_SERIAL.println(F(" "));
	USE_SERIAL.print(F("ssid: ")); 
	USE_SERIAL.print(gw.ssid);
	USE_SERIAL.print(F(" pass")); 
	USE_SERIAL.print(gw.pass);
	USE_SERIAL.println(F(" "));
	USE_SERIAL.print(F("severip: ")); 
	USE_SERIAL.print(gw.severip);
	USE_SERIAL.print(F(" severport")); 
	USE_SERIAL.print(gw.port);
	USE_SERIAL.println(F(" "));
#endif// endif 0
	writeConfig(CONFIGFILE, &gw);
}

void wps_server::goto_wps()
{
	WiFiManager wifiManager;
	
//	pinMode(led_pins.led1, OUTPUT);
//	ticker.attach(0.3, tick);

	USE_SERIAL.print(F("goto wps123")); 
	wifiManager.setConfigPortalTimeout(120);
	wifiManager.resetSettings();
	wifiManager.setAPStaticIPConfig(IPAddress(192, 168, 1, 1), IPAddress(0, 0, 0, 0), IPAddress(0, 0, 0, 0));
	wifiManager.autoConnect(AP_NAME, AP_PASSWD );

	struct station_config sta_conf;
	wifi_station_get_config(&sta_conf);
	
//	ticker.detach();
//	digitalWrite(led_pins.led1, 0);	  // set pin to the opposite state
}
bool wps_server::deal_goto_wps()
{
	static int count = 0;
	if(digitalRead(0)==0)
	{
		USE_SERIAL.println("goto wps count++"); 
		count++;
//		detachInterrupt(0);
//		delay(1000);
		if(count>20)
		{
		    USE_SERIAL.println("goto wps"); 
			led.led_wps_tick();
    		goto_wps();
			led.exit_wps_tick();
    		USE_SERIAL.println("wps exit"); 
		}
		return true;
	}
	else
	{
	 	if(count)   
		{
		    count--;
		}
	}
	return false;
}
wps_server wps;

