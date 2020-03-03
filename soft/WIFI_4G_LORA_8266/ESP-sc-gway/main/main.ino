/******************************************************************************

 				 版权所有 (C), 2017-2027, 福州晴川科技有限公司

 ******************************************************************************
  文 件 名   : main.ino
  版 本 号   : 初稿
  作    者   : zenghi
  联系方式 : 309966755@qq.com
  生成日期   : 2019-04-24
  功能描述   : 单通道lorawan网关主入口
******************************************************************************/
#include "all.h"
//defineTask(Task1);
#define USE_SERIAL Serial
#define GPRS_POWER_PIN 15


ESP8266WiFiMulti WiFiMulti;
int up_fw = 0;
int count = 0;
HardwareSerial Serial_bak(UART0);
HardwareSerial Serial_bak1(UART1);

void setup() {

    Serial.begin(115200);
//    Serial.begin(921600);
    Serial1.begin(921600);
    Serial_bak = Serial;
    Serial_bak1 = Serial1;
	USE_SERIAL.flush();
	USE_SERIAL.printf("VERSION:"VERSION"\r\n");
	USE_SERIAL.printf("*********************************************************\r\n");
    USE_SERIAL.printf("Welcome to qingchuan %s gw LTS (GNU QC1278 V1.0)\r\n",__DATE__);
    USE_SERIAL.printf("*     Qingchuan Technology Co., Ltd.\r\n");
    USE_SERIAL.printf("* Support:      http://zenghi.com\r\n");
    USE_SERIAL.printf("* AUTHOR:       黄瀚霆 zenghi \r\n");
    USE_SERIAL.printf("* EMAIL:        309966755@qq.com\r\n");
    USE_SERIAL.printf("*********************************************************\r\n");
	led.start();											 //LED初始化进行闪烁
	wps.read_gw();											 //获取全局设置参数
	USE_SERIAL.printf("ssid:%s pass:%s \r\n",(char *)wps.gw.ssid.c_str(), (char *)wps.gw.pass.c_str());

    lorawan.start();										 //lorawan驱动和服务启动
    Serial = Serial_bak1;                                  //对调串口方便gprs直接使用serial1 实际使用serial的口  不用动代码
    Serial1 = Serial_bak;                                  //对调串口方便gprs直接使用serial1 实际使用serial的口  不用动代码
    gprs.start(wps.gw.severip,wps.gw.port,GPRS_POWER_PIN);   //4g模组开机和参数设定
    WiFi.persistent(false);									 //不保存wifi参数防止反复开机坏闪存
	WiFi.mode(WIFI_STA);    								 //wifi模式
	WiFi.hostname("QC1278");								 //WiFi名称设定便于telnet
	delay(1000);											 //延时等待host生效
	WiFi.disconnect();										 //移除旧网络
	telnet.start();											 //启动telnet服务
    WiFiMulti.addAP(wps.gw.ssid.c_str(), wps.gw.pass.c_str());
}

void loop() 
{
	static long once_show = 0;
	static bool once = false;
	if((WiFiMulti.run() == WL_CONNECTED)) 
	{
		if(once==false)
		{
			once = true;
		    while(!lorawan.server_get_host())
    		{
    		    USE_SERIAL.printf("HOST ERROR");
    		    delay(500);
    		    while(1);
    		}
		}

		lorawan.set_wifi_status(true);
		telnet.loop();
	}
	else
	{
		if(once_show++>999999UL)
		{
			once_show = 0;
		    USE_SERIAL.printf("wifi disconect");
		}
		lorawan.set_wifi_status(false);
	}
	lorawan.data_deal();
	OTA.update_fw();
	wps.deal_goto_wps();
	led.run();
	yield();
}

