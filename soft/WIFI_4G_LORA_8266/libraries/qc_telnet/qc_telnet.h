/******************************************************************************

 				 版权所有 (C), 2017-2027, 福州晴川科技有限公司

 ******************************************************************************
  文 件 名   : qc_telnet.h
  版 本 号   : 初稿
  作    者   : zenghi
  联系方式 : 309966755@qq.com
  生成日期   : 2019-04-18
  功能描述   : qc_telnet.cpp 的头文件
******************************************************************************/
#ifndef __QC_TELNET_H__
#define __QC_TELNET_H__
#include <ESP8266WiFi.h>
#include <algorithm> // std::min
#include <ESP8266mDNS.h>
#include "WiFiServer.h"
#include "Stream.h"
#define MAX_SRV_CLIENTS 1
#define STACK_PROTECTOR  512 // bytes


class qc_telnet_class : public Stream {
public:

int available(void) override
{
return 0;
}

int peek(void) override
{
    return 0;
}
int read(void) override
{
return 0;
}
size_t read(char* buffer, size_t size)
{
    return 0;
}
size_t readBytes(char* buffer, size_t size) override
{
return 0;
}
size_t readBytes(uint8_t* buffer, size_t size) override
{
    return 0;
}
int availableForWrite(void)
{
    return 0;
}
void flush(void) override
{
}
size_t write(uint8_t c) override;
size_t write(const uint8_t *buffer, size_t size) override;
using Print::write; // Import other write() methods to support things like write(0) properly

//WiFiServer  server;
//WiFiServer server(22);
WiFiClient serverClients[MAX_SRV_CLIENTS];
void loop();
void write(const char *s);
void start();
void printf_logo();
private:
//const int port = 23;
//WiFiServer  server1();
};

extern qc_telnet_class telnet;//lora外部引用
//extern qq ok;//lora外部引用
#endif /* __QC_TELNET_H__ */
