/**************************************************************
   WiFiManager is a library for the ESP8266/Arduino platform
   (https://github.com/esp8266/Arduino) to enable easy
   configuration and reconfiguration of WiFi credentials using a Captive Portal
   inspired by:
   http://www.esp8266.com/viewtopic.php?f=29&t=2520
   https://github.com/chriscook8/esp-arduino-apboot
   https://github.com/esp8266/Arduino/tree/master/libraries/DNSServer/examples/CaptivePortalAdvanced
   Built by AlexT https://github.com/tzapu
   Licensed under MIT license
 **************************************************************/

#include "WiFiManager.h"
#include "wps_www_server.h"
#include "sn_deal.h"
#ifndef __QC
//enum sf_t {SF_AUTO=1,SF6=6, SF7, SF8, SF9, SF10, SF11, SF12 };
//extern String* wps.gw.severip(); wps.gw.severip
//extern int* get_TTN_PO();wps.gw.port
//extern void  write_ttn_bak();wps.write_ttn_bak
//extern float*  get_lat();wps.gw.lat
//extern float*  get_lon();wps.gw.lon
//extern String*  get_gw_ssid();wps.gw.ssid
//extern String*  get_gw_pass();wps.gw.pass
//extern int*  get_freqs();
//extern sf_t*  get_sf();

#endif /* __QC */
WiFiManagerParameter::WiFiManagerParameter(const char *custom) {
  _id = NULL;
  _placeholder = NULL;
  _length = 0;
  _value = NULL;

  _customHTML = custom;
}

WiFiManagerParameter::WiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length) {
  init(id, placeholder, defaultValue, length, "");
}

WiFiManagerParameter::WiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom) {
  init(id, placeholder, defaultValue, length, custom);
}

void WiFiManagerParameter::init(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom) {
  _id = id;
  _placeholder = placeholder;
  _length = length;
  _value = new char[length + 1];
  for (int i = 0; i < length + 1; i++) {
    _value[i] = 0;
  }
  if (defaultValue != NULL) {
    strncpy(_value, defaultValue, length);
  }

  _customHTML = custom;
}

WiFiManagerParameter::~WiFiManagerParameter() {
  if (_value != NULL) {
    delete[] _value;
  }
}

const char* WiFiManagerParameter::getValue() {
  return _value;
}
const char* WiFiManagerParameter::getID() {
  return _id;
}
const char* WiFiManagerParameter::getPlaceholder() {
  return _placeholder;
}
int WiFiManagerParameter::getValueLength() {
  return _length;
}
const char* WiFiManagerParameter::getCustomHTML() {
  return _customHTML;
}


WiFiManager::WiFiManager() {
    _max_params = WIFI_MANAGER_MAX_PARAMS;
	
	DEBUG_WM(F(" allocated MALLOC!!!!!"));
    _params = (WiFiManagerParameter**)malloc(_max_params * sizeof(WiFiManagerParameter*));
}

WiFiManager::~WiFiManager()
{
    if (_params != NULL)
    {
        DEBUG_WM(F("freeing allocated params!"));
        free(_params);
//		_params = NULL;
    }
}

bool WiFiManager::addParameter(WiFiManagerParameter *p) {
  if(_paramsCount + 1 > _max_params)
  {
    // rezise the params array
    _max_params += WIFI_MANAGER_MAX_PARAMS;
    DEBUG_WM(F("Increasing _max_params to:"));
    DEBUG_WM(_max_params);
    WiFiManagerParameter** new_params = (WiFiManagerParameter**)realloc(_params, _max_params * sizeof(WiFiManagerParameter*));
    if (new_params != NULL) {
      _params = new_params;
    } else {
      DEBUG_WM(F("ERROR: failed to realloc params, size not increased!"));
      return false;
    }
  }

  _params[_paramsCount] = p;
  _paramsCount++;
  DEBUG_WM(F("Adding parameter"));
  DEBUG_WM(p->getID());
  return true;
}

