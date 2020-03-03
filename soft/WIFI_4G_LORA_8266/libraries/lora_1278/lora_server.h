/******************************************************************************

 				 版权所有 (C), 2017-2027, 福州晴川科技有限公司

 ******************************************************************************
  文 件 名   : lora_server.h
  版 本 号   : 初稿
  作    者   : zenghi
  联系方式 : 309966755@qq.com
  生成日期   : 2019-04-14
  功能描述   : lora_server.cpp 的头文件
******************************************************************************/
#ifndef __LORA_SERVER_H__
#define __LORA_SERVER_H__
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <sys/time.h>
#include <TimeLib.h>	
#include "lora_rx_tx.h"
#include "lora_1278.h"
#include "sn_deal.h"

#define _TTNPORT    1700
#define _TTNSERVER "zenghi.com" 

#define NTP_TIMESERVER "ntp1.aliyun.com"
#define NTP_TIMESERVER1 "ntp2.aliyun.com"
#define NTP_TIMESERVER2 "cn.pool.ntp.org"

#define _PULL_INTERVAL 15//30				
#define _STAT_INTERVAL 150				
#define _NTP_INTERVAL 36000UL				

#define NTP_TIMEZONES	8
#define SECS_IN_HOUR	3600


#define PROTOCOL_VERSION			0x02  
#define PKT_PUSH_DATA				0x00
#define PKT_PUSH_ACK				0x01
#define PKT_PULL_DATA				0x02
#define PKT_PULL_RESP				0x03
#define PKT_PULL_ACK				0x04
#define PKT_TX_ACK                  0x05

#define MGT_RESET					0x15		// Not a LoRa Gateway Spec message
#define MGT_SET_SF					0x16
#define MGT_SET_FREQ				0x17

#define TX_BUFF_SIZE  1024						// Upstream buffer to send to MQTT
#define RX_BUFF_SIZE  1024						// Downstream received from MQTT
#define STATUS_SIZE	  512				

//#ifdef __cplusplus
//#if __cplusplus
//extern "C"{
//#endif
//#endif /* __cplusplus */
class lora_server_class
{
public:
	bool wifi_ok;
	bool UDPconnect();
	void data_deal();
	time_t getNtpTime();
//	bool Ntp_Time_Init();
	int sendNtpRequest(IPAddress timeServerIP);
	int sendUdp(IPAddress server, int port, uint8_t *msg, int length);
	int send_loradata(uint8_t *msg, int length);
	int readUdp(int packetSize);
	int read_gprs_Udp();
	void sendstat();
	void pullData();
	bool server_get_host();
	void start();
	void set_wifi_status(bool wifi_status);
private:
WiFiUDP _Udp;
String server_name = _TTNSERVER;
int server_port = _TTNPORT;
IPAddress ttnServer;					
IPAddress ntpServer;
uint32_t statTime = 0;					
uint32_t pulltime = 0;							
uint32_t ntptimer = 0;
//	void RxChainCalibration( void );
//	uint8_t readRegister(uint8_t addr);
//	void writeRegister(uint8_t addr, uint8_t value);
//	void writeBuffer(uint8_t addr, uint8_t *buf, uint8_t len);


};


//#ifdef __cplusplus
//#if __cplusplus
//}
//#endif
//#endif /* __cplusplus */

extern lora_server_class lorawan;

#endif /* __LORA_SERVER_H__ */
