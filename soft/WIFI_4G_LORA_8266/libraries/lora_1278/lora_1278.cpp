/******************************************************************************

 				 版权所有 (C), 2017-2027, 福州晴川科技有限公司

 ******************************************************************************
  文 件 名   : lora_1278.cpp
  版 本 号   : 初稿
  作    者   : zenghi
  联系方式 : 309966755@qq.com
  生成日期   : 2019-04-15
  功能描述   : lora 1278驱动层
******************************************************************************/
//#include "all.h"
#include "lora_1278.h"
#include "qc_telnet.h"

#define USE_SERIAL Serial
bool have_staus = false;
void ICACHE_RAM_ATTR Interrupt_0()
{
	have_staus = true;
//	Serial.println("\n  Interrupt_0");
}
void ICACHE_RAM_ATTR Interrupt_1()
{
	have_staus = true;
//	Serial.println("\n  Interrupt_1");
}
void ICACHE_RAM_ATTR Interrupt_2()
{
//	have_staus = true;
//	USE_SERIAL.println("\n  Interrupt_2");
}

/*************************************************************************************************
 * 功 能 : SPI写寄存器
 *************************************************************************************************/
uint8_t lora_1278::readRegister(uint8_t addr)
{
	SPI.beginTransaction(readSettings);				
    digitalWrite(lora_pins.ss, LOW);					
	SPI.transfer(addr & 0x7F);
	uint8_t res = (uint8_t) SPI.transfer(0x00);
    digitalWrite(lora_pins.ss, HIGH);				
	SPI.endTransaction();
    return((uint8_t) res);
}
/*************************************************************************************************
 * 功 能 : SPI写寄存器
 *************************************************************************************************/
void lora_1278::readBuffer(uint8_t addr, uint8_t *buf, uint8_t len)
{
	SPI.beginTransaction(readSettings);				
    digitalWrite(lora_pins.ss, LOW);					
	SPI.transfer(addr & 0x7F);
	
    for(uint8_t i = 0; i < len; i++ )
	{
	    buf[i] = (uint8_t) SPI.transfer(0x00);
	}
    digitalWrite(lora_pins.ss, HIGH);				
	SPI.endTransaction();
//    return((uint8_t) res);
}
/*************************************************************************************************
 * 功 能 : SPI写寄存器
 *************************************************************************************************/
void lora_1278::writeRegister(uint8_t addr, uint8_t value)
{
	SPI.beginTransaction(writeSettings);
	digitalWrite(lora_pins.ss, LOW);					
	SPI.transfer((addr | 0x80) & 0xFF);
	SPI.transfer(value & 0xFF);
    digitalWrite(lora_pins.ss, HIGH);				
	SPI.endTransaction();
}

/*************************************************************************************************
 * 功 能 : SPI写buf
 *************************************************************************************************/
void lora_1278::writeBuffer(uint8_t addr, uint8_t *buf, uint8_t len)
{
	SPI.beginTransaction(writeSettings);
	digitalWrite(lora_pins.ss, LOW);					
	SPI.transfer((addr | 0x80) & 0xFF);			
	for (uint8_t i=0; i<len; i++) {
		SPI.transfer(buf[i] & 0xFF);
	}
    digitalWrite(lora_pins.ss, HIGH);				
	SPI.endTransaction();
}

/*************************************************************************************************
 * 功 能 : 设置收发无线频点
 *************************************************************************************************/
void  lora_1278::setFreq(uint32_t freq)
{
    uint64_t frf = ((uint64_t)freq << 19) / 32000000;
    writeRegister(REG_FRF_MSB, (uint8_t)(frf>>16) );
    writeRegister(REG_FRF_MID, (uint8_t)(frf>> 8) );
    writeRegister(REG_FRF_LSB, (uint8_t)(frf>> 0) );
	return;
}

/*************************************************************************************************
 * 功 能 : 设置TX功率
 *************************************************************************************************/
