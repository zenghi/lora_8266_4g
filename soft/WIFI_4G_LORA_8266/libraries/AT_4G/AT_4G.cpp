/******************************************************************************

 				 版权所有 (C), 2017-2027, 福州晴川科技有限公司

 ******************************************************************************
  文 件 名   : AT_4G.cpp
  版 本 号   : 初稿
  作    者   : zenghi
  联系方式 : 309966755@qq.com
  生成日期   : 2019-04-23
  功能描述   : AT 4G模块
******************************************************************************/
#include "AT_4G.h"
#include <TimeLib.h>				
#include <time.h>  


#define USE_SERIAL Serial
#define GPRS_SERIAL  Serial1 //\"zenghi.com\",1700"
#define HOST_NAME "AT+CIPSTART=\"udp\","
static char u8RxData[1024];
static uint16_t u8RxCnt = 0;
/**********************************************C************************************************************/
extern "C"{
#include <stdio.h>
typedef bool (*at_func_ptr_t)(void * arg);
typedef struct 
{
    const char * cmd_string;
    const char * cmd_return;
    const char * cmd_return2;
    at_func_ptr_t   cmd_fun;
    void *arg;
}AT_CMD_4G;
static AT_4G_STATUS_ENUM init_falg = INIT_ING;

/*************************************************************************************************
 * 功 能 : 回去网络质量回调
 *************************************************************************************************/
bool get_csq(void *arg)
{
	char *data = (char *)arg;
	if(data!=NULL)
	{
		USE_SERIAL.printf("CSQ@@@:%s",data);
		USE_SERIAL.printf("#####:");
		USE_SERIAL.write(data[0]);
		USE_SERIAL.write(data[1]);
		USE_SERIAL.write(data[2]);
		USE_SERIAL.write(data[3]);
		USE_SERIAL.write(data[4]);
		USE_SERIAL.write(data[5]);
		USE_SERIAL.write(data[6]);
		if(strstr(u8RxData,"+CSQ: 0,99")!=NULL)
		{
			USE_SERIAL.printf("CSQerror",data);
			return false;
		}
//		delay(3000);
	}
	return true;
}
/*************************************************************************************************
 * 功 能 : 复位回调
 *************************************************************************************************/
bool AT_RESET(void *arg)
{
	char *data = (char *)arg;
	if(data!=NULL)
	{
		USE_SERIAL.printf("RESET:%s",data);
	}
	return true;
}
/*************************************************************************************************
 * 功 能 : 波特率修改回调
 *************************************************************************************************/
bool irp_switch(void *arg)
{
	USE_SERIAL.printf("irp_switch");
	GPRS_SERIAL.printf("%s\r\n","AT+IPR=921600");
	delay(1000);
	GPRS_SERIAL.begin(921600);
	return true;
}

//初始化动作参数
static AT_CMD_4G fun_num[] = 
{
//    { "AT+IPR=921600",  "OK",irp_switch,  NULL},//获取系统参数
    { "ATE0",            "OK","ALREADY CONNECT",irp_switch,  NULL},//获取系统参数
    { "AT+IPR=921600",   "OK","ALREADY CONNECT",NULL,  NULL},//获取系统参数
    { "ATE0",            "OK","ALREADY CONNECT",NULL,  NULL},//获取系统参数
    { "AT+CIPQSEND=1",   "OK","ALREADY CONNECT",NULL,  NULL},//复位
    { "AT+ICCID",        "89","ALREADY CONNECT",   NULL,  NULL},//  89860 中国卡号   只识别89国际编号
    { "AT+CSQ",          "+CSQ:","ALREADY CONNECT",  get_csq,  NULL},//获取系统参数
    { HOST_NAME,         "CONNECT OK","ALREADY CONNECT",   NULL,  NULL},//获取系统参数
    { "END@@##",        "END@@##","ALREADY CONNECT",   AT_RESET,  NULL},//获取系统参数
};
	
/*************************************************************************************************
 * 功 能 :  循环获取模块送过来的数据
 *************************************************************************************************/
static inline int get_uart_data()
{
	char data;
	int len;
	len = GPRS_SERIAL.available();
	if(len > 0)
	{
		delay(1);
		if(len!=GPRS_SERIAL.available()&&len!=265)//还有数据等待数据完毕再一次性调用,0xff buf满就不等待了
		{
//			GPRS_SERIAL.printf("have other data\r\n");
			return false;
		}
		len = GPRS_SERIAL.available();
		if(u8RxCnt+len>sizeof(u8RxData))
		{
		    len = sizeof(u8RxData) - u8RxCnt;//溢出概率存在轮询速度不够的情况就会丢失溢出数据
		}
//		GPRS_SERIAL.printf("u8RxCnt:%d\r\n",u8RxCnt);
//		GPRS_SERIAL.printf("len:%d\r\n",len);
		GPRS_SERIAL.readBytes(&u8RxData[u8RxCnt], len);
		u8RxCnt+=len;
		if(u8RxCnt>1)
		{
			if(u8RxData[u8RxCnt-2]=='\r'&&u8RxData[u8RxCnt-1]=='\n')
			{
				u8RxData[u8RxCnt]=0;
				len = u8RxCnt;
				u8RxCnt = 0;
				return len;
			}
		}
		if(u8RxCnt>=sizeof(u8RxData))
		{ 
			USE_SERIAL.printf("overflow\r\n");
			u8RxCnt = 0;
			u8RxData[0]=0;
			return false;
		}
	}
	return false;
}
/*************************************************************************************************
 * 功 能 : 清内部uart 数据
 *************************************************************************************************/
static inline void clear_uart_data()
{
	char data;
	GPRS_SERIAL.flush();
    while (GPRS_SERIAL.available() > 0) 
	{
		data = char(GPRS_SERIAL.read());
	}
	u8RxCnt = 0;
	u8RxData[0]=0;
}
/*************************************************************************************************
 * 功 能 : 循环运行  进行初始化动作  无线程 做循环等待机制    代码标志位有点多注意区分
 *************************************************************************************************/
static bool loop_fun_num()
{
	static bool init_once = false;
	static bool send_falg = false;
	static time_t now_time;
	static bool need_delay_falg=false;
	static long i = 0;
	static int conut = 0;
	static int csq_conut = 0;
	static int err_conut = 0;
	static AT_CMD_4G* tc_fun_num = &fun_num[0];
	
	//等待3S标志位动作
	if(need_delay_falg)
	{
	    if(now() - now_time < 3)
		{
		    return false;
		}
		else
		{
		    need_delay_falg = false;
		}
	}

	//初始化进行
	if(init_falg==INIT_ING)
	{
		if(init_once==false)//重新初始化
		{
		    gprs.open_pow();
			GPRS_SERIAL.begin(115200);
			init_once = true;
			tc_fun_num = &fun_num[0];
			send_falg = false;
			conut =0;
		}
		if(send_falg==false)//发一次数据等待结果
		{
		    clear_uart_data();
			if(strstr(HOST_NAME,tc_fun_num->cmd_string)!=NULL)
			{
			    char cmd[128]={0};
				snprintf(cmd,128,"AT+CIPSTART=\"udp\",\"%s\",%d",(char*)gprs.gprs_severip.c_str(),gprs.gprs_port); 
				GPRS_SERIAL.printf("%s\r\n",cmd);
			}
			else
    		{
    		    GPRS_SERIAL.printf("%s\r\n",tc_fun_num->cmd_string);
    		}
		}
		if(conut<5)//发送CMD等待结果，5次不成功认为失败
		{
    		if(++i<90000ul)//轮询等待结果 根据CPU loop 运行数据时间有所改变 大概等待2S
    		{
    			if(get_uart_data())
    			{
    			    if(strstr(u8RxData,tc_fun_num->cmd_return)!=NULL
						|| strstr(u8RxData,tc_fun_num->cmd_return2)!=NULL)
        			{	
        				if(tc_fun_num->cmd_fun!=NULL)
        				{
        				    if(tc_fun_num->cmd_fun(&u8RxData[2])==false)
							{
								i = 0;
								send_falg = false;
								conut = 0;
								now_time = now();
								need_delay_falg = true;
								if(++csq_conut>10)//等待CSQ网络OK
								{
									csq_conut = 0;
									init_once = false;
									i=0;
									send_falg = false;
									gprs.close_pow();
									now_time = now();
									need_delay_falg = true;
									USE_SERIAL.printf("4g error\r\n");//未获取争取响应
									if(++err_conut>3)
									{
										init_falg = INIT_ERR;
									}
								}
								return false;
							}
							else
							{
								now_time = now();
								need_delay_falg = true;
							}
        				}
        				if((tc_fun_num+1)!=NULL)//进入顺序初始化
        				{
        					USE_SERIAL.printf("go new\r\n");
							conut = 0;
        					i=0;
        					send_falg = false;
        					tc_fun_num = tc_fun_num + 1;
        					if(strstr(tc_fun_num->cmd_string,"END@@##")!=NULL)//结尾标识符  后续想个更简单办法
        					{
        						USE_SERIAL.printf("4g init ok\r\n");
        						init_falg = INIT_OK;
        					}
        					return true;
        				}
        			}
    			}
    			send_falg = true;//继续查询不重新发送数据
    			return false;
    		}
			i = 0;
			send_falg = false;
			conut++;//连续发送5次不成功的话认为失败
			return false;
		}

		//断电重新进行初始化
		init_once = false;
		i=0;
		send_falg = false;
		gprs.close_pow();
		now_time = now();
		need_delay_falg = true;
		USE_SERIAL.printf("4g error\r\n");//未获取争取响应
		if(tc_fun_num==&fun_num[0]||tc_fun_num==&fun_num[2])//没设备或者没卡的情况进入错误状态
		{
		    if(++err_conut>3)
			{
			    init_falg = INIT_ERR;
			}
		}
	}
	else
	{
	}
	return false;
}
}
/**********************************************C************************************************************/



