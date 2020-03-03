#include "ota_fw.h"
#define USE_SERIAL telnet//Serial
void OTA_CLASS::query_fw()
{
    once = false;
}

/*************************************************************************************************
 * 功 能 : OTA升级
 * 说 明 : 对接服务器，注意版本问题
 *************************************************************************************************/
void OTA_CLASS::update_fw()
{    
	char cmd[128]={0};
	static int day_bak;
	if (day() != day_bak) //每日查询是否需要进行系统升级
	{
		once = false;
		lora.begin(wps.gw.lora_freq,wps.gw.lora_sf);//重启下lora
		day_bak = day();
	}
	if(lorawan.wifi_ok && once==false)
    {
    	once = true;
		HTTPClient http;
		snprintf(cmd,128, "http://zenghi.com/fw/"VERSION_BIN); 
		http.begin( cmd );
//		USE_SERIAL.println( cmd );
		int httpCode = http.GET();
		http.end();

		if (httpCode != HTTP_CODE_OK) //全局升级文件不存在 找针对设备升级文件
		{
//			USE_SERIAL.println("httpCode < 0");
			uint8_t sn_num[8] = {0};
			sn.read(sn_num);
			snprintf(cmd,128, "http://zenghi.com/fw/%02x%02x%02x%02x%02x%02x%02x%02x%s",
				sn_num[0],sn_num[1],sn_num[2],sn_num[3],sn_num[4],sn_num[5],sn_num[6],sn_num[7],VERSION_BIN); 
//			USE_SERIAL.println(cmd );
			http.begin( cmd );
			httpCode = http.GET();
			http.end();
		}

		if (httpCode == HTTP_CODE_OK) 
	   	{
	   		if(httpCode == HTTP_CODE_OK)
			{
				USE_SERIAL.println("have up fw");
				led.led_upfw_tick();
			}
			if (httpCode == HTTP_CODE_OK) 
			{
				t_httpUpdate_return ret = ESPhttpUpdate.update( cmd ); 
				switch(ret) 
				{
					case HTTP_UPDATE_FAILED:
					USE_SERIAL.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
					break;
				
					case HTTP_UPDATE_NO_UPDATES:
					USE_SERIAL.println("HTTP_UPDATE_NO_UPDATES");
					break;
				
					case HTTP_UPDATE_OK:
					USE_SERIAL.println("HTTP_UPDATE_OK");
					break;
				}
			}
			else 
			{
			  USE_SERIAL.printf("[HTTP] GET... failed, error: %s\r\n", http.errorToString(httpCode).c_str());
			}

			led.exit_led_uptick();
		}
		else
		{
			USE_SERIAL.println("no have up fw");
		}
	}
}


OTA_CLASS OTA;