void lora_1278::setPow(uint8_t powe)
{

    uint8_t paConfig = 0;
    uint8_t paDac = 0;

	if(powe>=14)
	{
	    powe = 20;
	}

    paConfig = readRegister( REG_PAC );
    paDac = readRegister( REG_PADAC );

    paConfig = ( paConfig & RF_PACONFIG_PASELECT_MASK ) | RF_PACONFIG_PASELECT_PABOOST;
    paConfig = ( paConfig & RF_PACONFIG_MAX_POWER_MASK ) | 0x70;

    if( ( paConfig & 0x80 ) == 0x80 )
    {
        if( powe > 17 )
        {
            paDac = ( paDac & RF_PADAC_20DBM_MASK ) | RF_PADAC_20DBM_ON;
        }
        else
        {
            paDac = ( paDac & RF_PADAC_20DBM_MASK ) | RF_PADAC_20DBM_OFF;
        }
        if( ( paDac & RF_PADAC_20DBM_ON ) == RF_PADAC_20DBM_ON )
        {
            if( powe < 5 )
            {
                powe = 5;
            }
            if( powe > 20 )
            {
                powe = 20;
            }
            paConfig = ( paConfig & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( powe - 5 ) & 0x0F );
        }
        else
        {
            if( powe < 2 )
            {
                powe = 2;
            }
            if( powe > 17 )
            {
                powe = 17;
            }
            paConfig = ( paConfig & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( powe - 2 ) & 0x0F );
        }
    }
    else
    {
        if( powe < -1 )
        {
            powe = -1;
        }
        if( powe > 14 )
        {
            powe = 14;
        }
        paConfig = ( paConfig & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( powe + 1 ) & 0x0F );
    }
    writeRegister( REG_PAC, paConfig );
    writeRegister( REG_PADAC, paDac );
	
	return;
}



/*************************************************************************************************
 * 功 能 : 设置TX RX模式开关状态
 *************************************************************************************************/
void  lora_1278::opmode(uint8_t mode)
{
	if(mode==OPMODE_TX)
	{
		digitalWrite(lora_pins.vc1,HIGH );
		digitalWrite(lora_pins.vc2,LOW );
	}
	else
	{
		digitalWrite(lora_pins.vc1,LOW );
		digitalWrite(lora_pins.vc2,HIGH );
	}

	if (mode == OPMODE_LORA) 
		writeRegister(REG_OPMODE, (uint8_t) mode);
	else
	    writeRegister(REG_OPMODE, (uint8_t)((readRegister(REG_OPMODE) & ~OPMODE_MASK) | mode));
	
}

/*************************************************************************************************
 * 功 能 : 设置扩频因子
 *************************************************************************************************/
void lora_1278::setRate(uint8_t sf, uint8_t crc) 
{
	uint8_t mc1=0, mc2=0, mc3=0;

	if (sf==SF8) 
	{
		mc1= 0x78;				// SX1276_MC1_BW_125==0x70 | SX1276_MC1_CR_4_8==0x08
	}
	else 
	{
		mc1= 0x72;				// SX1276_MC1_BW_125==0x70 | SX1276_MC1_CR_4_5==0x02
	}
	mc2= ((sf<<4) | crc) & 0xFF; // crc is 0x00 or 0x04==SX1276_MC2_RX_PAYLOAD_CRCON
	mc3= 0x04;				// 0x04; SX1276_MC3_AGCAUTO
	if (sf == SF11 || sf == SF12) 
	{ 
		mc3|= 0x08; 
	}		
	
	writeRegister(REG_MODEM_CONFIG1, (uint8_t) mc1);
	writeRegister(REG_MODEM_CONFIG2, (uint8_t) mc2);
	writeRegister(REG_MODEM_CONFIG3, (uint8_t) mc3);

	if (sf == SF10 || sf == SF11 || sf == SF12)
	{
	    writeRegister(REG_SYMB_TIMEOUT_LSB, (uint8_t) 0x05);
	} 
	else 
	{
		writeRegister(REG_SYMB_TIMEOUT_LSB, (uint8_t) 0x08);
	}
	return;
}

/*************************************************************************************************
 * 功 能 : 送loraserver TX数据
 *************************************************************************************************/