void WiFiManager::setupConfigPortal() {
  #ifdef __QC
  dnsServer.reset(new DNSServer());
  #endif /* __QC */
  server.reset(new ESP8266WebServer(80));

  DEBUG_WM(F(""));
  _configPortalStart = millis();

  DEBUG_WM(F("Configuring access point... "));
  DEBUG_WM(_apName);
  if (_apPassword != NULL) {
    if (strlen(_apPassword) < 8 || strlen(_apPassword) > 63) {
      // fail passphrase to short or long!
      DEBUG_WM(F("Invalid AccessPoint password. Ignoring"));
      _apPassword = NULL;
    }
    DEBUG_WM(_apPassword);
  }

  //optional soft ip config
  if (_ap_static_ip) {
  	#ifndef __QC
    DEBUG_WM(F("configAP"));yield();
	delay(500); // Without delay I've seen the IP address blank
    DEBUG_WM(F("configAP1"));yield();
	delay(500); // Without delay I've seen the IP address blank
    DEBUG_WM(F("configAP2"));yield();
	DEBUG_WM(_ap_static_ip);
  	#endif /* __QC */
    WiFi.softAPConfig(_ap_static_ip, _ap_static_gw, _ap_static_sn);
  }

  if (_apPassword != NULL) {
    WiFi.softAP(_apName, _apPassword);//password option
  } else {
    WiFi.softAP(_apName);
  }

  delay(500); // Without delay I've seen the IP address blank
  DEBUG_WM(F("AP IP address: "));
  DEBUG_WM(WiFi.softAPIP());

  /* Setup the DNS server redirecting all the domains to the apIP */
  #ifdef __QC
  dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer->start(DNS_PORT, "*", WiFi.softAPIP());
  #endif /* __QC */

  /* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
  server->on(String(F("/")), std::bind(&WiFiManager::handleRoot, this));
  server->on(String(F("/wifi")), std::bind(&WiFiManager::handleWifi, this, true));
  server->on(String(F("/0wifi")), std::bind(&WiFiManager::handleWifi, this, false));
  server->on(String(F("/wifisave")), std::bind(&WiFiManager::handleWifiSave, this));
  #ifndef __QC
  server->on(String(F("/serversave")), std::bind(&WiFiManager::handleServerSave, this));
  #endif /* __QC */
#if 1
  server->on(String(F("/i")), std::bind(&WiFiManager::handleInfo, this));
  server->on(String(F("/r")), std::bind(&WiFiManager::handleReset, this));
#endif// endif 0
  //server->on("/generate_204", std::bind(&WiFiManager::handle204, this));  //Android/Chrome OS captive portal check.
#if 0
  server->on(String(F("/fwlink")), std::bind(&WiFiManager::handleRoot, this));  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
#endif// endif 0
#if 1
  server->onNotFound (std::bind(&WiFiManager::handleNotFound, this));
#endif// endif 0
  server->begin(); // Web server start
  DEBUG_WM(F("HTTP server started"));

}

boolean WiFiManager::autoConnect() {
  String ssid = "ESP" + String(ESP.getChipId());
  return autoConnect(ssid.c_str(), NULL);
}

boolean WiFiManager::autoConnect(char const *apName, char const *apPassword) {
  DEBUG_WM(F(""));
  DEBUG_WM(F("AutoConnect"));
  Serial.print(F("goto wps456")); 

  // read eeprom for ssid and pass
  //String ssid = getSSID();
  //String pass = getPassword();

  // attempt to connect; should it fail, fall back to AP
  WiFi.mode(WIFI_STA);

  #ifdef __QC
  if (connectWifi("", "") == WL_CONNECTED)   {
    DEBUG_WM(F("IP Address:"));
    DEBUG_WM(WiFi.localIP());
    //connected
    return true;
  }
  #endif /* __QC */

  return startConfigPortal(apName, apPassword);
}

boolean WiFiManager::configPortalHasTimeout(){
#if 1
    if(_configPortalTimeout == 0 || wifi_softap_get_station_num() > 0){
      _configPortalStart = millis(); // kludge, bump configportal start time to skew timeouts
      return false;
    }
#endif// endif 0
//	DEBUG_WM(F("TIME0:"));
//	DEBUG_WM(_configPortalStart);
//	DEBUG_WM(F("TIME00:"));
//	DEBUG_WM(_configPortalTimeout);
//	DEBUG_WM(F("TIME:"));
//	DEBUG_WM(_configPortalStart + _configPortalTimeout);
//	DEBUG_WM(F("TIME1:"));
//	DEBUG_WM(millis() );
//	DEBUG_WM(F("get_statio:"));
//	DEBUG_WM(wifi_softap_get_station_num() );
    return (millis() > _configPortalStart + _configPortalTimeout);
}

