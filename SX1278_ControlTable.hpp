/**
* @author Jakub Bubak
* @date 18.11.2023
*/

#ifndef KALMAN_ELECTRONICS_SX1278_DRIVER_SX1278_CONTROLTABLE_H
#define KALMAN_ELECTRONICS_SX1278_DRIVER_SX1278_CONTROLTABLE_H

namespace radio::sx1278 {

	enum class Status {
		OK = 0,
		ERROR = 1,
	};


	/** Registers common for FSK and LoRa **/
	enum class RegisterAddress : uint8_t  {
		RegFifo = 0x00,
		RegOpMode = 0x01,
		RegFrMsb = 0x06,
		RegFrMid = 0x07,
		RegFrLsb = 0x08,
		RegPaConfig = 0x09,
		RegPaRamp = 0x0A,
		RegOcp = 0x0B,
		RegLna = 0x0C,
		RegDioMapping1 = 0x40,
		RegDioMapping2 = 0x41,
		RegVersion = 0x42,
		RegTcxo = 0x4B,
		RegPaDac = 0x4D,
		RegFormerTemp = 0x5B,
		RegAgcRef = 0x61,
		RegAgcThresh1 = 0x62,
		RegAgcThresh2 = 0x63,
		RegAgcThresh3 = 0x64,
		RegPll = 0x70,

	};

	enum IrqFlags {
		All = 0xFF,
		RxTimeout = 1 << 7,
		RxDone = 1 << 6,
		PayloadCrcError = 1 << 5,
		ValidHeader = 1 << 4,
		TxDone = 1 << 3,
		CadDone = 1 << 2,
		FhssChangeChannel = 1 << 1,
		CadDetected = 1 << 0,
	};

	namespace lora {
		/** LoRa specific registers **/
		enum class RegisterAddress : uint8_t {
			RegFifoAddrPtr = 0x0D,
			RegFifoTxBaseAddr = 0x0E,
			RegFifoRxBaseAddr = 0x0F,
			RegFiFoRxCurrentAddr = 0x10,
			RegIrqFlagsMask = 0x11,
			RegIrqFlags = 0x12,
			RegRxNbBytes = 0x13,
			RegRxHeaderCntValueMsb = 0x14,
			RegRxHeaderCntValueLsb = 0x15,
			RegRxPacketCntValueMsb = 0x16,
			RegRxPacketCntValueLsb = 0x17,
			RegModemStat = 0x18,
			RegPktSnrValue = 0x19,
			RegPktRssiValue = 0x1A,
			RegRssiValue = 0x1B,
			RegHopChannel = 0x1C,
			RegModemConfig1 = 0x1D,
			RegModemConfig2 = 0x1E,
			RegSymbTimeoutLsb = 0x1F,
			RegPreambleMsb = 0x20,
			RegPreambleLsb = 0x21,
			RegPayloadLength = 0x22,
			RegMaxPayloadLength = 0x23,
			RegHopPeriod = 0x24,
			RegFifoRxByteAddr = 0x25,
			RegModemConfig3 = 0x26,
			RegFeiMsb = 0x28,
			RegFeiMid = 0x29,
			RegFeiLsb = 0x2A,
			RegRssiWideband = 0x2C,
			RegDetectOptimize = 0x31,
			RegInvertIQ = 0x33,
			RegDetectionThreshold = 0x37,
			RegSyncWord = 0x39,
		};

		enum class Mode : uint8_t {
			SLEEP = 0b000,
			STDBY = 0b001,
			FSTX = 0b010,
			TX = 0b011,
			FSRX = 0b100,
			RXCONTINUOUS = 0b101,
			RXSINGLE = 0b110,
			CAD = 0b111,
		};

		enum class Power : uint8_t {
			POWER_11_DB = 0xF6,
			POWER_14_DB = 0xF9,
			POWER_17_DB = 0xFC,
			POWER_20_DB = 0xFF,
		};

		enum class Bandwidth : uint8_t {
			BW_7_8_KHZ = 0b0000,
			BW_10_4_KHZ = 0b0001,
			BW_15_6_KHZ = 0b0010,
			BW_20_8_KHZ = 0b0011,
			BW_31_25_KHZ = 0b0100,
			BW_41_7_KHZ = 0b0101,
			BW_62_5_KHZ = 0b0110,
			BW_125_KHZ = 0b0111,
			BW_250_KHZ = 0b1000,
			BW_500_KHZ = 0b1001,
		};

		enum class CodingRate : uint8_t {
			CR_4_5 = 0b001,
			CR_4_6 = 0b010,
			CR_4_7 = 0b011,
			CR_4_8 = 0b100,
		};

		enum class SpreadingFactor : uint8_t {
			SF_6 = 6,
			SF_7 = 7,
			SF_8 = 8,
			SF_9 = 9,
			SF_10 = 10,
			SF_11 = 11,
			SF_12 = 12,
		};

		enum class HeaderMode : uint8_t {
			IMPLICIT = 0,
			EXPLICIT = 1,
		};

		enum class PayloadCRC : uint8_t {
			OFF = 0,
			ON = 1,
		};

		enum class LNAGain : uint8_t {
			G1 = 0b001,
			G2 = 0b010,
			G3 = 0b011,
			G4 = 0b100,
			G5 = 0b101,
			G6 = 0b110,
		};

	}

	namespace fsk {
		/** FSK specific registers **/
		enum class RegisterAddress : uint8_t  {
			//TODO: Add FSK registers
		};
	}

	namespace lf {
		/** Low Frequency Additional Registers **/
		enum class RegisterAddress : uint8_t {
			//TODO: Add LF registers
		};
	}

	namespace hf {
		/** High Frequency Additional Registers **/
		enum class RegisterAddress : uint8_t {
			//TODO: Add HF registers
		};
	}

}

#endif //KALMAN_ELECTRONICS_SX1278_DRIVER_SX1278_CONTROLTABLE_H