void lora_1278::txLoraModem(uint8_t *payLoad, uint8_t payLength, uint32_t tmst, uint8_t sfTx,
						uint8_t powe, uint32_t freq, uint8_t crc, uint8_t iiq)
{
		
	// Assert the value of the current mode
//	ASSERT((readRegister(REG_OPMODE) & OPMODE_LORA) != 0);
	
	// 2. enter standby mode (required for FIFO loading))
	opmode(OPMODE_STANDBY);									// set 0x01 to 0x01

#if 0
	uint32_t waitTime =  micros();
	/* 超时使用RX2窗口 by zenghi */
	if(waitTime+200000> tmst)
	{
	    sfTx = SF12;
		tmst+=1000000;//使用RX2窗口
	}
#endif// endif 0

	// 3. Init spreading factor and other Modem setting
	setRate(sfTx, crc);

	#ifndef __QC
	writeRegister( 0x20, ( 0 >> 8 ) & 0x00FF );
	writeRegister( 0x21, 8 & 0xFF );
	#endif /* __QC */
	// Frquency hopping
	//writeRegister(REG_HOP_PERIOD, (uint8_t) 0x00);		// set 0x24 to 0x00 only for receivers
	
	// 4. Init Frequency, config channel
	setFreq(freq);

	// 6. Set power level, REG_PAC
	setPow(powe);
	
	// 7. prevent node to node communication
	writeRegister(REG_INVERTIQ, (uint8_t) iiq);						// 0x33, (0x27 or 0x40)
	
	// 8. set the IRQ mapping DIO0=TxDone DIO1=NOP DIO2=NOP (or lesss for 1ch gateway)
    writeRegister(REG_DIO_MAPPING_1, (uint8_t)(
		MAP_DIO0_LORA_TXDONE | 
		MAP_DIO1_LORA_NOP | 
		MAP_DIO2_LORA_NOP |
		MAP_DIO3_LORA_CRC));
	// 9. clear all radio IRQ flags
    writeRegister(REG_IRQ_FLAGS, (uint8_t) 0xFF);
	
	// 10. mask all IRQs but TxDone
    writeRegister(REG_IRQ_FLAGS_MASK, (uint8_t) ~IRQ_LORA_TXDONE_MASK);
	
	// txLora
	opmode(OPMODE_FSTX);									// set 0x01 to 0x02 (actual value becomes 0x82)
	
	// 11, 12, 13, 14. write the buffer to the FiFo
	sendPkt(payLoad, payLength);
	// 15. wait extra delay out. The delayMicroseconds timer is accurate until 16383 uSec.
//	USE_SERIAL.print(F("tmst:"));
//	USE_SERIAL.print(tmst);
#if 0
	loraWait(tmst);
#endif// endif 0

	//Set the base addres of the transmit buffer in FIFO
	writeRegister(REG_FIFO_ADDR_PTR, (uint8_t) readRegister(REG_FIFO_TX_BASE_AD));	// set 0x0D to 0x0F (contains 0x80);	
	
	//For TX we have to set the PAYLOAD_LENGTH
	writeRegister(REG_PAYLOAD_LENGTH, (uint8_t) payLength);		// set 0x22, max 0x40==64Byte long
	
	//For TX we have to set the MAX_PAYLOAD_LENGTH
	writeRegister(REG_MAX_PAYLOAD_LENGTH, (uint8_t) MAX_PAYLOAD_LENGTH);	// set 0x22, max 0x40==64Byte long
	
	// Reset the IRQ register
	writeRegister(REG_IRQ_FLAGS_MASK, (uint8_t) 0x00);			// Clear the mask
	writeRegister(REG_IRQ_FLAGS, (uint8_t) IRQ_LORA_TXDONE_MASK);// set 0x12 to 0x08
	
	// 16. Initiate actual transmission of FiFo
	opmode(OPMODE_TX);											// set 0x01 to 0x03 (actual value becomes 0x83)
//	USE_SERIAL.print(F("go:"));


}// txLoraModem


/*************************************************************************************************
 * 功 能 : 进入RX模式
 *************************************************************************************************/
