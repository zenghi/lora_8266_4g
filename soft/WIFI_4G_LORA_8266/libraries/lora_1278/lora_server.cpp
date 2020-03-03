/******************************************************************************

 				 版权所有 (C), 2017-2027, 福州晴川科技有限公司

 ******************************************************************************
  文 件 名   : lora_server.cpp
  版 本 号   : 初稿
  作    者   : zenghi
  联系方式 : 309966755@qq.com
  生成日期   : 2019-04-18
  功能描述   : lora server 服务处理
******************************************************************************/
#include "lora_server.h"
#include "AT_4G.h"
#include "ota_fw.h"
#define USE_SERIAL telnet//Serial
//#define USE_SERIAL Serial
//WiFiUDP _Udp;
/*************************************************************************************************
 * 功 能 : 打开UDP监听端口
 *************************************************************************************************/
bool lora_server_class::UDPconnect() 
{
	bool ret = false;
//	unsigned int localPort = _TTNPORT;			// To listen to return messages from WiFi
	USE_SERIAL.printf("ttn_po:: %d\r\n",server_port);
	if (_Udp.begin(server_port) == 1) {
		ret = true;
	}
	else{
	}
	return(ret);
}//udpConnect

/*************************************************************************************************
 * 功 能 : 送UDP数据
 *************************************************************************************************/
int lora_server_class::sendUdp(IPAddress server, int port, uint8_t *msg, int length) {
	if(!wifi_ok) 
	{
	    if(port!=123)
    	{
    	    gprs.send((char *)msg, length);
    		return(1);
    	}
	}
	else
	{
	    _Udp.flush();
    
    	if (!_Udp.beginPacket(server, (int) port)) {
    		USE_SERIAL.print("\nssendUdp:: Error Udp.beginPacket");
    		return(0);
    	}
    	
    
    	if (_Udp.write((char *)msg, length) != length) {
    		USE_SERIAL.print("\nsendUdp:: Error write");
    		_Udp.endPacket();						// Close UDP
    		return(0);								// Return error
    	}
    	
    	
    	if (!_Udp.endPacket()) {
    		USE_SERIAL.print("\nsendUdp:: Error Udp.endPacket");
    		return(0);
    	}
	}
	return(1);
}

/*************************************************************************************************
 * 功 能 : 送lora UDP数据
 *************************************************************************************************/
int lora_server_class::send_loradata(uint8_t *msg, int length) 
{
	return lorawan.sendUdp(ttnServer, server_port, msg, length);
}

/*************************************************************************************************
 * 功 能 : 读取监听端口数据
 *************************************************************************************************/
int lora_server_class::readUdp(int packetSize)
{
	uint8_t protocol;
	uint16_t token;
	uint8_t ident; 
	uint8_t buff[32]; 						// General buffer to use for UDP, set to 64
	uint8_t buff_down[RX_BUFF_SIZE];		// Buffer for downstream

	if (packetSize > RX_BUFF_SIZE) {
		USE_SERIAL.println("readUDP:: ERROR package of size: ");
		_Udp.flush();
		return(-1);
	}
  
	// We assume here that we know the originator of the message
	// In practice however this can be any sender!
	if (_Udp.read(buff_down, packetSize) < packetSize) {
		USE_SERIAL.println("readUsb:: Reading less chars");
	}

	// Remote Address should be known
	IPAddress remoteIpNo = _Udp.remoteIP();

	// Remote port is either of the remote TTN server or from NTP server (=123)
	unsigned int remotePortNo = _Udp.remotePort();

	if (remotePortNo == 123) {
		return(0);
	}
	else {
		uint8_t *data = (uint8_t *) ((uint8_t *)buff_down + 4);
		protocol = buff_down[0];
		token = buff_down[2]*256 + buff_down[1];
		ident = buff_down[3];

		// now parse the message type from the server (if any)
		switch (ident) {

		case PKT_PUSH_DATA: // 0x00 UP
		break;
	
		case PKT_PUSH_ACK:	// 0x01 DOWN
		break;
	
		case PKT_PULL_DATA:	// 0x02 UP
		break;
	
		case PKT_PULL_RESP:	// 0x03 DOWN
			if (lora_even.sendPacket(data, packetSize-4) < 0) {
				return(-1);
			}
		
			// Now respond with an PKT_TX_ACK; 0x04 UP
			buff[0]=buff_down[0];
			buff[1]=buff_down[1];
			buff[2]=buff_down[2];
			//buff[3]=PKT_PULL_ACK;				// Pull request/Change of Mogyi
			buff[3]=PKT_TX_ACK;
			sn.read(&buff[4]);
			buff[12]=0;
			// Only send the PKT_PULL_ACK to the UDP socket that just sent the data!!!
			_Udp.beginPacket(remoteIpNo, remotePortNo);
			if (_Udp.write((char *)buff, 12) != 12) {
			}
			else {
			}

			if (!_Udp.endPacket()) {
			}
			yield();
		break;
	
		case PKT_PULL_ACK:	// 0x04 DOWN; the server sends a PULL_ACK to confirm PULL_DATA receipt
			lora_even.add_server_ack();
//			USE_SERIAL.println("ack ok ");
//			clean_dog_count(); //一直没有消息直接就认为挂了重启
		break;
	
		default:
		break;
		}
		// For downstream messages
		return packetSize;
	}
}
/*************************************************************************************************
 * 功 能 : 读取监听端口数据
 *************************************************************************************************/