boolean WiFiManager::startConfigPortal() {
  String ssid = "ESP" + String(ESP.getChipId());
  return startConfigPortal(ssid.c_str(), NULL);
}

boolean  WiFiManager::startConfigPortal(char const *apName, char const *apPassword) {
  //setup AP
  WiFi.mode(WIFI_AP_STA);
  DEBUG_WM(F("SET AP STA"));

  _apName = apName;
  _apPassword = apPassword;
  //notify we entered AP mode
  if ( _apcallback != NULL) {
    _apcallback(this);
  }

  connect = false;
  setupConfigPortal();

  while(1){
#if 1
    // check if timeout
    if(configPortalHasTimeout()) 
	{
		DEBUG_WM(F(" break."));
		break;
	}
#endif// endif 0
    //DNS
    #ifdef __QC
    dnsServer->processNextRequest();
    #endif /* __QC */
    //HTTP
    server->handleClient();


    if (connect) {
      connect = false;
      delay(2000);
      DEBUG_WM(F("Connecting to new AP"));

      // using user-provided  _ssid, _pass in place of system-stored ssid and pass
      if (connectWifi(_ssid, _pass) != WL_CONNECTED) {
        DEBUG_WM(F("Failed to connect."));
      } else {
      	#ifndef __QC
		wps.gw.ssid=_ssid;
		wps.gw.pass=_pass;
      	wps.write_ttn_bak();
      	#endif /* __QC */
        //connected
        WiFi.mode(WIFI_STA);
        //notify that configuration has changed and any optional parameters should be saved
        if ( _savecallback != NULL) {
          //todo: check if any custom parameters actually exist, and check if they really changed maybe
          _savecallback();
        }
        break;
      }

      if (_shouldBreakAfterConfig) {
        //flag set to exit after config after trying to connect
        //notify that configuration has changed and any optional parameters should be saved
        if ( _savecallback != NULL) {
          //todo: check if any custom parameters actually exist, and check if they really changed maybe
          _savecallback();
        }
        break;
      }
    }
    yield();
  }
#ifndef __QC
#if 0
    if (_params != NULL)
    {
        DEBUG_WM(F("freeing @@@@@@@@ params!"));
        free(_params);
		_params = NULL;
    }
#endif// endif 0
#endif /* __QC */
  server.reset();
  #ifdef __QC
  dnsServer.reset();
  #endif /* __QC */

  return  WiFi.status() == WL_CONNECTED;
}


int WiFiManager::connectWifi(String ssid, String pass) {
  DEBUG_WM(F("Connecting as wifi client..."));

  // check if we've got static_ip settings, if we do, use those.
  if (_sta_static_ip) {
    DEBUG_WM(F("Custom STA IP/GW/Subnet"));
    WiFi.config(_sta_static_ip, _sta_static_gw, _sta_static_sn);
    DEBUG_WM(WiFi.localIP());
  }
  //fix for auto connect racing issue
  if (WiFi.status() == WL_CONNECTED) {
    DEBUG_WM(F("Already connected. Bailing out."));
    return WL_CONNECTED;
  }
  //check if we have ssid and pass and force those, if not, try with last saved values
  if (ssid != "") {
    WiFi.begin(ssid.c_str(), pass.c_str());
  } else {
    if (WiFi.SSID()) {
      DEBUG_WM(F("Using last saved values, should be faster"));
      //trying to fix connection in progress hanging
      ETS_UART_INTR_DISABLE();
      wifi_station_disconnect();
      ETS_UART_INTR_ENABLE();

      WiFi.begin();
    } else {
      DEBUG_WM(F("No saved credentials"));
    }
  }

  int connRes = waitForConnectResult();
  DEBUG_WM ("Connection result: ");
  DEBUG_WM ( connRes );
  //not connected, WPS enabled, no pass - first attempt
  if (_tryWPS && connRes != WL_CONNECTED && pass == "") {
    startWPS();
    //should be connected at the end of WPS
    connRes = waitForConnectResult();
  }
  return connRes;
}

uint8_t WiFiManager::waitForConnectResult() {
  if (_connectTimeout == 0) {
    return WiFi.waitForConnectResult();
  } else {
    DEBUG_WM (F("Waiting for connection result with time out"));
    unsigned long start = millis();
    boolean keepConnecting = true;
    uint8_t status;
    while (keepConnecting) {
      status = WiFi.status();
      if (millis() > start + _connectTimeout) {
        keepConnecting = false;
        DEBUG_WM (F("Connection timed out"));
      }
      if (status == WL_CONNECTED || status == WL_CONNECT_FAILED) {
        keepConnecting = false;
      }
      delay(100);
    }
    return status;
  }
}

