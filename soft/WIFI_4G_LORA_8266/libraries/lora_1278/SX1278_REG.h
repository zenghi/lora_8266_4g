/******************************************************************************

 				 版权所有 (C), 2017-2027, 福州晴川科技有限公司

 ******************************************************************************
  文 件 名   : SX1278_REG.h
  版 本 号   : 初稿
  作    者   : zenghi
  联系方式 : 309966755@qq.com
  生成日期   : 2019-04-14
  功能描述   : SX1278_REG.h 的头文件
******************************************************************************/
#ifndef __SX1278_REG_H__
#define __SX1278_REG_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define PAYLOAD_LENGTH              0x40		// 64 bytes
#define MAX_PAYLOAD_LENGTH          0x80		// 128 bytes
	
#define REG_FIFO                    0x00		// rw FIFO address
#define REG_OPMODE                  0x01
	// Register 2 to 5 are unused for LoRa
#define REG_FRF_MSB					0x06
#define REG_FRF_MID					0x07
#define REG_FRF_LSB					0x08
#define REG_PAC                     0x09
#define REG_PARAMP                  0x0A
#define REG_LNA                     0x0C
#define REG_FIFO_ADDR_PTR           0x0D		// rw SPI interface address pointer in FIFO data buffer
#define REG_FIFO_TX_BASE_AD         0x0E		// rw write base address in FIFO data buffer for TX modulator
#define REG_FIFO_RX_BASE_AD         0x0F		// rw read base address in FIFO data buffer for RX demodulator (0x00)
	
#define REG_FIFO_RX_CURRENT_ADDR    0x10		// r  Address of last packet received
#define REG_IRQ_FLAGS_MASK          0x11
#define REG_IRQ_FLAGS               0x12
#define REG_RX_NB_BYTES             0x13
#define REG_PKT_SNR_VALUE			0x19
#define REG_PKT_RSSI				0x1A		// latest package
#define REG_RSSI					0x1B		// Current RSSI, section 6.4, or  5.5.5
#define REG_HOP_CHANNEL				0x1C
#define REG_MODEM_CONFIG1           0x1D
#define REG_MODEM_CONFIG2           0x1E
#define REG_SYMB_TIMEOUT_LSB  		0x1F
	
#define REG_PAYLOAD_LENGTH          0x22
#define REG_MAX_PAYLOAD_LENGTH 		0x23
#define REG_HOP_PERIOD              0x24
#define REG_MODEM_CONFIG3           0x26
#define REG_RSSI_WIDEBAND			0x2C
	
#define REG_INVERTIQ				0x33
#define REG_DET_TRESH				0x37		// SF6
#define REG_SYNC_WORD				0x39
#define REG_TEMP					0x3C
	
#define REG_DIO_MAPPING_1           0x40
#define REG_DIO_MAPPING_2           0x41
#define REG_VERSION	  				0x42
	
#define REG_PADAC					0x5A
#define REG_PADAC_SX1272			0x5A
#define REG_PADAC_SX1276			0x4D
	
	
	// ----------------------------------------
	// opModes
#define SX72_MODE_SLEEP             0x80
#define SX72_MODE_STANDBY           0x81
#define SX72_MODE_FSTX              0x82
#define SX72_MODE_TX                0x83		// 0x80 | 0x03
#define SX72_MODE_RX_CONTINUOS      0x85
	
	// ----------------------------------------
	// LMIC Constants for radio registers
#define OPMODE_LORA      			0x80
#define OPMODE_MASK      			0x07
#define OPMODE_SLEEP     			0x00
#define OPMODE_STANDBY   			0x01
#define OPMODE_FSTX      			0x02
#define OPMODE_TX        			0x03
#define OPMODE_FSRX      			0x04
#define OPMODE_RX        			0x05
#define OPMODE_RX_SINGLE 			0x06
#define OPMODE_CAD       			0x07
	
	
	
	// ----------------------------------------
	// LOW NOISE AMPLIFIER
	
#define LNA_MAX_GAIN                0x23		// Max gain 0x20 | Boost 0x03
#define LNA_OFF_GAIN                0x00
#define LNA_LOW_GAIN		    	0x20
	
	// CONF REG
#define REG1                        0x0A
#define REG2                        0x84
	
	// ----------------------------------------
	// MC1 sx1276 RegModemConfig1
#define SX1276_MC1_BW_125           0x70
#define SX1276_MC1_BW_250           0x80
#define SX1276_MC1_BW_500           0x90
#define SX1276_MC1_CR_4_5           0x02
#define SX1276_MC1_CR_4_6           0x04
#define SX1276_MC1_CR_4_7           0x06
#define SX1276_MC1_CR_4_8           0x08
#define SX1276_MC1_IMPLICIT_HEADER_MODE_ON  0x01
	
#define SX72_MC1_LOW_DATA_RATE_OPTIMIZE     0x01 	// mandated for SF11 and SF12
	
	// ----------------------------------------
	// MC2 definitions