/*************************************************************************************************
 * 功 能 : 送数据到服务器
 *************************************************************************************************/
int AT_4G_CLASS::send(char * data, int len)
{
	static bool sendFlg = false;
	char end = 0x1a;
	if(init_falg==INIT_OK)
	{
	    if (sendFlg == false) 
    	{
    		sendFlg = true;	// 防止重入
    		GPRS_SERIAL.write((uint8_t *)&end, 1);
    		delay(1);
    		GPRS_SERIAL.write((uint8_t *)"\r\n",sizeof("\r\n"));
    		delay(5);
    		GPRS_SERIAL.write((uint8_t *)"AT+CIPSEND\r\n",sizeof("AT+CIPSEND"));
    		delay(5);
    		GPRS_SERIAL.write((uint8_t *)data, len);
    		delay(5);
    		GPRS_SERIAL.write((uint8_t *)&end, 1);
    		sendFlg = false;
    		return true;
    	}
	}
	return false;
}

/*************************************************************************************************
 * 功 能 : 获取服务器数据
 *************************************************************************************************/
int AT_4G_CLASS::recv(char* rxbuffer)
{
	int len = 0;
	if(init_falg==INIT_OK)
	{
	    len = get_uart_data();
    	if(len)
    	{
    		memset(rxbuffer,0 , 1024);
    		memcpy(rxbuffer,u8RxData , len);
    	}
	}
	return len;
}


