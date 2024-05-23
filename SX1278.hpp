/**
* @author Jakub Bubak
* @date 07.11.2023
*/

#ifndef KALMAN_ELECTRONICS_SX1278_DRIVER_SX1278_HPP
#define KALMAN_ELECTRONICS_SX1278_DRIVER_SX1278_HPP



#include <etl/optional.h>

#include "main.h"
#include "SX1278_ControlTable.hpp"
#include "Utils/hw.hpp"

namespace radio::sx1278 {
	struct PinoutConfig {
		/** Pointer to HAL SPI handle **/
		SPI_HandleTypeDef* spi_handle;
		/** NSS pin **/
		utils::GPIO_Pin NSS;
		/** RESET pin **/
		utils::GPIO_Pin RESET;
		/** DIO pins **/
		utils::GPIO_Pin DIO0;
		// utils::GPIO_Pin DIO1;
		// utils::GPIO_Pin DIO2;
		// utils::GPIO_Pin DIO3;
		// utils::GPIO_Pin DIO4;
		// utils::GPIO_Pin DIO5;
	};

	class SX1278 {
	public:
		explicit SX1278(PinoutConfig pinout_config) : pinout_config(pinout_config) {};
		~SX1278() = default;

		/** Public methods **/
		Status init(
				uint32_t frequency = 433,
				lora::Power power = lora::Power::POWER_17_DB,
				lora::SpreadingFactor spreading_factor = lora::SpreadingFactor::SF_7,
				lora::Bandwidth bandwidth = lora::Bandwidth::BW_125_KHZ,
				lora::CodingRate coding_rate = lora::CodingRate::CR_4_5,
				lora::HeaderMode header_mode = lora::HeaderMode::EXPLICIT,
				lora::LNAGain lna_gain = lora::LNAGain::G1,
				lora::PayloadCRC crc = lora::PayloadCRC::ON,
				uint16_t preamble_length = 8,
				uint16_t timeout = 0,
				uint8_t max_current = 100
				);

		void reset() const;

		void startTransmit(uint8_t* data, uint8_t length);
		void startReceive();
		uint8_t getReceivedData(uint8_t* data, uint8_t length = 0);

		void set_frequency(uint32_t frequency);
		void set_power(lora::Power power);
		void set_spreading_factor(lora::SpreadingFactor spreading_factor);
		void set_bandwidth(lora::Bandwidth bandwidth);
		void set_coding_rate(lora::CodingRate coding_rate);
		void set_preamble_length(uint16_t preamble_length);
		void set_timeout(uint16_t timeout);
		void set_payload_crc(lora::PayloadCRC crc);
		void set_mode(lora::Mode mode);
		void set_ocp(uint8_t max_current);
		void set_header_mode(lora::HeaderMode header_mode);
		void set_lna_gain(lora::LNAGain lna_gain);

		int get_RSSI();
		uint8_t get_version();
		lora::Mode get_mode();
		void on_dio0_irq();

		void(*on_rx)(void) = nullptr;
	private:
		/** Hardware **/
		PinoutConfig pinout_config;

		/** Module settings **/
		lora::Mode _current_mode;
		uint32_t _frequency;
		lora::Power _power;
		lora::SpreadingFactor _spreading_factor;
		lora::Bandwidth _bandwidth;
		lora::CodingRate _coding_rate;
		lora::HeaderMode _header_mode;
		lora::LNAGain _lna_gain;
		lora::PayloadCRC _crc;
		uint16_t _preamble_length;
		uint16_t _timeout;
		uint8_t _max_current;

		void _handle_txdone_irq();
		void _handle_rxdone_irq();

		//TODO: add other settings, figure how to store them separately for FSK and LORA

		/** Internal methods **/
		template <typename RegVal, typename RegAddr>
		void SPI_write(RegAddr addr, RegVal val);

		template <typename RegValPtr, typename RegAddr>
		void SPI_BurstWrite(RegAddr addr, RegValPtr* val, uint8_t length);

		template<typename RegVal, typename RegAddr>
		etl::optional<RegVal> SPI_read(RegAddr reg);

		template <typename RegAddr, typename RegValPtr>
		bool SPI_burstRead(RegAddr addr, RegValPtr* val, uint8_t length);

		void clear_irq_flags(IrqFlags flags = IrqFlags::All);

	};

}

#endif //KALMAN_ELECTRONICS_SX1278_DRIVER_SX1278_HPP