int lora_server_class::read_gprs_Udp()
{
	uint8_t protocol;
	uint16_t token;
	uint8_t ident; 
	uint8_t buff[32]; 						// General buffer to use for UDP, set to 64
	uint8_t buff_down[RX_BUFF_SIZE];		// Buffer for downstream
	int packetSize;
	packetSize = gprs.recv((char *)buff_down);
	if(packetSize!=0)
	{
	    uint8_t *data = (uint8_t *) ((uint8_t *)buff_down + 4);
    	protocol = buff_down[0];
    	token = buff_down[2]*256 + buff_down[1];
    	ident = buff_down[3];
    
    	// now parse the message type from the server (if any)
    	switch (ident) {
    
    	case PKT_PUSH_DATA: // 0x00 UP
    	break;
    
    	case PKT_PUSH_ACK:	// 0x01 DOWN
    	break;
    
    	case PKT_PULL_DATA:	// 0x02 UP
    	break;
    
    	case PKT_PULL_RESP:	// 0x03 DOWN
    		if (lora_even.sendPacket(data, packetSize-4) < 0) {
    			return(-1);
    		}
    	
    		// Now respond with an PKT_TX_ACK; 0x04 UP
    		buff[0]=buff_down[0];
    		buff[1]=buff_down[1];
    		buff[2]=buff_down[2];
    		//buff[3]=PKT_PULL_ACK;				// Pull request/Change of Mogyi
    		buff[3]=PKT_TX_ACK;
			sn.read(&buff[4]);
    		buff[12]=0;
			gprs.send((char *)buff, 12);
    		yield();
    	break;
    
    	case PKT_PULL_ACK:	// 0x04 DOWN; the server sends a PULL_ACK to confirm PULL_DATA receipt
//    		USE_SERIAL.println("ack ok ");
			lora_even.add_server_ack();
    //			clean_dog_count(); //一直没有消息直接就认为挂了重启
    	break;
    
    	default:
    	break;
    	}
	}
	// For downstream messages
	return packetSize;
}

/*************************************************************************************************
 * 功 能 : NTP服务UDP设置
 *************************************************************************************************/
int lora_server_class::sendNtpRequest(IPAddress timeServerIP) {
	const int NTP_PACKET_SIZE = 48;				// Fixed size of NTP record
	byte packetBuffer[NTP_PACKET_SIZE];

	memset(packetBuffer, 0, NTP_PACKET_SIZE);	// Zeroise the buffer.
	
	packetBuffer[0]  = 0b11100011;   			// LI, Version, Mode
	packetBuffer[1]  = 0;						// Stratum, or type of clock
	packetBuffer[2]  = 6;						// Polling Interval
	packetBuffer[3]  = 0xEC;					// Peer Clock Precision
	// 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12] = 49;
	packetBuffer[13] = 0x4E;
	packetBuffer[14] = 49;
	packetBuffer[15] = 52;	

	
	if (!sendUdp( (IPAddress) timeServerIP, (int) 123, packetBuffer, NTP_PACKET_SIZE)) {
		
		USE_SERIAL.printf("oerrork");
		return(0);	
	}
	return(1);
}


/*************************************************************************************************
 * 功 能 : NTP服务获取
 *************************************************************************************************/
time_t lora_server_class::getNtpTime()
{
    if (!sendNtpRequest(ntpServer))					
	{
		return(0);
	}
	
	const int NTP_PACKET_SIZE = 48;					
	byte packetBuffer[NTP_PACKET_SIZE];
	memset(packetBuffer, 0, NTP_PACKET_SIZE);		

    uint32_t beginWait = millis();
	delay(10);
    while (millis() - beginWait < 3500) 
	{
		int size = _Udp.parsePacket();
		if ( size >= NTP_PACKET_SIZE ) {
		
			if (_Udp.read(packetBuffer, NTP_PACKET_SIZE) < NTP_PACKET_SIZE) {
				
				USE_SERIAL.println("error");
				break;
			}
			else {
				// Extract seconds portion.
				unsigned long secs;
				secs  = packetBuffer[40] << 24;
				secs |= packetBuffer[41] << 16;
				secs |= packetBuffer[42] <<  8;
				secs |= packetBuffer[43];
				// UTC is 1 TimeZone correction when no daylight saving time
				return(secs - 2208988800UL + NTP_TIMEZONES * SECS_IN_HOUR);
			}
			_Udp.flush();	
		}
		delay(100);									// Wait 100 millisecs, allow kernel to act when necessary
    }
	_Udp.flush();
	
	return(0); 										
}


