/******************************************************************************

 				 版权所有 (C), 2017-2027, 福州晴川科技有限公司

 ******************************************************************************
  文 件 名   : led_show.h
  版 本 号   : 初稿
  作    者   : zenghi
  联系方式 : 309966755@qq.com
  生成日期   : 2019-04-24
  功能描述   : led_show.cpp 的头文件
******************************************************************************/
#ifndef __LED_SHOW_H__
#define __LED_SHOW_H__
#include <Ticker.h>
#include <ESP8266WiFi.h>

class led_class
{
	public:
	Ticker ticker;
	Ticker tickerup;
	volatile int dog_count = 0;//跑飞双重检测
    inline bool get_led();
    inline void led_CLOSE();
    inline void led_link();
    inline void led_OPEN();
    void start();
	void clean_dog_count();
	void exit_led_tick();
	void led_tick();
	void tick();

	
	void uptick();
	void led_upfw_tick();
	void exit_led_uptick();
	void led_wps_tick();
	void exit_wps_tick();
	void run();
};

extern led_class led;


#endif /* __LED_SHOW_H__ */