/*************************************************************************************************
 * 功 能 : 初始化内部参数
 * 输 入 : [1]String severip   lora 域名或者IP
 *	       [2]int port         lora端口
 *	       [3]uint8_t pow_pin  电源控制关键  低有效
 * 返 回 : 无
 * 说 明 : 外部调用
 *************************************************************************************************/
bool AT_4G_CLASS::start(String severip,int port,uint8_t pow_pin)
{
	power_pin = pow_pin;
	gprs_severip=severip;	
	gprs_port = port;			
	open_pow();
	GPRS_SERIAL.begin(115200);
	return true;
}
/*************************************************************************************************
 * 功 能 : C++调用C
 * 说 明 : 循环调用机理
 *************************************************************************************************/
bool AT_4G_CLASS::run()
{
	loop_fun_num();
	return true;
}
/*************************************************************************************************
 * 功 能 : 获取4G网络情况
 *************************************************************************************************/
AT_4G_STATUS_ENUM AT_4G_CLASS::network()
{
	return init_falg;
}
/*************************************************************************************************
 * 功 能 : 获取4G网络时间更新到系统内部
 *************************************************************************************************/
bool AT_4G_CLASS::get_time()
{
//	struct tm * timeinfo;
//	struct time now;
	char timedata[2] = {0};
	char *data = NULL;
	GPRS_SERIAL.printf("AT+CCLK?\r\n");
	delay(10);
	if(get_uart_data())
	{
		data = strstr(u8RxData,"+CCLK: \"");
		if(data!=NULL)
		{

		struct tm tm;
		timedata[0]=data[8];
		timedata[1]=data[9];
		timedata[2]=0;
	    tm.tm_year = (int)atoi(timedata) + 100;
		timedata[0]=data[11];
		timedata[1]=data[12];
		timedata[2]=0;
	    tm.tm_mon = (int)atoi(timedata) - 1;
		timedata[0]=data[14];
		timedata[1]=data[15];
		timedata[2]=0;
	    tm.tm_mday = (int)atoi(timedata);
		timedata[0]=data[17];
		timedata[1]=data[18];
		timedata[2]=0;
	    tm.tm_hour =  (int)atoi(timedata);
		timedata[0]=data[20];
		timedata[1]=data[21];
		timedata[2]=0;
	    tm.tm_min = (int)atoi(timedata);
		timedata[0]=data[23];
		timedata[1]=data[24];
		timedata[2]=0;
	    tm.tm_sec = (int)atoi(timedata);

		
		USE_SERIAL.printf("time:%s\r\n",&data[8]);//未获取争取响应
		USE_SERIAL.printf("tm_year:%d\r\n",tm.tm_year);//未获取争取响应
		USE_SERIAL.printf("tm_mon:%d\r\n",tm.tm_mon);//未获取争取响应
		USE_SERIAL.printf("tm_mday:%d\r\n",tm.tm_mday);//未获取争取响应
		USE_SERIAL.printf("tm_hour:%d\r\n",tm.tm_hour);//未获取争取响应
		USE_SERIAL.printf("tm_min:%d\r\n",tm.tm_min);//未获取争取响应
		USE_SERIAL.printf("tm_sec:%d\r\n",tm.tm_sec);//未获取争取响应
	    time_t t = mktime(&tm);
		
		USE_SERIAL.println(ctime(&t));
		setTime(t);
		}
	}
	return true;
}
AT_4G_CLASS gprs;