void lora_1278::rxLoraModem()
{
	// 1. Put system in LoRa mode
	//opmode(OPMODE_LORA);
	writeRegister(REG_IRQ_FLAGS_MASK, (uint8_t) 0x00);	// Reset the interrupt mask
	writeRegister(REG_IRQ_FLAGS, (uint8_t) 0xFF);	
	// 2. Put the radio in sleep mode
//	opmode(OPMODE_STANDBY);
	opmode(OPMODE_SLEEP);										// set 0x01 to 0x00
	opmode(OPMODE_LORA);										// set register 0x01 to 0x80
	// 3. Set frequency based on value in freq
	setFreq(net_freqs);										// set to 868.1MHz

	// 4. Set spreading Factor and CRC
    setRate(net_sf, 0x04);
	
	// prevent node to node communication
	writeRegister(REG_INVERTIQ, (uint8_t) 0x27);				// 0x33, 0x27; to reset from TX
	
	// Max Payload length is dependent on 256 byte buffer. 
	// At startup TX starts at 0x80 and RX at 0x00. RX therefore maximized at 128 Bytes
	//For TX we have to set the PAYLOAD_LENGTH
    //writeRegister(REG_PAYLOAD_LENGTH, (uint8_t) PAYLOAD_LENGTH);	// set 0x22, 0x40==64Byte long

	// Set CRC Protection or MAX payload protection
	//writeRegister(REG_MAX_PAYLOAD_LENGTH, (uint8_t) MAX_PAYLOAD_LENGTH);	// set 0x23 to 0x80==128
//	writeRegister(REG_MAX_PAYLOAD_LENGTH,0xff);	// set 0x23 to 0x80==128 bytes
//	writeRegister(REG_PAYLOAD_LENGTH,0xff);			// 0x22, 0x40==64Byte long
	writeRegister(REG_MAX_PAYLOAD_LENGTH,MAX_PAYLOAD_LENGTH);	// set 0x23 to 0x80==128 bytes
	writeRegister(REG_PAYLOAD_LENGTH,PAYLOAD_LENGTH);			// 0x22, 0x40==64Byte long
	//Set the start address for the FiFO (Which should be 0)
	writeRegister(REG_FIFO_ADDR_PTR, (uint8_t) readRegister(REG_FIFO_RX_BASE_AD));	// set 0x0D to 0x0F (contains 0x00);

//	writeRegister(REG_FIFO_ADDR_PTR, (uint8_t) readRegister(REG_FIFO_RX_BASE_AD));	// set reg 0x0D to 0x0F
#ifndef __QC
	writeRegister( 0x20, ( 150 >> 8 ) & 0x00FF );
	writeRegister( 0x21, 150 & 0xFF );
#endif /* __QC */


	
	// Low Noise Amplifier used in receiver
	writeRegister(REG_LNA, (uint8_t) LNA_MAX_GAIN);  						// 0x0C, 0x23
	
	// Accept no interrupts except RXDONE, RXTOUT en RXCRC
	writeRegister(REG_IRQ_FLAGS_MASK, (uint8_t) ~(
		IRQ_LORA_RXDONE_MASK | 
		IRQ_LORA_RXTOUT_MASK | 
		IRQ_LORA_HEADER_MASK | 
		IRQ_LORA_CRCERR_MASK));
	// set frequency hopping
	writeRegister(REG_HOP_PERIOD,0x00);						// 0x24, 0x00 was 0xFF
	// Set RXDONE interrupt to dio0
	writeRegister(REG_DIO_MAPPING_1, (uint8_t)(
			MAP_DIO0_LORA_RXDONE | 
			MAP_DIO1_LORA_RXTOUT |
			MAP_DIO2_LORA_NOP |			
			MAP_DIO3_LORA_CRC));

	// Set Continous Receive Mode, usefull if we stay on one SF
//	_state= S_RX;
	opmode(OPMODE_RX);										// 0x80 | 0x05 (listen)
	
	// 9. clear all radio IRQ flags
    writeRegister(REG_IRQ_FLAGS, 0xFF);
	
	return;
}
/*************************************************************************************************
 * 功 能 : 无
 *************************************************************************************************/
void lora_1278::rxLoraModem_loop()
{
    
}


/*************************************************************************************************
 * 功 能 : 校准
 *************************************************************************************************/
void lora_1278::RxChainCalibration( void )
{
    uint8_t regPaConfigInitVal;
    uint32_t initialFreq;

    // Save context
    regPaConfigInitVal = readRegister( REG_PACONFIG );
    initialFreq = ( double )( ( ( uint32_t )readRegister( REG_FRFMSB ) << 16 ) |
                              ( ( uint32_t )readRegister( REG_FRFMID ) << 8 ) |
                              ( ( uint32_t )readRegister( REG_FRFLSB ) ) ) * ( double )FREQ_STEP;

    // Cut the PA just in case, RFO output, power = -1 dBm
    writeRegister( REG_PACONFIG, 0x00 );

    // Launch Rx chain calibration for LF band
    writeRegister( REG_IMAGECAL, ( readRegister( REG_IMAGECAL ) & RF_IMAGECAL_IMAGECAL_MASK ) | RF_IMAGECAL_IMAGECAL_START );
    while( ( readRegister( REG_IMAGECAL ) & RF_IMAGECAL_IMAGECAL_RUNNING ) == RF_IMAGECAL_IMAGECAL_RUNNING )
    {
    }

    // Sets a Frequency in HF band
    setFreq( net_freqs );

    // Launch Rx chain calibration for HF band
    writeRegister( REG_IMAGECAL, ( readRegister( REG_IMAGECAL ) & RF_IMAGECAL_IMAGECAL_MASK ) | RF_IMAGECAL_IMAGECAL_START );
    while( ( readRegister( REG_IMAGECAL ) & RF_IMAGECAL_IMAGECAL_RUNNING ) == RF_IMAGECAL_IMAGECAL_RUNNING )
    {
    }

    // Restore context
    writeRegister( REG_PACONFIG, regPaConfigInitVal );
    setFreq( initialFreq );
}
/*************************************************************************************************
 * 功 能 : lora 1278初始化
 *************************************************************************************************/
