/******************************************************************************

 				 版权所有 (C), 2017-2027, 福州晴川科技有限公司

 ******************************************************************************
  文 件 名   : lora_rx_tx.cpp
  版 本 号   : 初稿
  作    者   : zenghi
  联系方式 : 309966755@qq.com
  生成日期   : 2019-04-15
  功能描述   : lora RX TX机制处理
******************************************************************************/
#include "lora_rx_tx.h"
#include "qc_telnet.h"
#define USE_SERIAL telnet//Serial

extern "C" {

static ICACHE_RAM_ATTR void timer1Interrupt()
{
	timer1_disable();
	lora_even.tx_ok = false;
	if(lora_even.tx_send_ing==false)//sx1278没有再发射数据，有再发射数据 此次发射数据丢失处理
	{
	    lora_even.tx_send_ing = true;
//		Serial.printf("tx go\r\n");
    	lora.txLoraModem(
    		lora_even.LoraDown_bak->payLoad,
    		lora_even.LoraDown_bak->payLength,
    		lora_even.LoraDown_bak->tmst,
    		lora_even.LoraDown_bak->sfTx,
    		lora_even.LoraDown_bak->powe,
    		lora_even.LoraDown_bak->fff,
    		lora_even.LoraDown_bak->crc,
    		lora_even.LoraDown_bak->iiq
    	);
	}
	memset(lora_even.LoraDown_bak,0,sizeof(LoraBuffer));
}

// Interrupt on/off control
static ICACHE_RAM_ATTR void timer1Interrupt();
/*************************************************************************************************
 * 功 能 : 重载定时器
 * 输 入 : uint32_t dealy  delay 最大值 8388607/5
 *************************************************************************************************/
static void initTimer(uint32_t dealy) {
  timer1_disable();
  ETS_FRC_TIMER1_INTR_ATTACH(NULL, NULL);
  ETS_FRC_TIMER1_NMI_INTR_ATTACH(timer1Interrupt);
//  timer1_enable(TIM_DIV1 , TIM_EDGE, TIM_SINGLE);
//  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
//	8388607
	timer1_write(dealy*5);//TIM_DIV16 5个机器周期才为1 US
}

static void ICACHE_RAM_ATTR deinitTimer() 
{
  ETS_FRC_TIMER1_NMI_INTR_ATTACH(NULL);
  timer1_disable();
  timer1_isr_init();
}

}

//1
/*************************************************************************************************
 * 功 能 : 获取频点
 *************************************************************************************************/
void lora_rx_tx_calss::get_freq(long unsigned int f, char *val ) 
{
	char num = 0;
	*val = f%10000UL/1000 + '0';
	val++;			  
	*val = f%1000UL/100 + '0';
	val++;			  
	*val = f%100UL/10 + '0';
	val++;			  
	*val = '.';
	val++;			  
	*val = f%10UL/1 + '0';
	val++;			  
	*val = '0';
	val++;			  
	*val = '0';
	val++;			  
	*val = '0';
	val++;			  
	*val = '0';
	val++;			  
	*val = '0';
	val++;			  
	*val =0;
}

/*************************************************************************************************
 * 功 能 : 数据打包
 *************************************************************************************************/
