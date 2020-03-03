/******************************************************************************

 				 版权所有 (C), 2017-2027, 福州晴川科技有限公司

 ******************************************************************************
  文 件 名   : lora_rx_tx.h
  版 本 号   : 初稿
  作    者   : zenghi
  联系方式 : 309966755@qq.com
  生成日期   : 2019-04-15
  功能描述   : lora_rx_tx.cpp 的头文件
******************************************************************************/
#ifndef __LORA_RX_TX_H__
#define __LORA_RX_TX_H__
//#include "all.h"
#include "lora_1278.h"
#include "lora_server.h"
#include <gBase64.h>
#include <ArduinoJson.h>
#include "sn_deal.h"

struct LoraBuffer {
	uint8_t	* 	payLoad;
	uint8_t		payLength;
	uint32_t	tmst;						// in millis()
	uint8_t		sfTx;
	uint8_t		powe;
	uint32_t	fff;
	uint8_t		crc;
	uint8_t		iiq;
	uint16_t	s_tmst;						// in s()
};

struct LoraUpdata {
	uint8_t		payLoad[128];
	uint8_t		payLength;
	int			prssi; 
	long		snr;
	int			rssicorr;
	uint8_t		sf;
};

class lora_rx_tx_calss
{
	public:
    bool tx_send_ing;       //发送数据保护锁 防止新发射刷新
	bool tx_ok;            //定时器动作ok标志位
	LoraBuffer* LoraDown_bak = &fifo_LoraDown[0];
	uint32_t cp_nb_rx_rcv = 0; /* count packets received */
	uint32_t cp_nb_rx_ok = 0; /* count packets received */
	int cp_up_ack_rcv	 = 0;//得到的ACK 用于网络质量参考
	uint32_t cp_dw_dgram_rcv = 0;//得到服务器数据
	uint32_t cp_nb_tx_ok = 0;	//服务器得到的数据下发OK
	void rx_stuts_deal();
	void tx_send();
	int sendPacket(uint8_t *buf, uint8_t length);
	void add_server_ack();
	int get_server_ack();
	void tx_loop();
		
	private:
	LoraBuffer now_LoraDown;//1S之内收发SF7 最快也就20
	LoraBuffer fifo_LoraDown[20];//1S之内收发SF7 最快也就20
	uint8_t payLoad[128];		
	void get_freq(long unsigned int f, char *val ); 
	int buildPacket(uint32_t tmst, uint8_t *buff_up, struct LoraUpdata LoraUp, bool internal); 
	int receivePacket(); 
	LoraBuffer LoraDown;
	LoraUpdata LoraUp;
};

extern lora_rx_tx_calss lora_even;

#endif /* __LORA_RX_TX_H__ */
