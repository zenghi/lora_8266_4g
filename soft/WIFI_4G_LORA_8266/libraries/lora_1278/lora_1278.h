/******************************************************************************

 				 版权所有 (C), 2017-2027, 福州晴川科技有限公司

 ******************************************************************************
  文 件 名   : lora_1278.h
  版 本 号   : 初稿
  作    者   : zenghi
  联系方式 : 309966755@qq.com
  生成日期   : 2019-04-14
  功能描述   : lora_1278.h 的头文件
******************************************************************************/
#ifndef __LORA_1278_H__
#define __LORA_1278_H__
#include <SPI.h>
#include "SX1278_REG.h"
#include "wps_www_server.h"
extern "C" {
#include "user_interface.h"
#include "lwip/err.h"
#include "lwip/dns.h"
#include "c_types.h"
}
#define MYRSSI_OFFSET_LF -164//157
#define RSSI_OFFSET_LF 164//157
//namespace lora_1278 {
//};

enum lora_status {LORA_RX=0,LORA_RX_DONE,LORA_RX_TOUT,
	LORA_RX_CRCERR,LORA_TX,LORA_TX_DONE,LORA_ERROR };

#define SPISPEED1 14000000//8000000						// Set to 8 * 10E6
//#define readSettings SPISettings(14000000, MSBFIRST, SPI_MODE0)
//#define writeSettings SPISettings(14000000, MSBFIRST, SPI_MODE0)
#define readSettings SPISettings(SPISPEED1, MSBFIRST, SPI_MODE0)
#define writeSettings SPISettings(SPISPEED1, MSBFIRST, SPI_MODE0)
struct lora_pins_st {
	uint8_t dio0=4;		
	uint8_t dio1=5;		
//	uint8_t dio2=0;		
	uint8_t ss=16;		
	uint8_t rst=0;		
	uint8_t vc1=9;	// GPIO10    
	uint8_t vc2=10;	// GPIO9      
} ;


class lora_1278
{
public:
//	enum sf_t {SF_AUTO=1,SF6=6, SF7, SF8, SF9, SF10, SF11, SF12 };
	void begin();
	void begin(uint32_t freqs,sf_t sf);
	void loraWait(uint32_t tmst);
	uint8_t receivePkt(uint8_t *payload);
	bool sendPkt(uint8_t *payLoad, uint8_t payLength);
	void setFreq(uint32_t freq);
	void setPow(uint8_t powe);
	void opmode(uint8_t mode);
	void setRate(uint8_t sf, uint8_t crc);
	void txLoraModem(uint8_t *payLoad, uint8_t payLength, uint32_t tmst, uint8_t sfTx,
						uint8_t powe, uint32_t freq, uint8_t crc, uint8_t iiq);
	void rxLoraModem();
	void rxLoraModem_loop();
	lora_status get_stuts();
	int16_t get_rssi();
	int8_t get_snr();
	int8_t get_sf();
private:
	uint32_t net_freqs = 477900000;
	sf_t net_sf = SF7;
	lora_pins_st lora_pins;
	long txDelay= 0x00;
	void RxChainCalibration( void );
	uint8_t readRegister(uint8_t addr);
	void writeRegister(uint8_t addr, uint8_t value);
	void writeBuffer(uint8_t addr, uint8_t *buf, uint8_t len);
	void readBuffer(uint8_t addr, uint8_t *buf, uint8_t len);
};


//#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SPI)
extern lora_1278 lora;
//#endif

#endif /* __LORA_1278_H__ */