void WiFiManager::startWPS() {
  DEBUG_WM(F("START WPS"));
  WiFi.beginWPSConfig();
  DEBUG_WM(F("END WPS"));
}
/*
  String WiFiManager::getSSID() {
  if (_ssid == "") {
    DEBUG_WM(F("Reading SSID"));
    _ssid = WiFi.SSID();
    DEBUG_WM(F("SSID: "));
    DEBUG_WM(_ssid);
  }
  return _ssid;
  }

  String WiFiManager::getPassword() {
  if (_pass == "") {
    DEBUG_WM(F("Reading Password"));
    _pass = WiFi.psk();
    DEBUG_WM("Password: " + _pass);
    //DEBUG_WM(_pass);
  }
  return _pass;
  }
*/
String WiFiManager::getConfigPortalSSID() {
  return _apName;
}

void WiFiManager::resetSettings() {
  DEBUG_WM(F("settings invalidated"));
  DEBUG_WM(F("THIS MAY CAUSE AP NOT TO START UP PROPERLY. YOU NEED TO COMMENT IT OUT AFTER ERASING THE DATA."));
  yield();
  WiFi.disconnect(true);
  yield();
  #ifndef __QC
  delay(200);
  yield();
  #endif /* __QC */
  //delay(200);
}
void WiFiManager::setTimeout(unsigned long seconds) {
  setConfigPortalTimeout(seconds);
}

void WiFiManager::setConfigPortalTimeout(unsigned long seconds) {
  _configPortalTimeout = seconds * 1000;

  
  DEBUG_WM(_configPortalTimeout);
}

void WiFiManager::setConnectTimeout(unsigned long seconds) {
  _connectTimeout = seconds * 1000;
}

void WiFiManager::setDebugOutput(boolean debug) {
  _debug = debug;
}

void WiFiManager::setAPStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn) {
  _ap_static_ip = ip;
  _ap_static_gw = gw;
  _ap_static_sn = sn;
}

void WiFiManager::setSTAStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn) {
  _sta_static_ip = ip;
  _sta_static_gw = gw;
  _sta_static_sn = sn;
}

void WiFiManager::setMinimumSignalQuality(int quality) {
  _minimumQuality = quality;
}

void WiFiManager::setBreakAfterConfig(boolean shouldBreak) {
  _shouldBreakAfterConfig = shouldBreak;
}

/** Handle root or redirect to captive portal */
void WiFiManager::handleRoot() {
  DEBUG_WM(F("Handle root"));
#if 0
  if (captivePortal()) { // If caprive portal redirect instead of displaying the page.
	  DEBUG_WM(F("Handle root2"));
    return;
  }
#endif// endif 0
  DEBUG_WM(F("Handle root1"));
  String page = FPSTR(HTTP_HEAD);
  page.replace("{v}", "Options");
//  page.replace("{v}", "Options");
//  page.replace("{v}", "QC");//实在不知道为什么 就这个头不能改
  page += FPSTR(HTTP_SCRIPT);
  page += FPSTR(HTTP_STYLE);
  page += _customHeadElement;
  page += FPSTR(HTTP_HEAD_END);
  page += String(F("<h1>"));
  #ifdef __QC
  page += _apName;
  #endif /* __QC */
  page += HTTP_TILE;
  page += String(F("</h1>"));
  #ifdef __QC
  page += String(F("<h3>WiFiManager</h3>"));
  #endif /* __QC */
  page += FPSTR(HTTP_PORTAL_OPTIONS);
  #ifndef __QC
  page += FPSTR(HTTP_INFO);
  #endif /* __QC */
  page += FPSTR(HTTP_END);

  server->sendHeader("Content-Length", String(page.length()));
  server->send(200, "text/html", page);

}

