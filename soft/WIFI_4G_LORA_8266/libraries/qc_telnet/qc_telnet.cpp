/******************************************************************************

 				 版权所有 (C), 2017-2027, 福州晴川科技有限公司

 ******************************************************************************
  文 件 名   : qc_telnet.cpp
  版 本 号   : 初稿
  作    者   : zenghi
  联系方式 : 309966755@qq.com
  生成日期   : 2019-04-20
  功能描述   : 添加AT指令 进行远程修改参数
******************************************************************************/
#include "qc_telnet.h"
#include "wps_www_server.h"
#include "lora_1278.h"
//#include "AT_4G.h"

#define USE_SERIAL Serial
WiFiServer server(23); 


extern "C"{
typedef void (*at_func_ptr_t)(void * arg);
typedef struct 
{
    const char * cmd_string;
    at_func_ptr_t   cmd_fun;
    void *arg;
}AT_CMD_STRU;

/*************************************************************************************************
 * 功 能 : TX延时参数微调
 *************************************************************************************************/
static void AT_TXDELAY(void* arg)
{
    char * data = (char *)arg;
	int dealy;			
    if(*data >='0'&&*data <='9'  || *data =='-')
	{
		dealy =  (int)atoi(data);
		if(dealy>-100e3&&dealy<100e3)
	    {
	        wps.gw.tx_dealy = dealy;
    		telnet.printf("ok,AT+TXDELAY=%d  us\r\n",wps.gw.tx_dealy);
	    }
		else
		{
			telnet.printf("ERROR\r\n");
		}
	}
    else if(*data == '?')
    {
		telnet.printf("AT+TXDELAY=%d us\r\n",wps.gw.tx_dealy);
    }
	else
	{
		telnet.printf("ERROR\r\n");
	}
	
}

/*************************************************************************************************
 * 功 能 : help帮助说明
 *************************************************************************************************/
static void AT_HELP(void* arg)
{
    char * data = (char *)arg;
	sf_t  tc_sf;
	telnet.printf("AT\r\n");
	telnet.printf("REBOOT           (reboot sys)\r\n");
	telnet.printf("AT+SAVE          (save configuration)\r\n");
	telnet.printf("AT+SF=x          (x:7-12                  ?:return now sf)\r\n");
	telnet.printf("AT+TXFQ=x        (x:0 Server frequency | 400000000~5250000000  ?:return now freq)\r\n");
	telnet.printf("AT+RXFQ=x        (x:400000000~5250000000  ?:return now freq)\r\n");
	telnet.printf("AT+TXDELAY=x     (x:100000 ~ -100000 us   ?:return now txdelay)\r\n");
	telnet.printf("AT+SSID=x        (x:ssid name             ?:return now ssid)\r\n");
	telnet.printf("AT+PASS=x        (x:pass word             ?:return now pass)\r\n");
	telnet.printf("AT+SERVERIP=x    (x:xxx.com x.x.x.x       ?:return now server ip)\r\n");
	telnet.printf("AT+SERVERPORT=x  (x:1700                  ?:return now server port)\r\n");
}
/*************************************************************************************************
 * 功 能 : 重启
 *************************************************************************************************/
static void REBOOT(void* arg)
{
    char * data = (char *)arg;
	telnet.printf("REBOOTING\r\n");
	while(1);
}
/*************************************************************************************************
 * 功 能 : 设置wifi网络名称
 *************************************************************************************************/
static void AT_SSID(void* arg)
{
    char * data = (char *)arg;
    if(*data == '?')
    {
        telnet.printf("ssid:%s\r\n",(char *)wps.gw.ssid.c_str());
		return;
    }
	wps.gw.ssid = data;
	telnet.printf("ok,ssid:%s  \r\n",(char *)wps.gw.ssid.c_str());
}
/*************************************************************************************************
 * 功 能 : 设置wifi网络密码
 *************************************************************************************************/
static void AT_PASS(void* arg)
{
    char * data = (char *)arg;
    if(*data == '?')
    {
        telnet.printf("pass:%s\r\n",(char *)wps.gw.pass.c_str());
		return;
    }
	wps.gw.pass = data;
	telnet.printf("ok,pass:%s  \r\n",(char *)wps.gw.pass.c_str());
}
/*************************************************************************************************
 * 功 能 : 设置服务器IP
 *************************************************************************************************/
static void AT_SERVER_IP(void* arg)
{
    char * data = (char *)arg;
	
    if(*data == '?')
    {
        telnet.printf("server_ip:%s\r\n",(char *)wps.gw.severip.c_str());
		return;
    }
	wps.gw.severip = data;
	telnet.printf("ok,severip:%s  \r\n",(char *)wps.gw.severip.c_str());
}
/*************************************************************************************************
 * 功 能 : 设置服务器端口
 *************************************************************************************************/
static void AT_SERVER_PORT(void* arg)
{
    char * data = (char *)arg;
    if(*data >='0'&&*data <='9')
	{
	    wps.gw.port = (int)atoi(data);
    	telnet.printf("ok,port:%d\r\n",(char *)wps.gw.port);
	}
    else if(*data == '?')
    {
        telnet.printf("port:%d\r\n",wps.gw.port);
    }
	else
	{
        telnet.printf("ERROR\r\n");
	}
}


/*************************************************************************************************
 * 功 能 : 保存系统参数
 *************************************************************************************************/
static void AT_SAVE(void* arg)
{
    char * data = (char *)arg;
	sf_t  tc_sf;
	wps.write_ttn_bak();
	telnet.printf("save ok\r\n");
}

/*************************************************************************************************
 * 功 能 : 设置扩频因子
 *************************************************************************************************/
static void AT_SF(void* arg)
{
    char * data = (char *)arg;
	sf_t  tc_sf;
    if(*data >='0'&&*data <='9')
    {
        tc_sf = (sf_t)atoi(data);
        if(tc_sf>=SF7&&tc_sf<=SF12)
        {
            wps.gw.lora_sf = tc_sf;//DR_5;
            telnet.printf("SF%d,ok\r\n",wps.gw.lora_sf);
			lora.begin(wps.gw.lora_freq,wps.gw.lora_sf);
        }
        else
        {
            telnet.printf("ERROR\r\n");
        }
    }
    else if(*data == '?')
    {
        telnet.printf("SF%d\r\n",wps.gw.lora_sf);
    }
    else
    {
        telnet.printf("ERROR\r\n");
    }
}
/*************************************************************************************************
 * 功 能 : 设置接收频点
 *************************************************************************************************/
static void AT_RXFQ(void* arg)
{
    char * data = (char *)arg;
    uint32_t fq;
    if( (*data >='0'&&*data <='9') )
    {
        fq = atoi(data);
//        if((fq>=470.0e6&& fq<=525.0e6)  || (fq>=430.0e6&& fq<=440.0e6 ))
		  if((fq>=400.0e6&& fq<=525.0e6)  )
        {
            wps.gw.lora_freq = fq;
            telnet.printf("%09d,ok\r\n",fq);
			lora.begin(wps.gw.lora_freq,wps.gw.lora_sf);
        }
        else
        {
            telnet.printf("ERROR\r\n");
        }
    }
    else if(*data == '?')
    {
        telnet.printf("RXFQ:%d\r\n",wps.gw.lora_freq);
    }
    else
    {
        telnet.printf("ERROR\r\n");
    }
}
/*************************************************************************************************
 * 功 能 : 设置下行频点 为0则使用服务器频点
 *************************************************************************************************/
static void AT_TXFQ(void* arg)
{
    char * data = (char *)arg;
    uint32_t fq;
    if( (*data >='0'&&*data <='9') )
    {
        fq = atoi(data);
//        if((fq>=470.0e6&& fq<=525.0e6)  || (fq>=430.0e6&& fq<=440.0e6 ))
		  if((fq>=400.0e6&& fq<=525.0e6)  || fq==0)
        {
            wps.gw.lora_freq = fq;
            telnet.printf("%09d,ok\r\n",fq);
        }
        else
        {
            telnet.printf("ERROR\r\n");
        }
    }
    else if(*data == '?')
    {
    	if(wps.gw.lora_tx_freq)
		{
			telnet.printf("TXFQ:%d\r\n",wps.gw.lora_tx_freq);
		}
		else
		{
   		    telnet.printf("TXFQ:Server frequency\r\n");
		}
    }
    else
    {
        telnet.printf("ERROR\r\n");
    }
}
/*************************************************************************************************
 * 功 能 : AT功能
 *************************************************************************************************/
static void AT(void* arg)
{
    telnet.printf("OK\r\n");
}
/*************************************************************************************************
 * 功 能 : AT指令
 *************************************************************************************************/
static const AT_CMD_STRU fun_num[] = 
{
    { "HELP",           AT_HELP,          NULL},//获取系统参数
    { "help",           AT_HELP,          NULL},//获取系统参数
    { "REBOOT",         REBOOT,           NULL},//获取系统参数
    { "AT+TXDELAY=",    AT_TXDELAY,       NULL},//获取系统参数
    { "AT+SF=",         AT_SF,            NULL},//获取系统参数
    { "AT+TXFQ=",       AT_RXFQ,          NULL},//复位
    { "AT+RXFQ=",       AT_RXFQ,          NULL},//复位
    { "AT+SAVE",        AT_SAVE,          NULL},//获取系统参数
    { "AT+SSID=",        AT_SSID,          NULL},//获取系统参数
    { "AT+PASS=",        AT_PASS,          NULL},//获取系统参数
    { "AT+SERVERIP=",    AT_SERVER_IP,     NULL},//获取系统参数
    { "AT+SERVERPORT=",  AT_SERVER_PORT,   NULL},//获取系统参数
    { "AT",             AT,               NULL},//获取系统参数
};
static char u8RxData[185];
static uint8_t u8RxCnt = 0;
/*************************************************************************************************
 * 功 能 : AT命令回调
 *************************************************************************************************/
static void INIT_SECTION(void)
{
	uint8_t len = 0;
	uint8_t i = 0;
	{
		for(i = 0;i<sizeof(fun_num)/sizeof(AT_CMD_STRU);i++)
		{
			len = strlen(fun_num[i].cmd_string);
			if(memcmp(fun_num[i].cmd_string,u8RxData,len)==0)
			{
				if(fun_num[i].cmd_fun!=NULL)
				{
				    fun_num[i].cmd_fun(&u8RxData[len]);
				}
				break;
			}
		}
	}
}
/*************************************************************************************************
 * 功 能 : 获取TELNET的数据
 *************************************************************************************************/
static inline void get_telnet_data(uint8_t data)
{
	{
		u8RxData[u8RxCnt] = data;
		u8RxCnt++;
		if(u8RxCnt>1)
		{
			if(u8RxData[u8RxCnt-2]=='\r'&&u8RxData[u8RxCnt-1]=='\n')
			{
				u8RxData[u8RxCnt]=0;
				u8RxCnt = 0;
				INIT_SECTION();
				return;
			}
		}
		if(u8RxCnt>=sizeof(u8RxData))
		{ 
			telnet.printf("overflow\r\n");
			u8RxCnt = 0;
		}
	}
}
}