/*************************************************************************************************
 * 功 能 : 送网关数据情况
 *************************************************************************************************/
void lora_server_class::sendstat() {

    uint8_t status_report[STATUS_SIZE]; 					
    char stat_timestamp[32];								
    time_t t;
    int stat_index=0;
	uint8_t token_h   = (uint8_t)rand(); 					
    uint8_t token_l   = (uint8_t)rand();					

    status_report[0]  = PROTOCOL_VERSION;					
	status_report[1]  = token_h;
    status_report[2]  = token_l;
    status_report[3]  = PKT_PUSH_DATA;						
	sn.read(&status_report[4]);
    stat_index = 12;										
    t = now();												
	sprintf(stat_timestamp, "%04d-%02d-%02d %02d:%02d:%02d CET", year(),month(),day(),hour(),minute(),second());
	USE_SERIAL.print(stat_timestamp);
	int ackr = 0;
//	USE_SERIAL.printf("rxnb:%d,rxok%d,dwnb%d,txnb%d,ack:%d \r\n",
//	lora_even.cp_nb_rx_rcv,lora_even.cp_nb_rx_ok,lora_even.cp_dw_dgram_rcv,lora_even.cp_nb_tx_ok,lora_even.cp_up_ack_rcv);
	int j = snprintf((char *)(status_report + stat_index), STATUS_SIZE-stat_index, 
		"{\"stat\":{\"time\":\"%s\",\"lati\":%s,\"long\":%s,\"alti\":%i,\"rxnb\":%u,\"rxok\":%u,\"rxfw\":%u,\"ackr\":%u.0,\"dwnb\":%u,\"txnb\":%u}}", 
		stat_timestamp,"26.5639","118.2120",VERSION_NUM,lora_even.cp_nb_rx_rcv, lora_even.cp_nb_rx_ok, lora_even.cp_nb_rx_rcv, 
		lora_even.cp_up_ack_rcv, lora_even.cp_dw_dgram_rcv, lora_even.cp_nb_tx_ok);
//	USE_SERIAL.printf("%s",(char *)(status_report + stat_index));
	lora_even.cp_nb_rx_rcv = 0;
	lora_even.cp_nb_rx_ok = 0;
	lora_even.cp_up_ack_rcv = 0;
	lora_even.cp_dw_dgram_rcv = 0;
	lora_even.cp_nb_tx_ok = 0;

    stat_index += j;
    status_report[stat_index] = 0; 							// add string terminator, for safety
    sendUdp(ttnServer, server_port, status_report, stat_index);
	return;
}

/*************************************************************************************************
 * 功 能 :lorawan心跳连接
 *************************************************************************************************/
void lora_server_class::pullData() {

    uint8_t pullDataReq[12]; 								
    int pullIndex=0;
	int i;
	
	uint8_t token_h = (uint8_t)rand(); 						
    uint8_t token_l = (uint8_t)rand();						
	
    pullDataReq[0]  = PROTOCOL_VERSION;						
    pullDataReq[1]  = token_h;
    pullDataReq[2]  = token_l;
    pullDataReq[3]  = PKT_PULL_DATA;						
	sn.read(&pullDataReq[4]);
    pullIndex = 12;								
	
	uint8_t *pullPtr;
	pullPtr = pullDataReq,
	sendUdp(ttnServer, server_port, pullDataReq, pullIndex);
	return;
}

/*************************************************************************************************
 * 功 能 : lorawan 数据机理处理
 *************************************************************************************************/