void lora_1278::begin()
{
	static bool just_once = false;
//	net_freqs = freqs;
//	net_sf = sf;
	if(just_once==false)
	{
	    pinMode(lora_pins.vc1, OUTPUT);
    	pinMode(lora_pins.vc2, OUTPUT);
        pinMode(lora_pins.dio0, INPUT);								// This pin is interrupt
    	pinMode(lora_pins.dio1, INPUT);								// This pin is interrupt
    	pinMode(lora_pins.rst, OUTPUT);
	    pinMode(lora_pins.ss, OUTPUT);
		attachInterrupt(lora_pins.dio0, Interrupt_0, RISING);	// Separate interrupts
		attachInterrupt(lora_pins.dio1, Interrupt_1, RISING);	// Separate interrupts		
		attachInterrupt(lora_pins.rst,  Interrupt_2, FALLING);	// Separate interrupts	

    	SPI.begin();
    	delay(500);

    	just_once = true;
	}
	detachInterrupt(lora_pins.rst);
	digitalWrite(lora_pins.rst, LOW);
	delayMicroseconds(1000);
	digitalWrite(lora_pins.rst, HIGH);
	delayMicroseconds(1000);
	pinMode(lora_pins.rst, INPUT_PULLUP);
	attachInterrupt(lora_pins.rst,	Interrupt_2, FALLING);	// Separate interrupts	

	RxChainCalibration();
	// 2. Set radio to sleep
	opmode(OPMODE_SLEEP);										// set register 0x01 to 0x00

	// 1 Set LoRa Mode
	opmode(OPMODE_LORA);										// set register 0x01 to 0x80
	
	setFreq(net_freqs);												// set to 868.1MHz or the last saved frequency
	
	// 4. Set spreading Factor
    setRate(net_sf, 0x04);
	
	// Low Noise Amplifier used in receiver
    writeRegister(REG_LNA, (uint8_t) LNA_MAX_GAIN);  			// 0x0C, 0x23
	
    uint8_t version = readRegister(REG_VERSION);				// Read the LoRa chip version id
    if (version == 0x22) {
        USE_SERIAL.printf("SX1272\r\n");
        // sx1272
//        sx1272 = true;
    } 
	else if (version == 0x12) {
        USE_SERIAL.printf("SX1276\r\n");
//        sx1272 = false;
	} 
	else {
        USE_SERIAL.printf("lora ERROR\r\n");
		while(1);
    }
	// If we are here, the chip is recognized successfully
	
	// 7. set sync word
	writeRegister(REG_SYNC_WORD, (uint8_t) 0x34);				// set 0x39 to 0x34 LORA_MAC_PREAMBLE
	
	// prevent node to node communication
	writeRegister(REG_INVERTIQ,0x27);							// 0x33, 0x27; to reset from TX
	
	// Max Payload length is dependent on 256 byte buffer. At startup TX starts at
	// 0x80 and RX at 0x00. RX therefore maximized at 128 Bytes
	writeRegister(REG_MAX_PAYLOAD_LENGTH,MAX_PAYLOAD_LENGTH);	// set 0x23 to 0x80==128 bytes
	writeRegister(REG_PAYLOAD_LENGTH,PAYLOAD_LENGTH);			// 0x22, 0x40==64Byte long
//	writeRegister(REG_MAX_PAYLOAD_LENGTH,0xff);	// set 0x23 to 0x80==128 bytes
//	writeRegister(REG_PAYLOAD_LENGTH,0xff);			// 0x22, 0x40==64Byte long
	writeRegister(REG_FIFO_ADDR_PTR, (uint8_t) readRegister(REG_FIFO_RX_CURRENT_ADDR));	// set reg 0x0D to 0x0F
	writeRegister(REG_HOP_PERIOD,0x00);							// reg 0x24, set to 0x00

	// 5. Config PA Ramp up time								// set reg 0x0A  
	writeRegister(REG_PARAMP, (readRegister(REG_PARAMP) & 0xF0) | 0x08); // set PA ramp-up time 50 uSec
	
	// Set 0x4D PADAC for SX1276 ; XXX register is 0x5a for sx1272
	//High Power
	writeRegister(REG_PADAC_SX1276,  0x87); 					// set 0x4D (PADAC) to 0x84
	//writeRegister(REG_PADAC, readRegister(REG_PADAC) | 0x4);
#if 1
	#ifndef __QC
	writeRegister( 0x20, ( 150 >> 8 ) & 0x00FF );
	writeRegister( 0x21, 150 & 0xFF );
	#endif /* __QC */
#endif// endif 0
	// Reset interrupt Mask, enable all interrupts
	writeRegister(REG_IRQ_FLAGS_MASK, 0x00);
	
	// 9. clear all radio IRQ flags
    writeRegister(REG_IRQ_FLAGS, 0xFF);

	rxLoraModem();
}

