/******************************************************************************

 				 版权所有 (C), 2017-2027, 福州晴川科技有限公司

 ******************************************************************************
  文 件 名   : wps_www_server.h
  版 本 号   : 初稿
  作    者   : zenghi
  联系方式 : 309966755@qq.com
  生成日期   : 2019-04-16
  功能描述   : wps_www_server.cpp 的头文件
******************************************************************************/
#ifndef __WPS_WWW_SERVER_H__
#define __WPS_WWW_SERVER_H__
#include <WiFiManager.h>						
#include <ESP8266WebServer.h>
#include "FS.h"
enum sf_t {SF_AUTO=1,SF6=6, SF7, SF8, SF9, SF10, SF11, SF12 };

#define AP_NAME "QC1278"
#define AP_PASSWD "123456"
#define DBG_PRINTF(...)   //Serial.print
#define _LAT 27.106//25.750941//27.106
#define _LON 123.476528//120.248
#define CONFIGFILE "/qc.txt"
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */
struct gw_config {
	String ssid="QC";		
	String pass="123456789";		
	String severip="lora.zenghi.com"; 	
	int port = 1700;			
	float lat=0;			  
	float lon=0;
	sf_t lora_sf=SF10;			
	int lora_tx_freq = 478100000;//server端频点 		
	int lora_freq=478100000; 		
	int tx_dealy= -1000;//默认快4ms发射，寄存器写入运行时间消耗			
	bool mifi_flg = false;//默认没有4G版本	
} ;
//enum sf_t {SF_AUTO=1,SF6=6, SF7, SF8, SF9, SF10, SF11, SF12 };
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

class wps_server
{
public:
	gw_config	gw;
	int readConfig(const char *fn, struct gw_config *c);
	int read_gw();
	bool deal_goto_wps();
	void goto_wps();
	int writeConfig(const char *fn, struct gw_config *c);
	void  write_ttn_bak();	
};
extern wps_server wps;

#endif /* __WPS_WWW_SERVER_H__ */