#define SX72_MC2_FSK                0x00
#define SX72_MC2_SF7                0x70		// SF7 == 0x07, so (SF7<<4) == SX7_MC2_SF7
#define SX72_MC2_SF8                0x80
#define SX72_MC2_SF9                0x90
#define SX72_MC2_SF10               0xA0
#define SX72_MC2_SF11               0xB0
#define SX72_MC2_SF12               0xC0
	
	// ----------------------------------------
	// MC3
#define SX1276_MC3_LOW_DATA_RATE_OPTIMIZE  0x08
#define SX1276_MC3_AGCAUTO                 0x04
	
	// ----------------------------------------
	// FRF
#define FRF_MSB						0xD9		// 868.1 Mhz
#define FRF_MID						0x06
#define FRF_LSB						0x66
	
	// ----------------------------------------
	// DIO function mappings					 D0D1D2D3
#define MAP_DIO0_LORA_RXDONE   		0x00  // 00------ bit 7 and 6
#define MAP_DIO0_LORA_TXDONE   		0x40  // 01------
#define MAP_DIO0_LORA_CADDONE  		0x80  // 10------
#define MAP_DIO0_LORA_NOP   		0xC0  // 11------
	
#define MAP_DIO1_LORA_RXTOUT   		0x00  // --00---- bit 5 and 4
#define MAP_DIO1_LORA_FCC			0x10  // --01----
#define MAP_DIO1_LORA_CADDETECT		0x20  // --10----
#define MAP_DIO1_LORA_NOP      		0x30  // --11----
	
#define MAP_DIO2_LORA_FCC0      	0x00  // ----00-- bit 3 and 2
#define MAP_DIO2_LORA_FCC1      	0x04  // ----01-- bit 3 and 2
#define MAP_DIO2_LORA_FCC2      	0x08  // ----10-- bit 3 and 2
#define MAP_DIO2_LORA_NOP      		0x0C  // ----11-- bit 3 and 2
	
#define MAP_DIO3_LORA_CADDONE  		0x00  // ------00 bit 1 and 0
#define MAP_DIO3_LORA_HEADER		0x01  // ------01
#define MAP_DIO3_LORA_CRC			0x02  // ------10
#define MAP_DIO3_LORA_NOP      		0x03  // ------11
	
	// FSK specific
#define MAP_DIO0_FSK_READY     		0x00  // 00------ (packet sent / payload ready)
	
#define MAP_DIO1_FSK_NOP       		0x30  // --11----
#define MAP_DIO2_FSK_TXNOP     		0x04  // ----01--
#define MAP_DIO2_FSK_TIMEOUT   		0x08  // ----10--
	
	// ----------------------------------------
	// Bits masking the corresponding IRQs from the radio
#define IRQ_LORA_RXTOUT_MASK 		0x80	// RXTOUT
#define IRQ_LORA_RXDONE_MASK 		0x40	// RXDONE after receiving the header and CRC, we receive payload part
#define IRQ_LORA_CRCERR_MASK 		0x20
#define IRQ_LORA_HEADER_MASK 		0x10	// valid HEADER mask. This interrupt is first when receiving a message
#define IRQ_LORA_TXDONE_MASK 		0x08
#define IRQ_LORA_CDDONE_MASK 		0x04	// CDDONE
#define IRQ_LORA_FHSSCH_MASK 		0x02
#define IRQ_LORA_CDDETD_MASK 		0x01
	
	
	// ----------------------------------------
	// Definitions for UDP message arriving from server
#define PROTOCOL_VERSION			0x02  //V1.3
#define PKT_PUSH_DATA				0x00
#define PKT_PUSH_ACK				0x01
#define PKT_PULL_DATA				0x02
#define PKT_PULL_RESP				0x03
#define PKT_PULL_ACK				0x04
#define PKT_TX_ACK                  0x05
	
#define MGT_RESET					0x15		// Not a LoRa Gateway Spec message
#define MGT_SET_SF					0x16
#define MGT_SET_FREQ				0x17
	
	
#define RF_PADAC_20DBM_MASK                         0xF8
#define RF_PADAC_20DBM_ON                           0x07
#define RF_PADAC_20DBM_OFF                          0x04  // Default
#define RF_PACONFIG_PASELECT_MASK                   0x7F
#define RF_PACONFIG_PASELECT_PABOOST                0x80
#define RF_PACONFIG_PASELECT_RFO                    0x00 // Default
#define RF_PACONFIG_MAX_POWER_MASK                  0x8F
#define RF_PACONFIG_OUTPUTPOWER_MASK                0xF0
	
#define REG_PACONFIG                                0x09
#define REG_FRFMSB                                  0x06
#define REG_FRFMID                                  0x07
#define REG_FRFLSB                                  0x08
#define FREQ_STEP                                   61.03515625
#define REG_IMAGECAL                                0x3B
#define RF_IMAGECAL_IMAGECAL_MASK                   0xBF
#define RF_IMAGECAL_IMAGECAL_START                  0x40
#define RF_IMAGECAL_IMAGECAL_RUNNING                0x20
	
	

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __SX1278_REG_H__ */