/*************************************************************************************************
 * 功 能 : 重写printf 回调参数 使得printf具备TELNET功能
 *************************************************************************************************/
size_t qc_telnet_class::write(uint8_t c) 
{
	USE_SERIAL.write(c);
	if (serverClients[0])
	{
	    serverClients[0].write(c);
	}
	return 0;
//	return qc_telnet_class.serverClients[0].write(c,1);
}
/*************************************************************************************************
 * 功 能 : 重写printf 回调参数 使得printf具备TELNET功能
 *************************************************************************************************/
size_t qc_telnet_class::write(const uint8_t *buffer, size_t size) 
{
	int i = 0;
	for(i=0;i<size;i++)
	{
	    USE_SERIAL.write(buffer[i]);
	}
	if (serverClients[0])
	{
	    return serverClients[0].write((char *)buffer,size);
	}
	else
	{
	    return 0;
	}
}


/*************************************************************************************************
 * 功 能 : 开机服务启动
 *************************************************************************************************/
void qc_telnet_class::start() 
{
    server.begin();
    server.setNoDelay(true);
}
/*************************************************************************************************
 * 功 能 : 打印logo
 *************************************************************************************************/
void qc_telnet_class::printf_logo() 
{
    telnet.printf("*********************************************************\r\n");
    telnet.printf("Welcome to qingchuan %s gw LTS (GNU QC1278 V1.0)\r\n",__DATE__);
    telnet.printf("*     Qingchuan Technology Co., Ltd.\r\n");
    telnet.printf("* Support:      http://zenghi.com\r\n");
    telnet.printf("* AUTHOR:       黄瀚霆 zenghi \r\n");
    telnet.printf("* EMAIL:        309966755@qq.com\r\n");
    telnet.printf("*********************************************************\r\n");
    telnet.printf("***Use the help command to view the help documentation***\r\n");
}