/** Wifi config page handler */
void WiFiManager::handleWifi(boolean scan) {

  String page = FPSTR(HTTP_HEAD);
//  page.replace("{v}", "Config ESP");
  page.replace("{v}", "配置");
  page += FPSTR(HTTP_SCRIPT);
  page += FPSTR(HTTP_STYLE);
  page += _customHeadElement;
  page += FPSTR(HTTP_HEAD_END);

  if (scan) {
    int n = WiFi.scanNetworks();
    DEBUG_WM(F("Scan done"));
    if (n == 0) {
      DEBUG_WM(F("No networks found"));
      page += F("No networks found. Refresh to scan again.");
    } else {

      //sort networks
      int indices[n];
      for (int i = 0; i < n; i++) {
        indices[i] = i;
      }

      // RSSI SORT

      // old sort
      for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
          if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
            std::swap(indices[i], indices[j]);
          }
        }
      }

      /*std::sort(indices, indices + n, [](const int & a, const int & b) -> bool
        {
        return WiFi.RSSI(a) > WiFi.RSSI(b);
        });*/

      // remove duplicates ( must be RSSI sorted )
      if (_removeDuplicateAPs) {
        String cssid;
        for (int i = 0; i < n; i++) {
          if (indices[i] == -1) continue;
          cssid = WiFi.SSID(indices[i]);
          for (int j = i + 1; j < n; j++) {
            if (cssid == WiFi.SSID(indices[j])) {
              DEBUG_WM("DUP AP: " + WiFi.SSID(indices[j]));
              indices[j] = -1; // set dup aps to index -1
            }
          }
        }
      }

      //display networks in page
      for (int i = 0; i < n; i++) {
        if (indices[i] == -1) continue; // skip dups
        DEBUG_WM(WiFi.SSID(indices[i]));
        DEBUG_WM(WiFi.RSSI(indices[i]));
        int quality = getRSSIasQuality(WiFi.RSSI(indices[i]));

        if (_minimumQuality == -1 || _minimumQuality < quality) {
          String item = FPSTR(HTTP_ITEM);
          String rssiQ;
          rssiQ += quality;
          item.replace("{v}", WiFi.SSID(indices[i]));
          item.replace("{r}", rssiQ);
          if (WiFi.encryptionType(indices[i]) != ENC_TYPE_NONE) {
            item.replace("{i}", "l");
          } else {
            item.replace("{i}", "");
          }
          //DEBUG_WM(item);
          page += item;
          delay(0);
        } else {
          DEBUG_WM(F("Skipping due to quality"));
        }

      }
      page += "<br/>";
    }
  }

  page += FPSTR(HTTP_FORM_START);
  char parLength[5];
  // add the extra parameters to the form
  for (int i = 0; i < _paramsCount; i++) {
    if (_params[i] == NULL) {
      break;
    }

    String pitem = FPSTR(HTTP_FORM_PARAM);
    if (_params[i]->getID() != NULL) {
      pitem.replace("{i}", _params[i]->getID());
      pitem.replace("{n}", _params[i]->getID());
      pitem.replace("{p}", _params[i]->getPlaceholder());
      snprintf(parLength, 5, "%d", _params[i]->getValueLength());
      pitem.replace("{l}", parLength);
      pitem.replace("{v}", _params[i]->getValue());
      pitem.replace("{c}", _params[i]->getCustomHTML());
    } else {
      pitem = _params[i]->getCustomHTML();
    }

    page += pitem;
  }
  if (_params[0] != NULL) {
    page += "<br/>";
  }

  if (_sta_static_ip) {

    String item = FPSTR(HTTP_FORM_PARAM);
    item.replace("{i}", "ip");
    item.replace("{n}", "ip");
    item.replace("{p}", "Static IP");
    item.replace("{l}", "15");
    item.replace("{v}", _sta_static_ip.toString());

    page += item;

    item = FPSTR(HTTP_FORM_PARAM);
    item.replace("{i}", "gw");
    item.replace("{n}", "gw");
    item.replace("{p}", "Static Gateway");
    item.replace("{l}", "15");
    item.replace("{v}", _sta_static_gw.toString());

    page += item;

    item = FPSTR(HTTP_FORM_PARAM);
    item.replace("{i}", "sn");
    item.replace("{n}", "sn");
    item.replace("{p}", "Subnet");
    item.replace("{l}", "15");
    item.replace("{v}", _sta_static_sn.toString());

    page += item;

    page += "<br/>";
  }

  page += FPSTR(HTTP_FORM_END);
  page += FPSTR(HTTP_SCAN_LINK);

  page += FPSTR(HTTP_END);

  server->sendHeader("Content-Length", String(page.length()));
  server->send(200, "text/html", page);


  DEBUG_WM(F("Sent config page"));
}

