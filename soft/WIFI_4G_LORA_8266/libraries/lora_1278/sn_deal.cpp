/******************************************************************************

 				 版权所有 (C), 2017-2027, 福州晴川科技有限公司

 ******************************************************************************
  文 件 名   : sn_deal.cpp
  版 本 号   : 初稿
  作    者   : zenghi
  联系方式 : 309966755@qq.com
  生成日期   : 2019-04-24
  功能描述   : 写入SN设备码
******************************************************************************/
#include "sn_deal.h"
#define USE_SERIAL Serial

int sn_class::CharToInt(char hex) 
{
    if (hex>='0' && hex <='9')
        return hex - '0';
    if (hex>='A' && hex <= 'F')
        return hex-'A'+10;
    if(hex>='a' && hex <= 'f')
        return hex-'a'+10;
    return -1;
}

int sn_class::StringToInt(char *hex)
{
    return CharToInt(hex[0]) * 16 + CharToInt(hex[1]);
}

void sn_class::read_rx_data()
{
	char c;
	bool in_flag = false;   
	HardwareSerial *hs_rx= &USE_SERIAL;	
	while (hs_rx->available() > 0) 
	{
		c = char(hs_rx->read());
		if (in_flag == false) 
		{
			response = c;
			in_flag = true;
		}
		else 
		{
			response += c;
		}
		delay(1);
	}	

}

int sn_class::writesn( ) 
{
	if (!SPIFFS.exists("/sn.txt")) {
		USE_SERIAL.println("no save");
	}
	File f = SPIFFS.open("/sn.txt", "w");
	if (!f) {
		USE_SERIAL.println("ERROR:: ERROR:: ERROR:: ERROR:: ERROR:: ERROR:: ");
		USE_SERIAL.println("/qc.txt");
		return(-1);
	}
	f.write(sn_num, 8);
	f.close();
	
	USE_SERIAL.println("save sn ok");
	return(1);
}
int sn_class::readsn( ) 
{
	if (SPIFFS.begin()) 
	{
		USE_SERIAL.printf("SPIFFS init success");
	}
	else 
	{
		USE_SERIAL.printf("SPIFFS init error");
	}
	
	if (!SPIFFS.exists("/sn.txt")) {
		USE_SERIAL.println("no save sn");
		return(-1);
	}
	File f = SPIFFS.open("/sn.txt", "r");
	if (!f) {
		USE_SERIAL.println("ERROR:: ERROR:: ERROR:: ERROR:: ERROR:: ERROR:: ");
		USE_SERIAL.println("/qc.txt");
		return(-1);
	}
	f.write(sn_num, 8);
	f.close();
	
	USE_SERIAL.println("save sn ok");
	return(1);
}

bool sn_class::have_mac()
{
    
	if(
	  (sn_num[0]==0xff
	 &&sn_num[1]==0xff
	 &&sn_num[2]==0xff	 
	 &&sn_num[3]==0xff	
	 &&sn_num[4]==0xff
 	 &&sn_num[5]==0xff
 	 &&sn_num[6]==0xff
 	 &&sn_num[7]==0xff)
 	 ||
 	  (sn_num[0]==0
	 &&sn_num[1]==0
	 &&sn_num[2]==0	 
	 &&sn_num[3]==0	
	 &&sn_num[4]==0
 	 &&sn_num[5]==0
	 &&sn_num[6]==0
 	 &&sn_num[7]==0)
	)
	{
	    return false;
	}
	else
	{
		if(sn_num[0]==0xee&&sn_num[1]==0xee)
		{
		    return true;
		}
	}
	return false;
}

/*************************************************************************************************
 * 功 能 : 双备份区域 默认开头为eeee 标志
 *************************************************************************************************/
void sn_class::start()
{
	char *data;
	EEPROM.begin(512);
	EEPROM.get(0, sn_num);
	EEPROM.end();
	if(!have_mac())
	{
		readsn();
		if(!have_mac())
		{
			USE_SERIAL.println("get uart write sn");
			led.led_upfw_tick();
		    while(1)
	    	{
	    		ESP.wdtFeed();
				read_rx_data();
				data = strstr(response.c_str(), "QCADDR");
	        	if(data!=NULL)
	        	{
					sn_num[0] = StringToInt(&data[6]);
					sn_num[1] = StringToInt(&data[8]);
					sn_num[2] = StringToInt(&data[10]);
					sn_num[3] = StringToInt(&data[12]);
					sn_num[4] = StringToInt(&data[14]);
					sn_num[5] = StringToInt(&data[16]);		
					sn_num[6] = StringToInt(&data[18]);
					sn_num[7] = StringToInt(&data[20]);		
					EEPROM.begin(512); 
					EEPROM.put(0, sn_num);
					EEPROM.commit();
					EEPROM.end();
					writesn();
					USE_SERIAL.printf("SN:%02x%02x%02x%02x%02x%02x%02x%02x \r\n",
						sn_num[0],sn_num[1],sn_num[2],sn_num[3],sn_num[4],sn_num[5],sn_num[6],sn_num[7]);
					USE_SERIAL.println("End write");
					led.exit_led_uptick();
					return ;
	        	}
	    	}
		}
		else
		{
			EEPROM.begin(512); 
			EEPROM.put(0, sn_num);
			EEPROM.commit();
			EEPROM.end();
		}
	}
	else
	{
		USE_SERIAL.printf("SN:%02x%02x%02x%02x%02x%02x%02x%02x \r\n",
			sn_num[0],sn_num[1],sn_num[2],sn_num[3],sn_num[4],sn_num[5],sn_num[6],sn_num[7]);
	}
	return ;
}
void sn_class::read(uint8_t *data)
{
	data[0] = sn_num[0];
	data[1] = sn_num[1];
	data[2] = sn_num[2];
	data[3] = sn_num[3];
	data[4] = sn_num[4];
	data[5] = sn_num[5];
	data[6] = sn_num[6];
	data[7] = sn_num[7];
//	data[8] = sn_num[8];
}
const int tc_freqs [] = { 
476300000,
476500000,
476700000,
476900000,
477100000,
477300000,
477500000,
477700000,
477900000,
478100000,
};
void sn_class::set_ynl_freq(int *data)
{
	uint8_t  my_ch = 0; 							
	my_ch = sn_num[7]&0x0f;
	if(my_ch>=0x0a)
	{
		my_ch = 9;//默认通道9
	}
	*data = tc_freqs[my_ch];
	USE_SERIAL.printf("sn_num:%d \r\n",my_ch);
	return ;
}
sn_class sn;