/*************************************************************************************************
 * 功 能 : telnet 循环运行
 *************************************************************************************************/
void qc_telnet_class::loop() 
{
	uint8_t data;
  //check if there are any new clients
  if (server.hasClient()) 
  	{
    //find free/disconnected spot
    int i;
    for (i = 0; i < MAX_SRV_CLIENTS; i++)
      if (!serverClients[i]) { // equivalent to !serverClients[i].connected()
        serverClients[i] = server.available();
		printf_logo();
        break;
      }

    //no free/disconnected spot so reject
    if (i == MAX_SRV_CLIENTS) {
      server.available().println("busy");
      // hints: server.available() is a WiFiClient with short-term scope
      // when out of scope, a WiFiClient will
      // - flush() - all data will be sent
      // - stop() - automatically too
      USE_SERIAL.printf("server is busy with %d active connections\r\n", MAX_SRV_CLIENTS);
    }
  }

  //check TCP clients for data
  for (int i = 0; i < MAX_SRV_CLIENTS; i++)
    while (serverClients[i].available() && USE_SERIAL.availableForWrite() > 0) {
      // working char by char is not very efficient
      data = serverClients[i].read();
      get_telnet_data(data);
      USE_SERIAL.write(data);
    }

  // determine maximum output size "fair TCP use"
  // client.availableForWrite() returns 0 when !client.connected()
  size_t maxToTcp = 0;
  for (int i = 0; i < MAX_SRV_CLIENTS; i++)
    if (serverClients[i]) {
      size_t afw = serverClients[i].availableForWrite();
      if (afw) {
        if (!maxToTcp) {
          maxToTcp = afw;
        } else {
          maxToTcp = std::min(maxToTcp, afw);
        }
      } else {
        // warn but ignore congested clients
        USE_SERIAL.println("one client is congested");
      }
    }

  //check UART for data
#if 0
  size_t len = std::min((size_t)USE_SERIAL.available(), maxToTcp);
  len = std::min(len, (size_t)STACK_PROTECTOR);
  if (len) {
    uint8_t sbuf[len];
    size_t serial_got = USE_SERIAL.readBytes(sbuf, len);
    // push UART data to all connected telnet clients
    for (int i = 0; i < MAX_SRV_CLIENTS; i++)
      // if client.availableForWrite() was 0 (congested)
      // and increased since then,
      // ensure write space is sufficient:
      if (serverClients[i].availableForWrite() >= serial_got) {
        size_t tcp_sent = serverClients[i].write(sbuf, serial_got);
        if (tcp_sent != len) {
          USE_SERIAL.printf("len mismatch: available:%zd serial-read:%zd tcp-write:%zd\r\n", len, serial_got, tcp_sent);
        }
      }
  }
#endif// endif 0
}
//qc_telnet_class22::qc_telnet_class22()
//{}

qc_telnet_class telnet;//lora外部引用
//HardwareSerial1 ok;
//qq ok;//lora外部引用