/** Handle the WLAN save form and redirect to WLAN config page again */
void WiFiManager::handleWifiSave() {
  DEBUG_WM(F("WiFi save"));

  //SAVE/connect here
  _ssid = server->arg("s").c_str();
  _pass = server->arg("p").c_str();

  //parameters
  for (int i = 0; i < _paramsCount; i++) {
    if (_params[i] == NULL) {
      break;
    }
    //read parameter
    String value = server->arg(_params[i]->getID()).c_str();
    //store it in array
    value.toCharArray(_params[i]->_value, _params[i]->_length + 1);
    DEBUG_WM(F("Parameter"));
    DEBUG_WM(_params[i]->getID());
    DEBUG_WM(value);
  }

  if (server->arg("ip") != "") {
    DEBUG_WM(F("static ip"));
    DEBUG_WM(server->arg("ip"));
    //_sta_static_ip.fromString(server->arg("ip"));
    String ip = server->arg("ip");
    optionalIPFromString(&_sta_static_ip, ip.c_str());
  }
  if (server->arg("gw") != "") {
    DEBUG_WM(F("static gateway"));
    DEBUG_WM(server->arg("gw"));
    String gw = server->arg("gw");
    optionalIPFromString(&_sta_static_gw, gw.c_str());
  }
  if (server->arg("sn") != "") {
    DEBUG_WM(F("static netmask"));
    DEBUG_WM(server->arg("sn"));
    String sn = server->arg("sn");
    optionalIPFromString(&_sta_static_sn, sn.c_str());
  }

  String page = FPSTR(HTTP_HEAD);
//  page.replace("{v}", "Credentials Saved");
  page.replace("{v}", "保存成功");
  page += FPSTR(HTTP_SCRIPT);
  page += FPSTR(HTTP_STYLE);
  page += _customHeadElement;
  page += FPSTR(HTTP_HEAD_END);
  page += FPSTR(HTTP_SAVED);
  page += FPSTR(HTTP_END);

  server->sendHeader("Content-Length", String(page.length()));
  server->send(200, "text/html", page);

  DEBUG_WM(F("Sent wifi save page"));

  connect = true; //signal ready to connect/reset
}

#ifndef __QC
/** Handle the WLAN save form and redirect to WLAN config page again */
void WiFiManager::handleServerSave() {

  //SAVE/connect here
	bool need_bak = false;
	DEBUG_WM(F("save ok 1"));
	String ssid = server->arg("ssid").c_str();
	String pass = server->arg("pass").c_str();
	String ip = server->arg("ip").c_str();
	String po = server->arg("po").c_str();
	
	String lora_tx_freq = server->arg("txfreq").c_str();
	String mifi_flg = server->arg("mifi").c_str();
//	String lat = server->arg("lat").c_str();
//	String lon = server->arg("lng").c_str();
	
	String freq = server->arg("freq").c_str();
	String sf = server->arg("sf").c_str();
	
	if(ssid!="")
	{
		wps.gw.ssid = ssid;
		need_bak =true;
	}
	if(pass!="")
	{
	    wps.gw.pass = pass;
		need_bak =true;
	}
	if(ip!="")
	{
	    wps.gw.severip = ip;
		need_bak =true;
	}
	if(po!=0)
	{
		wps.gw.port = atoi(po.c_str());	
		need_bak =true;
	}
	if(lora_tx_freq!=0)
	{
		wps.gw.lora_tx_freq = atoi(lora_tx_freq.c_str());	
		need_bak =true;
	}
	if(mifi_flg!=0)
	{
	    wps.gw.mifi_flg = atof(mifi_flg.c_str());   
		need_bak =true;
	}

	if(freq!=0)
	{
	    wps.gw.lora_freq = atoi(freq.c_str());
		need_bak =true;
	}
	
	if(sf!=0)
	{
	    wps.gw.lora_sf = (sf_t)atoi(sf.c_str());
		if(wps.gw.lora_sf!=SF_AUTO&& (wps.gw.lora_sf<7||wps.gw.lora_sf>12))
		{
		    wps.gw.lora_sf=SF_AUTO;
		}
		need_bak =true;
	}
	if(need_bak)
	{
		DEBUG_WM(F("write_ttn_bak ok"));
		Serial.print(F("write_bak ok"));
		wps.write_ttn_bak();
	}
	WiFiManager::handleInfo();

}