int lora_rx_tx_calss::buildPacket(uint32_t tmst, uint8_t *buff_up, struct LoraUpdata LoraUp, bool internal) 
{
	long SNR;
    int rssicorr;
	int prssi;											// packet rssi
	
	char cfreq[12] = {0};								// Character array to hold freq in MHz
//	lastTmst = tmst;									// Following/according to spec
	int buff_index=0;
	char b64[256];
	
	uint8_t *message = LoraUp.payLoad;
	char messageLength = LoraUp.payLength;
		
#if 0
	unsigned char NwkSKey[16] = _NWKSKEY;
	checkMic(message, messageLength, NwkSKey);
#endif// endif 0

	// Read SNR and RSSI from the register. Note: Not for internal sensors!
	// For internal sensor we fake these values as we cannot read a register
	if (internal) {
		SNR = 12;
		prssi = 50;
		rssicorr = RSSI_OFFSET_LF;
	}
	else {
		SNR = LoraUp.snr;
		prssi = LoraUp.prssi;								// read register 0x1A, packet rssi
		rssicorr = LoraUp.rssicorr;
	}
	int j;
	
	int encodedLen = base64_enc_len(messageLength);		// max 341
	base64_encode(b64, (char *) message, messageLength);// max 341
	uint8_t token_h = (uint8_t)rand(); 					// random token
	uint8_t token_l = (uint8_t)rand(); 					// random token
	
	buff_up[0] = PROTOCOL_VERSION;						// 0x01 still
	buff_up[1] = token_h;
	buff_up[2] = token_l;
	
	buff_up[3] = PKT_PUSH_DATA;							// 0x00
	
	// READ MAC ADDRESS OF ESP8266, and insert 0xFF 0xFF in the middle
	sn.read(&buff_up[4]);
	buff_index = 12; 									// 12-byte binary (!) header

	// start of JSON structure that will make payload
	memcpy((void *)(buff_up + buff_index), (void *)"{\"rxpk\":[", 9);
	buff_index += 9;
	buff_up[buff_index] = '{';
	++buff_index;
	j = snprintf((char *)(buff_up + buff_index), TX_BUFF_SIZE-buff_index, "\"tmst\":%u", tmst);
	buff_index += j;
	get_freq((long unsigned int)478100000/100000,cfreq);					// XXX This can be done better
	j = snprintf((char *)(buff_up + buff_index), TX_BUFF_SIZE-buff_index, ",\"chan\":%1u,\"rfch\":%1u,\"freq\":%s", 0, 0, cfreq);
	buff_index += j;
	memcpy((void *)(buff_up + buff_index), (void *)",\"stat\":1", 9);
	buff_index += 9;
	memcpy((void *)(buff_up + buff_index), (void *)",\"modu\":\"LORA\"", 14);
	buff_index += 14;
	
	/* Lora datarate & bandwidth, 16-19 useful chars */
	switch (LoraUp.sf) {
		case SF6:
			memcpy((void *)(buff_up + buff_index), (void *)",\"datr\":\"SF6", 12);
			buff_index += 12;
			break;
		case SF7:
			memcpy((void *)(buff_up + buff_index), (void *)",\"datr\":\"SF7", 12);
			buff_index += 12;
			break;
		case SF8:
            memcpy((void *)(buff_up + buff_index), (void *)",\"datr\":\"SF8", 12);
            buff_index += 12;
            break;
		case SF9:
            memcpy((void *)(buff_up + buff_index), (void *)",\"datr\":\"SF9", 12);
            buff_index += 12;
            break;
		case SF10:
            memcpy((void *)(buff_up + buff_index), (void *)",\"datr\":\"SF10", 13);
            buff_index += 13;
            break;
		case SF11:
            memcpy((void *)(buff_up + buff_index), (void *)",\"datr\":\"SF11", 13);
            buff_index += 13;
            break;
		case SF12:
            memcpy((void *)(buff_up + buff_index), (void *)",\"datr\":\"SF12", 13);
            buff_index += 13;
            break;
		default:
            memcpy((void *)(buff_up + buff_index), (void *)",\"datr\":\"SF?", 12);
            buff_index += 12;
	}
	memcpy((void *)(buff_up + buff_index), (void *)"BW125\"", 6);
	buff_index += 6;
	memcpy((void *)(buff_up + buff_index), (void *)",\"codr\":\"4/5\"", 13);
	buff_index += 13;
	j = snprintf((char *)(buff_up + buff_index), TX_BUFF_SIZE-buff_index, ",\"lsnr\":%li", SNR);
	buff_index += j;
	j = snprintf((char *)(buff_up + buff_index), TX_BUFF_SIZE-buff_index, ",\"rssi\":%d,\"size\":%u", prssi-rssicorr, messageLength);
	buff_index += j;
	memcpy((void *)(buff_up + buff_index), (void *)",\"data\":\"", 9);
	buff_index += 9;

	// Use gBase64 library to fill in the data string
	encodedLen = base64_enc_len(messageLength);			// max 341
	j = base64_encode((char *)(buff_up + buff_index), (char *) message, messageLength);

	buff_index += j;
	buff_up[buff_index] = '"';
	++buff_index;

	// End of packet serialization
	buff_up[buff_index] = '}';
	++buff_index;
	buff_up[buff_index] = ']';
	++buff_index;
	
	// end of JSON datagram payload */
	buff_up[buff_index] = '}';
	++buff_index;
	buff_up[buff_index] = 0; 							// add string terminator, for safety

	
	return(buff_index);
}


