/******************************************************************************

 				 版权所有 (C), 2017-2027, 福州晴川科技有限公司

 ******************************************************************************
  文 件 名   : ota_fw.h
  版 本 号   : 初稿
  作    者   : zenghi
  联系方式 : 309966755@qq.com
  生成日期   : 2019-04-24
  功能描述   : ota_fw.cpp 的头文件
******************************************************************************/
#ifndef __OTA_FW_H__
#define __OTA_FW_H__

#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include "qc_telnet.h"
#include "led_show.h"
#include "lora_server.h"
#define VERSION_NUM 428
#define VERSION "QC428"
#define VERSION_BIN  VERSION".bin"
class OTA_CLASS
{
	public:
	bool once = false;
	void update_fw();
	void query_fw();
};
extern OTA_CLASS OTA;

#endif /* __OTA_FW_H__ */