void getaway_id_port(String &page)
{
    page += FPSTR(HTTP_FORM_SERVER);
    page += FPSTR(HTTP_FORM_SAVE);
    page += FPSTR(HTTP_FORM_EXIT);
	page += FPSTR(HTTP_INFO);
}


#endif /* __QC */
/** Handle the info page */
void WiFiManager::handleInfo() {
  DEBUG_WM(F("Info"));

  String page = FPSTR(HTTP_HEAD);
//  page.replace("{v}", "Info");
  page.replace("{v}", "信息");
  page += FPSTR(HTTP_SCRIPT);
  page += FPSTR(HTTP_STYLE);
  page += _customHeadElement;
  page += FPSTR(HTTP_HEAD_END);
  
  #ifndef __QC
  page += F("<dl>");
  page += F("<dt>网关ID</dt><dd>");
//  page += F("cccc");
  {
//	  uint8_t MAC_array[6];
//      WiFi.macAddress(MAC_array);
	  uint8_t pullDataReq[8];								  
	  sn.read(&pullDataReq[0]);
      page += String(pullDataReq[0], HEX);
      page += String(pullDataReq[1], HEX);
      page += String(pullDataReq[2], HEX);
      page += String(pullDataReq[3], HEX);
      page += String(pullDataReq[4], HEX);
      page += String(pullDataReq[5], HEX);
      page += String(pullDataReq[6], HEX);
	  page += String(pullDataReq[7], HEX);
  }
  page += F("</dd>");
  
    page += F("<dt>WIFI账户</dt><dd>");
  {
	  String TTN_ssid = wps.gw.ssid;
      page += TTN_ssid.c_str();
  }

  page += F("</dd>");
  page += F("<dt>WIFI密码</dt><dd>");
  {
	  String TTN_pass = wps.gw.pass;
      page += TTN_pass.c_str();
  }

  page += F("</dd>");
  
  page += F("<dt>服务地址</dt><dd>");
  {
	  String TTN_IP = wps.gw.severip;
      page += TTN_IP.c_str();
  }

  page += F("</dd>");
  page += F("<dt>服务端口</dt><dd>");
	{
		char c[8]; 
		sprintf(c, "%d", wps.gw.port); 
		String sever_po = c;
		page += sever_po;
	}
  page += F("</dd>");
  page += F("</dd>");
  page += F("<dt>单wifi网络开关</dt><dd>");
	{
//		char c[8]; 
//		sprintf(c, "%f",wps.gw.mifi_flg); 
//		String mifi_flg = c;
//		page += "server配置";
		if(wps.gw.mifi_flg)
		{
		    page += "单wifi模式";
		}
		else
		{
			page += "4G WIFI网络模式";
		}
	}
  page += F("</dd>");
  page += F("</dd>");
  page += F("<dt>网关发射频点</dt><dd>");
	{
		char c[8]; 
		sprintf(c, "%ld",wps.gw.lora_tx_freq); 
		String sever_txfreq = c;
		if(wps.gw.lora_tx_freq==0)
		{
			page += "server配置";
		}
		else
		{
			page += sever_txfreq;
		}
  	}
  page += F("</dd>");
  page += F("</dd>");
  page += F("</dd>");
  page += F("<dt>网关接收频点</dt><dd>");
	{
		char c[8]; 
		sprintf(c, "%ld",wps.gw.lora_freq); 
		String sever_freq = c;
		page += sever_freq;
	}
  page += F("</dd>");
  page += F("</dd>");
  page += F("<dt>扩频因子</dt><dd>");
	{
		char c[8]; 
		sprintf(c, "%d",(int)wps.gw.lora_sf); 
		String sever_sf = c;
#if 0
		if(wps.gw.lora_sf==SF_AUTO)
		{
		    page += "自动模式";
		}
		else
#endif// endif 0
		{
		    page += "SF";
    		page += sever_sf;
		}
	}
  page += F("</dd>");
  
  page += F("<dl>");
  page += F("<dl>");
  page += F("<dl>");
  getaway_id_port(page);
//  page += MAC_array[0];
//  page += MAC_array[1];
//  page += MAC_array[2];
//  page += MAC_array[3];
//  page += MAC_array[4];
//  page += MAC_array[5];
  page += F("</dd>");
  #endif /* __QC */
  #if 0
  page += F("<dl>");
  page += F("<dt>Chip ID</dt><dd>");
  page += ESP.getChipId();
  page += F("</dd>");
  page += F("<dt>Flash Chip ID</dt><dd>");
  page += ESP.getFlashChipId();
  page += F("</dd>");
  page += F("<dt>IDE Flash Size</dt><dd>");
  page += ESP.getFlashChipSize();
  page += F(" bytes</dd>");
  page += F("<dt>Real Flash Size</dt><dd>");
  page += ESP.getFlashChipRealSize();
  page += F(" bytes</dd>");
  page += F("<dt>Soft AP IP</dt><dd>");
  page += WiFi.softAPIP().toString();
  page += F("</dd>");
  page += F("<dt>Soft AP MAC</dt><dd>");
  page += WiFi.softAPmacAddress();
  page += F("</dd>");
  page += F("<dt>Station MAC</dt><dd>");
  page += WiFi.macAddress();
  page += F("</dd>");
  page += F("</dl>");
#endif// endif 0
  page += FPSTR(HTTP_END);

  server->sendHeader("Content-Length", String(page.length()));
  server->send(200, "text/html", page);

  DEBUG_WM(F("Sent info page"));
}