/*************************************************************************************************
 * 功 能 : 数据打包送到服务器
 *************************************************************************************************/
int lora_rx_tx_calss::receivePacket()
{
	uint8_t buff_up[TX_BUFF_SIZE]; 					
	long SNR;
	uint8_t message[128] = { 0x00 };				
	uint8_t messageLength = 0;
	uint32_t tmst = (uint32_t) micros();			

	if (LoraUp.payLength > 0) {
        int build_index = buildPacket(tmst, buff_up, LoraUp, false);
		LoraUp.payLength = 0;
		LoraUp.payLoad[0] = 0x00;
		if (!lorawan.send_loradata(buff_up, build_index)) {
			USE_SERIAL.printf("SEND error!!!\r\n");
			return(-2); 							
		}
		return(build_index);
    }
	
	return(0);										
	
}

/*************************************************************************************************
 * 功 能 : 服务器ACK回馈
 *************************************************************************************************/
void lora_rx_tx_calss::add_server_ack()
{
	cp_up_ack_rcv++;
}
/*************************************************************************************************
 * 功 能 : 获取ACK计数
 *************************************************************************************************/
int lora_rx_tx_calss::get_server_ack()
{
	int data  = 0;//得到的ACK 用于网络质量参考
	data = cp_up_ack_rcv;
	cp_up_ack_rcv = 0;
	return data;
}

/*************************************************************************************************
 * 功 能 : 处理LORA 状态机制
 *************************************************************************************************/
void lora_rx_tx_calss::rx_stuts_deal()
{
	tx_loop();
	switch(lora.get_stuts())
	{
	    case LORA_RX :
	        break;
	    case LORA_RX_DONE :
			tx_send_ing = false;
			cp_nb_rx_rcv++; 										// Receive statistics counter
			LoraUp.payLoad[0]= 0x00;								// Empty the message
			if((LoraUp.payLength = lora.receivePkt(LoraUp.payLoad)) <= 0) {
				USE_SERIAL.println("NULL");
				lora.rxLoraModem();
				return;
			}
			LoraUp.snr = lora.get_snr();
			LoraUp.prssi = lora.get_rssi();
			LoraUp.sf = lora.get_sf();
			if (receivePacket() <= 0) {
				USE_SERIAL.printf("\nerror =");
			}
			USE_SERIAL.printf("ADDR=%x  rssi=%d  snr=%d",
			LoraUp.payLoad[4]<<24|LoraUp.payLoad[3]<<16|LoraUp.payLoad[2]<<8|LoraUp.payLoad[1],LoraUp.prssi,LoraUp.snr);
			cp_nb_rx_ok++;
	        break;
	    case LORA_RX_CRCERR :
			USE_SERIAL.println("CRCERR");
			tx_send_ing = false;
			lora.rxLoraModem();
	        break;
	    case LORA_RX_TOUT :
			USE_SERIAL.println("RX_TOUT");
			tx_send_ing = false;
			lora.rxLoraModem();
	        break;
        case LORA_TX_DONE :
			tx_send_ing = false;
			lora.rxLoraModem();
	        break;
	    case LORA_ERROR :
			tx_send_ing = false;
			lora.begin();
	        break;
	    default:
		break;
	}
}


/*************************************************************************************************
 * 功 能 : 获取服务器数据 进行TX发射
 *************************************************************************************************/
