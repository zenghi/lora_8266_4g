/******************************************************************************

 				 版权所有 (C), 2017-2027, 福州晴川科技有限公司

 ******************************************************************************
  文 件 名   : AT_4G.h
  版 本 号   : 初稿
  作    者   : zenghi
  联系方式 : 309966755@qq.com
  生成日期   : 2019-04-19
  功能描述   : AT_4G.cpp 的头文件
******************************************************************************/
#ifndef __AT_4G_H__
#define __AT_4G_H__
#include <ESP8266WiFi.h>
#include "Stream.h"
typedef enum 
{
    INIT_ING = 0,
	INIT_OK,
	INIT_ERR
}AT_4G_STATUS_ENUM;

class AT_4G_CLASS
{
public:
	uint8_t power_pin = 15;
	void open_pow(){pinMode(power_pin, OUTPUT);digitalWrite(power_pin, 0);};
	void close_pow(){pinMode(power_pin, OUTPUT);digitalWrite(power_pin, 1);};
	String gprs_severip;
	int gprs_port;
//	void get_csq(void *arg);
//	void AT_RESET(void *arg);
	int send(char * data, int len);
	int recv(char* rxbuffer);

    bool start(String severip,int port,uint8_t pow_pin);
    bool run();
	AT_4G_STATUS_ENUM network();
	bool get_time();
private:
};
//#define open_pow() do{pinMode(power_pin, OUTPUT);digitalWrite(power_pin, 0);}	while(0)
//#define close_pow() do{pinMode(power_pin, OUTPUT);digitalWrite(power_pin, 1);}while(0)	
extern AT_4G_CLASS gprs;

#endif /* __AT_4G_H__ */