/*************************************************************************************************
 * 功 能 : lora 1278初始化
 *************************************************************************************************/
void lora_1278::begin(uint32_t freqs,sf_t sf)
{
	static bool just_once = false;
	net_freqs = freqs;
	net_sf = sf;
	if(just_once==false)
	{
	    pinMode(lora_pins.vc1, OUTPUT);
    	pinMode(lora_pins.vc2, OUTPUT);
        pinMode(lora_pins.dio0, INPUT);								// This pin is interrupt
    	pinMode(lora_pins.dio1, INPUT);								// This pin is interrupt
    	pinMode(lora_pins.rst, OUTPUT);
	    pinMode(lora_pins.ss, OUTPUT);
		attachInterrupt(lora_pins.dio0, Interrupt_0, RISING);	// Separate interrupts
		attachInterrupt(lora_pins.dio1, Interrupt_1, RISING);	// Separate interrupts		
		attachInterrupt(lora_pins.rst,  Interrupt_2, FALLING);	// Separate interrupts	

    	SPI.begin();
    	delay(500);

    	just_once = true;
	}
	detachInterrupt(lora_pins.rst);
	digitalWrite(lora_pins.rst, LOW);
	delayMicroseconds(1000);
	digitalWrite(lora_pins.rst, HIGH);
	delayMicroseconds(1000);
	pinMode(lora_pins.rst, INPUT_PULLUP);
	attachInterrupt(lora_pins.rst,	Interrupt_2, FALLING);	// Separate interrupts	

	RxChainCalibration();
	// 2. Set radio to sleep
	opmode(OPMODE_SLEEP);										// set register 0x01 to 0x00

	// 1 Set LoRa Mode
	opmode(OPMODE_LORA);										// set register 0x01 to 0x80
	
	setFreq(net_freqs);												// set to 868.1MHz or the last saved frequency
	
	// 4. Set spreading Factor
    setRate(net_sf, 0x04);
	
	// Low Noise Amplifier used in receiver
    writeRegister(REG_LNA, (uint8_t) LNA_MAX_GAIN);  			// 0x0C, 0x23
	
    uint8_t version = readRegister(REG_VERSION);				// Read the LoRa chip version id
    if (version == 0x22) {
        USE_SERIAL.printf("SX1272\r\n");
        // sx1272
//        sx1272 = true;
    } 
	else if (version == 0x12) {
        USE_SERIAL.printf("SX1276\r\n");
//        sx1272 = false;
	} 
	else {
        USE_SERIAL.printf("ERROR\r\n");
		while(1);
    }
	// If we are here, the chip is recognized successfully
	
	// 7. set sync word
	writeRegister(REG_SYNC_WORD, (uint8_t) 0x34);				// set 0x39 to 0x34 LORA_MAC_PREAMBLE
	
	// prevent node to node communication
	writeRegister(REG_INVERTIQ,0x27);							// 0x33, 0x27; to reset from TX
	
	// Max Payload length is dependent on 256 byte buffer. At startup TX starts at
	// 0x80 and RX at 0x00. RX therefore maximized at 128 Bytes
	writeRegister(REG_MAX_PAYLOAD_LENGTH,MAX_PAYLOAD_LENGTH);	// set 0x23 to 0x80==128 bytes
	writeRegister(REG_PAYLOAD_LENGTH,PAYLOAD_LENGTH);			// 0x22, 0x40==64Byte long
//	writeRegister(REG_MAX_PAYLOAD_LENGTH,0xff);	// set 0x23 to 0x80==128 bytes
//	writeRegister(REG_PAYLOAD_LENGTH,0xff);			// 0x22, 0x40==64Byte long
	
	writeRegister(REG_FIFO_ADDR_PTR, (uint8_t) readRegister(REG_FIFO_RX_CURRENT_ADDR));	// set reg 0x0D to 0x0F
	writeRegister(REG_HOP_PERIOD,0x00);							// reg 0x24, set to 0x00

	// 5. Config PA Ramp up time								// set reg 0x0A  
	writeRegister(REG_PARAMP, (readRegister(REG_PARAMP) & 0xF0) | 0x08); // set PA ramp-up time 50 uSec
	
	// Set 0x4D PADAC for SX1276 ; XXX register is 0x5a for sx1272
	//High Power
	writeRegister(REG_PADAC_SX1276,  0x87); 					// set 0x4D (PADAC) to 0x84
	//writeRegister(REG_PADAC, readRegister(REG_PADAC) | 0x4);
#if 1
	#ifndef __QC
	writeRegister( 0x20, ( 150 >> 8 ) & 0x00FF );
	writeRegister( 0x21, 150 & 0xFF );
	#endif /* __QC */
#endif// endif 0
	// Reset interrupt Mask, enable all interrupts
	writeRegister(REG_IRQ_FLAGS_MASK, 0x00);
	
	// 9. clear all radio IRQ flags
    writeRegister(REG_IRQ_FLAGS, 0xFF);

	rxLoraModem();
}// initLoraModem





