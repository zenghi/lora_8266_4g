/******************************************************************************

 				 版权所有 (C), 2017-2027, 福州晴川科技有限公司

 ******************************************************************************
  文 件 名   : sn_deal.h
  版 本 号   : 初稿
  作    者   : zenghi
  联系方式 : 309966755@qq.com
  生成日期   : 2019-04-23
  功能描述   : sn_deal.cpp 的头文件
******************************************************************************/
#ifndef __SN_DEAL_H__
#define __SN_DEAL_H__
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include "FS.h"
#include "led_show.h"

class sn_class
{
public:
	  int CharToInt(char hex);
	  bool have_mac();
	  int readsn( );
	  void read_rx_data();
	  int StringToInt(char *hex);
	  int writesn( );
	  void start();
	  void read(uint8_t *data);
	  void set_ynl_freq(int *data);
private:
	String response;
	uint8_t sn_num[8] = {0};
};
extern sn_class sn;
#endif /* __SN_DEAL_H__ */