void lora_server_class::data_deal()
{
	static bool just_once = false;
	static bool just_once_switch = false;
	uint32_t nowSeconds = now();
	int packetSize;
	
	lora_even.rx_stuts_deal();//获取lora物理层状态机制处理
	if(!wifi_ok) 
	{
		if(wps.gw.mifi_flg==false)//MIFI版本不运行4G网络
		{
		    gprs.run();//自动连接
    		if(gprs.network()==INIT_OK)
    	    {
    			read_gprs_Udp();
    			if(just_once_switch==true)//重新连接ACK
    			{
    				just_once = false;
    				just_once_switch = false;
    			}
    	        if(just_once==false)
            	{
            		gprs.get_time();
            		sendstat(); 									
            		pullData(); 	
    				nowSeconds = now();								
            		just_once = true;
    				ntptimer = nowSeconds;
    				statTime = nowSeconds;
    				pulltime = nowSeconds;
            	}
    			if (nowSeconds - ntptimer >= _NTP_INTERVAL) 
    			{
            		gprs.get_time();
    				ntptimer = nowSeconds;
    			}
                if ((nowSeconds - statTime) >= _STAT_INTERVAL*2) 
            	{	
                    sendstat();										
            		statTime = nowSeconds;
                }
            	
                if ((nowSeconds - pulltime) >= _PULL_INTERVAL) 
            	{	
                    pullData();										
            		pulltime = nowSeconds;
                }
    	    }
		}
	}
	else
	{
		if(just_once_switch==false)//重新连接ACK
		{
		    just_once_switch = true;
			just_once = false;
		}
	    if(just_once==false)
    	{
    		time_t newTime;
    		newTime = (time_t)getNtpTime();
    		if (newTime != 0) 
    		{
    			USE_SERIAL.println(ctime(&newTime));
    		    setTime(newTime);
    		}
    		sendstat(); 									
    		pullData(); 									
    		just_once = true;
			nowSeconds = now(); 							
			ntptimer = nowSeconds;
			statTime = nowSeconds;
			pulltime = nowSeconds;
    	}
    	
    	while( (packetSize = _Udp.parsePacket()) > 0) 
    	{
    		if (readUdp(packetSize) <= 0) {
    			break;
    		}
    	}
    	
    	if (nowSeconds - ntptimer >= _NTP_INTERVAL) 
    	{
    		time_t newTime;
    		newTime = (time_t)getNtpTime();
    		if (newTime != 0) setTime(newTime);
    		ntptimer = nowSeconds;
    	}
    	
        if ((nowSeconds - statTime) >= _STAT_INTERVAL) 
    	{	
            sendstat();										
    		statTime = nowSeconds;
        }
    	
        if ((nowSeconds - pulltime) >= _PULL_INTERVAL) 
    	{	
            pullData();										
    		pulltime = nowSeconds;
        }
	}
}

/*************************************************************************************************
 * 功 能 : 初始化 读取系统参数 对lora硬件参数配置初始化
 *************************************************************************************************/
void lora_server_class::start()
{
	server_port = wps.gw.port;
	server_name = wps.gw.severip;
	sn.start();
//	sn.set_ynl_freq(&wps.gw.lora_freq);
//	wps.gw.lora_tx_freq = wps.gw.lora_freq;
	USE_SERIAL.printf("lora_freq:%d,  lora_sf:%d  \r\n",wps.gw.lora_freq,wps.gw.lora_sf);
	USE_SERIAL.printf("sever_ip:%s, sever_port:%d \r\n",(char*)server_name.c_str(),server_port);
	lora.begin(wps.gw.lora_freq,wps.gw.lora_sf);
	UDPconnect();
}
/*************************************************************************************************
 * 功 能 : 获取域名IP
 *************************************************************************************************/
bool lora_server_class::server_get_host()
{
	bool flag = false;
	bool flag1 = false;
	if(!ntpServer)
    {
        if (!WiFi.hostByName(NTP_TIMESERVER, ntpServer))		
    	{
    		USE_SERIAL.print("ntp1.aliyun.com give ip ");
    		if (!WiFi.hostByName(NTP_TIMESERVER1, ntpServer))	
    		{
    		    USE_SERIAL.print("ntp2.aliyun.com give ip ");
    		    if (!WiFi.hostByName(NTP_TIMESERVER2, ntpServer))	
    		    {
    		    	USE_SERIAL.print("cn.pool.ntp.org give ip");
            		flag = true;
    		    }
    		}
    	};
    }
	USE_SERIAL.printf("ntpServer:");
	USE_SERIAL.println(ntpServer);
	if(!ttnServer)
	{
		USE_SERIAL.println("GET IP");
		if (!WiFi.hostByName(server_name.c_str(), ttnServer))			
		{
			USE_SERIAL.print("host name1  give ip ");
			if (!WiFi.hostByName(server_name.c_str(), ttnServer))
			{
				USE_SERIAL.print("host name  give ip ");
				flag1 = true;
			}
		};
			
	}
	USE_SERIAL.printf("ttnServer:");
	USE_SERIAL.println(ttnServer);
	if(flag1||flag)
	{
	    return false;
	}
	else
	{
		return true;
	}
}
/*************************************************************************************************
 * 功 能 : 设置wifi状态
 *************************************************************************************************/
void lora_server_class::set_wifi_status(bool wifi_status)
{
	wifi_ok = wifi_status;
}

lora_server_class lorawan;