/*************************************************************************************************
 * 功 能 : 得到无线数据读取
 *************************************************************************************************/
uint8_t lora_1278::receivePkt(uint8_t *payload)
{
    uint8_t irqflags = readRegister(REG_IRQ_FLAGS);			
    //  Check for payload IRQ_LORA_CRCERR_MASK=0x20 set
    if (irqflags & IRQ_LORA_CRCERR_MASK)
    {
		USE_SERIAL.printf("RX crcerr\r\n");
		writeRegister(REG_IRQ_FLAGS, IRQ_LORA_CRCERR_MASK);
		return 0;
    }
	else if ((irqflags & IRQ_LORA_HEADER_MASK) == false)
    {
		USE_SERIAL.printf("RX HEADER err\r\n");
		// Reset VALID-HEADER flag 0x10
        writeRegister(REG_IRQ_FLAGS, IRQ_LORA_HEADER_MASK);	
        return 0;
    }
	else if (irqflags & IRQ_LORA_RXDONE_MASK)
	{
		writeRegister(REG_IRQ_FLAGS, IRQ_LORA_RXDONE_MASK);	
        uint8_t receivedCount = readRegister(REG_RX_NB_BYTES);			
        writeRegister(REG_FIFO_ADDR_PTR, (uint8_t) readRegister(REG_FIFO_RX_CURRENT_ADDR));		
		if (receivedCount > PAYLOAD_LENGTH) {
			receivedCount=PAYLOAD_LENGTH;
			USE_SERIAL.print(F("over data\r\n"));
		}
		readBuffer(REG_FIFO,payload,receivedCount);
		return(receivedCount);
    }
	writeRegister(REG_IRQ_FLAGS, (uint8_t) (
		IRQ_LORA_RXDONE_MASK | 
		IRQ_LORA_RXTOUT_MASK |
		IRQ_LORA_HEADER_MASK | 
		IRQ_LORA_CRCERR_MASK));							// 0x12; Clear RxDone IRQ_LORA_RXDONE_MASK
    return 0;
}
/*************************************************************************************************
 * 功 能 : lora txbuf数据区 写入进行物理发射
 *************************************************************************************************/
bool lora_1278::sendPkt(uint8_t *payLoad, uint8_t payLength)
{
	writeRegister(REG_FIFO_ADDR_PTR, (uint8_t) readRegister(REG_FIFO_TX_BASE_AD));	// 0x0D, 0x0E
	
	writeRegister(REG_PAYLOAD_LENGTH, (uint8_t) payLength);				// 0x22
	payLoad[payLength] = 0x00;
	writeBuffer(REG_FIFO, (uint8_t *) payLoad, payLength);
	return true;
}