/** Handle the reset page */
void WiFiManager::handleReset() {
  DEBUG_WM(F("Reset"));

  String page = FPSTR(HTTP_HEAD);
//  page.replace("{v}", "Info");
  page.replace("{v}", "重启");
  page += FPSTR(HTTP_SCRIPT);
  page += FPSTR(HTTP_STYLE);
  page += _customHeadElement;
  page += FPSTR(HTTP_HEAD_END);
  page += F("网关即将重启.");
  page += FPSTR(HTTP_END);

  server->sendHeader("Content-Length", String(page.length()));
  server->send(200, "text/html", page);

  DEBUG_WM(F("Sent reset page"));
  delay(5000);
  while(1);
//  ESP.reset();
  delay(2000);
}

void WiFiManager::handleNotFound() {
#if 0
  if (captivePortal()) { // If captive portal redirect instead of displaying the error page.
    return;
  }
#endif// endif 0
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server->uri();
  message += "\nMethod: ";
  message += ( server->method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server->args();
  message += "\n";

  for ( uint8_t i = 0; i < server->args(); i++ ) {
    message += " " + server->argName ( i ) + ": " + server->arg ( i ) + "\n";
  }
  server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server->sendHeader("Pragma", "no-cache");
  server->sendHeader("Expires", "-1");
  server->sendHeader("Content-Length", String(message.length()));
  server->send ( 404, "text/plain", message );
}


/** Redirect to captive portal if we got a request for another domain. Return true in that case so the page handler do not try to handle the request again. */
boolean WiFiManager::captivePortal() {
  if (!isIp(server->hostHeader()) ) {
    DEBUG_WM(F("Request redirected to captive portal"));
    server->sendHeader("Location", String("http://") + toStringIp(server->client().localIP()), true);
    server->send ( 302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server->client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}

//start up config portal callback
void WiFiManager::setAPCallback( void (*func)(WiFiManager* myWiFiManager) ) {
  _apcallback = func;
}

//start up save config callback
void WiFiManager::setSaveConfigCallback( void (*func)(void) ) {
  _savecallback = func;
}

//sets a custom element to add to head, like a new style tag
void WiFiManager::setCustomHeadElement(const char* element) {
  _customHeadElement = element;
}

//if this is true, remove duplicated Access Points - defaut true
void WiFiManager::setRemoveDuplicateAPs(boolean removeDuplicates) {
  _removeDuplicateAPs = removeDuplicates;
}



template <typename Generic>
void WiFiManager::DEBUG_WM(Generic text) {
  Serial.println(text);

  if (_debug) {
//    Serial.print("*WM: ");
//    Serial.println(text);
  }
}

int WiFiManager::getRSSIasQuality(int RSSI) {
  int quality = 0;

  if (RSSI <= -100) {
    quality = 0;
  } else if (RSSI >= -50) {
    quality = 100;
  } else {
    quality = 2 * (RSSI + 100);
  }
  return quality;
}

/** Is this an IP? */
boolean WiFiManager::isIp(String str) {
  for (size_t i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

/** IP to String? */
String WiFiManager::toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}