int lora_rx_tx_calss::sendPacket(uint8_t *buf, uint8_t length) 
{
	int i=0;
	cp_dw_dgram_rcv++;
	StaticJsonBuffer<1024> jsonBuffer;
	char * bufPtr = (char *) (buf);
	buf[length] = 0;
	
	JsonObject& root = jsonBuffer.parseObject(bufPtr);
	if (!root.success()) {
		USE_SERIAL.println("rootERR");
		return(-1);
	}
	delay(1);
	// Meta Data sent by server (example)
	// {"txpk":{"codr":"4/5","data":"YCkEAgIABQABGmIwYX/kSn4Y","freq":868.1,"ipol":true,"modu":"LORA","powe":14,"rfch":0,"size":18,"tmst":1890991792,"datr":"SF7BW125"}}
	const char * data	= root["txpk"]["data"];
	uint8_t psize		= root["txpk"]["size"];
	bool ipol			= root["txpk"]["ipol"];
	uint8_t powe		= root["txpk"]["powe"];
	uint32_t tmst		= (uint32_t) root["txpk"]["tmst"].as<unsigned long>();

	// Not used in the protocol:
	const char * datr	= root["txpk"]["datr"];			// eg "SF7BW125"
	const float ff		= root["txpk"]["freq"];			// eg 869.525
	const char * modu	= root["txpk"]["modu"];			// =="LORA"
	const char * codr	= root["txpk"]["codr"];

	if (data != NULL) {
	}
	else {
		USE_SERIAL.println("dataERR");
		return(-1);
	}
	
	uint8_t iiq = (ipol? 0x40: 0x27);					// if ipol==true 0x40 else 0x27
	uint8_t crc = 0x00;									// switch CRC off for TX
	uint8_t payLength = base64_dec_len((char *) data, strlen(data));
	
	// Fill payload with decoded message
	base64_decode((char *) payLoad, (char *) data, strlen(data));

	const uint8_t sfTx = atoi(datr+2);					// Convert "SF9BW125" to 9
	uint32_t fff;
	if(wps.gw.lora_tx_freq)
	{
	     fff = wps.gw.lora_tx_freq;
	}
	else
	{
		 fff = (uint32_t) ((uint32_t)((ff)*1000)) * 1000;
	    USE_SERIAL.printf("fff:%d\r\n",fff);
	}
	LoraDown.payLoad = payLoad;
	LoraDown.payLength = payLength;
	LoraDown.tmst = tmst;								// Downstream in milis
	LoraDown.sfTx = sfTx;
	LoraDown.powe = powe;
	LoraDown.fff = fff;
	LoraDown.crc = crc;
	LoraDown.iiq = iiq;
	cp_nb_tx_ok++;

	tx_send();

	if (payLength != psize) {
	}
	return 1;
}
/*************************************************************************************************
 * 功 能 : 存需要发的无线数据
 *************************************************************************************************/
void lora_rx_tx_calss::tx_send()
{
	static int count = 0;
	uint32_t micros1 = 0;
	if ( count < sizeof(fifo_LoraDown)/sizeof(LoraBuffer) )
	{
//		USE_SERIAL.printf("GET tmst :%d\r\n",LoraDown.tmst );
//		USE_SERIAL.printf("GET micros :%ld\r\n",micros() );
		micros1 = (LoraDown.tmst - micros());
//		USE_SERIAL.printf("GET micros1 :%d\r\n",micros1 );
		LoraDown.s_tmst = (micros1/1e6);
//		USE_SERIAL.printf("GET TX :%d\r\n",LoraDown.s_tmst );
		if(LoraDown.s_tmst>=0&&LoraDown.s_tmst<9)
		{
			LoraDown.s_tmst+=1;
			LoraDown.s_tmst = LoraDown.s_tmst * LoraDown.s_tmst;//5s 偏移25ms  担心标准模块不准确
		}
		else
		{
		    LoraDown.s_tmst = 1;
		}
	    fifo_LoraDown[count] = LoraDown;
		count++;
		if(count==sizeof(fifo_LoraDown)/sizeof(LoraBuffer))
		{
			count = 0;
		}
	}
}
/*************************************************************************************************
 * 功 能 : 发射数据 查找列队数据最近一次需要发射的数据
 *************************************************************************************************/