/*************************************************************************************************
 * 功 能 : lora等待延时后进行发射，保证时间窗口一致
 *************************************************************************************************/
void lora_1278::loraWait(uint32_t tmst)
{
	uint32_t startMics = micros();						// Start of the loraWait function
	tmst += txDelay;
	
	uint32_t waitTime =  micros();
	if(waitTime> tmst)
	{
	    return;
	}
	else
	{
		waitTime = tmst - waitTime;
	}
	if(waitTime>10000000)
	{
	    return;
	}

	while (waitTime > 16000) {
		delay(15);										// ms delay including yield, slightly shorter
		waitTime =  micros();
		if(waitTime> tmst)
		{
			return;
		}
		else
		{
			waitTime = tmst - waitTime;
		}
		
	}
	if (waitTime>0) delayMicroseconds(waitTime);
}
/*************************************************************************************************
 * 功 能 : 获取lora 状态  轮询处理
 *************************************************************************************************/
lora_status lora_1278::get_stuts()
{
	if(!have_staus)
	{
		return LORA_RX;
	}
	uint8_t flags = readRegister(REG_IRQ_FLAGS);
	uint8_t mask  = readRegister(REG_IRQ_FLAGS_MASK);
	uint8_t intr  = flags & ( ~ mask );				// Only react on non masked interrupts

	if (intr & IRQ_LORA_TXDONE_MASK) 
	{
		have_staus = 0;
		return LORA_TX_DONE;
	}

	if(intr & IRQ_LORA_RXDONE_MASK)
	{
		if(intr & IRQ_LORA_CRCERR_MASK)
		{
			USE_SERIAL.printf("LORA_RX_CRCERR:%d",intr);
			have_staus = 0;
			return LORA_RX_CRCERR;
		}
//		USE_SERIAL.printf("LORAdone \r\n");
		have_staus = 0;
	    return LORA_RX_DONE;
	}
	if (intr & IRQ_LORA_RXTOUT_MASK) 
	{
		USE_SERIAL.printf("LORA_RX_TOUT:%d",intr);
		have_staus = 0;
		return LORA_RX_TOUT;
	}
	if (intr & IRQ_LORA_HEADER_MASK)
	{
		USE_SERIAL.printf("HEADER:%d",intr);
	}
	else if (intr)
	{
		USE_SERIAL.printf("error:%d",intr);
		return LORA_ERROR;
	}
	return LORA_RX;
}

/*************************************************************************************************
 * 功 能 : 获取rssi
 *************************************************************************************************/
int16_t lora_1278::get_rssi()
{
	int8_t snr = 0;
	int8_t snr1 = 0;
	snr1 = readRegister( REG_PKT_SNR_VALUE );
	if( snr1 & 0x80 ) // The SNR sign bit is 1
	{
		// Invert and divide by 4
		snr = ( ( ~snr1 + 1 ) & 0xFF ) >> 2;
		snr = -snr;
	}
	else
	{
		// Divide by 4
		snr = ( snr1 & 0xFF ) >> 2;
	}
	
	int16_t rssi = readRegister( REG_PKT_RSSI );
	if( snr < 0 )
	{
		{
			return MYRSSI_OFFSET_LF + rssi + ( rssi >> 4 ) +
														  snr;
		}
	}
	else
	{
		{
			return MYRSSI_OFFSET_LF + rssi + ( rssi >> 4 );
		}
	}

}

/*************************************************************************************************
 * 功 能 : 获取snr
 *************************************************************************************************/
int8_t lora_1278::get_snr()
{
	return readRegister( REG_PKT_SNR_VALUE );
}

/*************************************************************************************************
 * 功 能 : 获取SF扩频
 *************************************************************************************************/
int8_t lora_1278::get_sf()
{
	return readRegister(REG_MODEM_CONFIG2) >> 4;
}

lora_1278 lora;//lora外部引用

#if 0
void ICACHE_RAM_ATTR Interrupt_0()
{
}

void ICACHE_RAM_ATTR Interrupt_1()
{
}

void ICACHE_RAM_ATTR Interrupt_2() 
{
	goto_wps_flag=true;
}
#endif// endif 0

