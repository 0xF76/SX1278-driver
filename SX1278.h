#ifndef INC_SX1278_H_
#define INC_SX1278_H_

#include "main.h"
#include <string.h>

#define SX1278_TX_TIMEOUT			2000
#define SX1278_RX_TIMEOUT			2000

//--------- MODES ---------//
#define SX1278_SLEEP_MODE					0
#define	SX1278_STNBY_MODE					1
#define SX1278_TX_MODE						3
#define SX1278_RXCONTIN_MODE				5
//------- BANDWIDTH -------//
#define SX1278_BW_7_8KHz					0
#define SX1278_BW_10_4KHz					1
#define SX1278_BW_15_6KHz					2
#define SX1278_BW_20_8KHz					3
#define SX1278_BW_31_25KHz					4
#define SX1278_BW_41_7KHz					5
#define SX1278_BW_62_5KHz					6
#define SX1278_BW_125KHz					7
#define SX1278_BW_250KHz					8
#define SX1278_BW_500KHz					9

//------ CODING RATE ------//
#define SX1278_CR_4_5						1
#define SX1278_CR_4_6						2
#define SX1278_CR_4_7						3
#define SX1278_CR_4_8						4

//--- SPREADING FACTORS ---//
#define SX1278_SF_7							7
#define SX1278_SF_8							8
#define SX1278_SF_9							9
#define SX1278_SF_10						10
#define SX1278_SF_11  						11
#define SX1278_SF_12						12

//------ POWER GAIN ------//
#define SX1278_POWER_11db					0xF6
#define SX1278_POWER_14db					0xF9
#define SX1278_POWER_17db					0xFC
#define SX1278_POWER_20db					0xFF

//------- REGISTERS -------//
#define SX1278_RegFiFo						0x00
#define SX1278_RegOpMode					0x01
#define SX1278_RegFrMsb						0x06
#define SX1278_RegFrMid						0x07
#define SX1278_RegFrLsb						0x08
#define SX1278_RegPaConfig					0x09
#define SX1278_RegOcp						0x0B
#define SX1278_RegLna						0x0C
#define SX1278_RegFiFoAddPtr				0x0D
#define SX1278_RegFiFoTxBaseAddr			0x0E
#define SX1278_RegFiFoRxBaseAddr			0x0F
#define SX1278_RegFiFoRxCurrentAddr			0x10
#define SX1278_RegIrqFlags					0x12
#define SX1278_RegRxNbBytes					0x13
#define SX1278_RegPktRssiValue				0x1A
#define	SX1278_RegModemConfig1				0x1D
#define SX1278_RegModemConfig2				0x1E
#define SX1278_RegSymbTimeoutL				0x1F
#define SX1278_RegPreambleMsb				0x20
#define SX1278_RegPreambleLsb				0x21
#define SX1278_RegPayloadLength				0x22
#define SX1278_RegDioMapping1				0x40
#define SX1278_RegDioMapping2				0x41
#define SX1278_RegVersion					0x42

//------ SX1278 STATUS ------//
#define SX1278_OK							200
#define SX1278_NOT_FOUND					404

typedef struct {
	
	// Hardware:
	GPIO_TypeDef*      		NSS_port;
	uint16_t			    NSS_pin;
	GPIO_TypeDef*      		RESET_port;
	uint16_t			    RESET_pin;
	GPIO_TypeDef*      		DIO0_port;
	uint16_t			    DIO0_pin;
	SPI_HandleTypeDef* 		hSPIx;
	
	// Module:
	int						current_mode;
	int 					frequency;
	uint8_t					spredingFactor;
	uint8_t					bandWidth;
	uint8_t					crcRate;
	uint16_t				preamble;
	uint8_t					power;
	uint8_t					overCurrentProtection;
	
} SX1278;

SX1278 SX1278_new(void);
void SX1278_reset(SX1278*);
void SX1278_changeMode(SX1278*, int);

uint8_t SX1278_read(SX1278*, uint8_t);

void SX1278_write(SX1278*, uint8_t, uint8_t);
void SX1278_BurstWrite(SX1278*, uint8_t, uint8_t*, uint8_t);

void SX1278_setFrequency(SX1278*, int);
void SX1278_setSpreadingFactor(SX1278*, int);
void SX1278_setPower(SX1278*, uint8_t);
void SX1278_setOCP(SX1278*, uint8_t);
void SX1278_setTOMsb_setCRCon(SX1278*);

void SX1278_transmit(SX1278*, uint8_t*, uint8_t);

uint8_t SX1278_receive(SX1278*, uint8_t*, uint8_t);

int SX1278_getRSSI(SX1278*);


uint16_t SX1278_init(SX1278*);

#endif /* INC_SX1278_H_ */