void lora_rx_tx_calss::tx_loop()
{
	int count = 0;
	long long time_data = 0;
	int count_bak = 0;
	LoraBuffer LoraDown_classc;
	for ( count = 0 ; count<sizeof(fifo_LoraDown)/sizeof(LoraBuffer) ; count++ )
	{
		if(fifo_LoraDown[count].tmst)
		{
			if(LoraDown_bak->tmst==0&&fifo_LoraDown[count].tmst)//0数据处理
			{
				LoraDown_bak = &fifo_LoraDown[count];
			}
		    else if(LoraDown_bak->tmst > fifo_LoraDown[count].tmst)//取时间快到的数据
    		{
    		    LoraDown_bak = &fifo_LoraDown[count];
    			count_bak = count;
    		}
		}
		
		if(fifo_LoraDown[count].payLength && fifo_LoraDown[count].tmst==0)//CLASSC 马上发射
		{
		    LoraDown_classc = fifo_LoraDown[count];
			if(tx_send_ing==false)
			{
			    tx_send_ing = true;
    			lora.txLoraModem(
    				fifo_LoraDown[count].payLoad,
    				fifo_LoraDown[count].payLength,
    				fifo_LoraDown[count].tmst,
    				fifo_LoraDown[count].sfTx,
    				fifo_LoraDown[count].powe,
    				fifo_LoraDown[count].fff,
    				fifo_LoraDown[count].crc,
    				fifo_LoraDown[count].iiq
    			);
				memset(&fifo_LoraDown[count],0,sizeof(LoraBuffer));
			}
			
		}
	}
	if(LoraDown_bak->tmst&&tx_ok==false)
	{
		time_data = LoraDown_bak->tmst - micros();
		if(time_data<9000e3&&time_data>1000e3)
		{
		    //继续等待，到1S以内再给定时器动作
		}
		else if( time_data<(-1000e3)|| time_data>9000e3  )//小于100MS 大于9S错误
		{
			USE_SERIAL.printf("Timeout, poor network or server performance\r\n");
#if 0
			lora_even.LoraDown_bak->sfTx = SF12;//超时单独给盈能量做的
			lora.txLoraModem(
				lora_even.LoraDown_bak->payLoad,
				lora_even.LoraDown_bak->payLength,
				lora_even.LoraDown_bak->tmst,
				lora_even.LoraDown_bak->sfTx,
				lora_even.LoraDown_bak->powe,
				lora_even.LoraDown_bak->fff,
				lora_even.LoraDown_bak->crc,
				lora_even.LoraDown_bak->iiq
			);
#endif// endif 0
			memset(LoraDown_bak,0,sizeof(LoraBuffer));
		}
		else if(time_data<1000e3 && time_data>0)//1S以内数据
		{
			tx_ok = true;
			memcpy(&now_LoraDown,LoraDown_bak,sizeof(LoraBuffer));//保存临时待发射数据
//			if(8388607)
		    initTimer(time_data+wps.gw.tx_dealy*LoraDown_bak->s_tmst);//定时器调用后清除数据,中断调用允许打断保证下行数据的优先级
			USE_SERIAL.printf("tx send\r\n");
		}
	}
	else if(LoraDown_bak->tmst&&tx_ok==true)
	{
	    if(LoraDown_bak->tmst!=now_LoraDown.tmst)//临界区，与待发射数据不一致 来了插队数据 
		{
			timer1_disable();
			if(LoraDown_bak->tmst)
			{
			    memcpy(&now_LoraDown,LoraDown_bak,sizeof(LoraBuffer));
    			time_data = LoraDown_bak->tmst - micros();
    		    tx_ok = true;
				initTimer(time_data+wps.gw.tx_dealy*LoraDown_bak->s_tmst);//定时器调用后清除数据,中断调用允许打断保证下行数据的优先级
			}
		}
	}
}
lora_rx_tx_calss lora_even; //lora事件机制

