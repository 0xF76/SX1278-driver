/**
* @author Jakub Bubak
* @date 07.11.2023
*/

#include "SX1278.hpp"


/**
 * @brief Writes a value to a register in the SX1278 LoRa transceiver via SPI.
 *
 * @tparam RegVal The data type of the register value.
 * @tparam RegAddr The data type of the register address.
 * @param addr The address of the register to write to.
 * @param val The value to write to the register.
 *
 * @note This function assumes that the register address and value are both 1 byte long.
 * @note The MSB of the address is set to 1 to indicate a write operation.
 */
template <typename RegVal, typename RegAddr>
void radio::sx1278::SX1278::SPI_write(RegAddr addr, RegVal val) {
	static_assert(sizeof(RegAddr) == 1, "Register address must be 1 byte long");
	static_assert(sizeof(RegVal) == 1, "Register value must be 1 byte long");

	uint8_t address = static_cast<uint8_t>(addr) | 0x80; /** set MSB to 1 to indicate write **/
	auto value = static_cast<uint8_t>(val);

	HAL_GPIO_WritePin(pinout_config.NSS.GPIOPort, pinout_config.NSS.GPIOPin, GPIO_PIN_RESET);

	HAL_SPI_Transmit(pinout_config.spi_handle, &address, sizeof(address), HAL_MAX_DELAY); /** send address **/
	while(HAL_SPI_GetState(pinout_config.spi_handle) != HAL_SPI_STATE_READY); /** wait for SPI to finish **/
	HAL_SPI_Transmit(pinout_config.spi_handle, &value, sizeof(value), HAL_MAX_DELAY); /** send value **/
	while(HAL_SPI_GetState(pinout_config.spi_handle) != HAL_SPI_STATE_READY); /** wait for SPI to finish **/

	HAL_GPIO_WritePin(pinout_config.NSS.GPIOPort, pinout_config.NSS.GPIOPin, GPIO_PIN_SET);

	//TODO: add error handling
}

/**
 * @brief Writes a burst of values to consecutive registers in the SX1278 LoRa transceiver via SPI.
 *
 * @tparam RegValPtr The data type of the pointer to register values.
 * @tparam RegAddr The data type of the register address.
 * @param addr The starting address of the register to write to.
 * @param val A pointer to the array of values to write to consecutive registers.
 * @param length The number of values to write in the burst.
 *
 * @note This function assumes that the register address is 1 byte long, and the pointer to register values points to 1-byte values.
 * @note The MSB of the address is set to 1 to indicate a write operation.
 *
 */
template<typename RegValPtr, typename RegAddr>
void radio::sx1278::SX1278::SPI_BurstWrite(RegAddr addr, RegValPtr* val, uint8_t length) {
	static_assert(sizeof(RegAddr) == 1, "Register address must be 1 byte long");
	static_assert(sizeof(RegValPtr) == 1, "Pointer to Register values must be 1 byte long");

	uint8_t address = static_cast<uint8_t>(addr) | 0x80; /** set MSB to 1 to indicate write **/

	HAL_GPIO_WritePin(pinout_config.NSS.GPIOPort, pinout_config.NSS.GPIOPin, GPIO_PIN_RESET);

	HAL_SPI_Transmit(pinout_config.spi_handle, &address, sizeof(address), HAL_MAX_DELAY); /** send address **/
	while(HAL_SPI_GetState(pinout_config.spi_handle) != HAL_SPI_STATE_READY); /** wait for SPI to finish **/
	HAL_SPI_Transmit(pinout_config.spi_handle, val, length, HAL_MAX_DELAY); /** send value **/
	while(HAL_SPI_GetState(pinout_config.spi_handle) != HAL_SPI_STATE_READY); /** wait for SPI to finish **/

	HAL_GPIO_WritePin(pinout_config.NSS.GPIOPort, pinout_config.NSS.GPIOPin, GPIO_PIN_SET);

	//TODO: add error handling
}

/**
 * @brief Reads a value from a register in the SX1278 LoRa transceiver via SPI.
 *
 * @tparam RegVal The data type of the register value.
 * @tparam RegAddr The data type of the register address.
 * @param reg The address of the register to read from.
 *
 * @note This function assumes that the register address and value are both 1 byte long.
 * @note The MSB of the address is set to 0 to indicate a read operation.
 *
 * @return An optional containing the read value if the read operation was successful; otherwise, an empty optional.
 */

template <typename RegVal, typename RegAddr>
etl::optional<RegVal> radio::sx1278::SX1278::SPI_read(RegAddr reg) {
	static_assert(sizeof(RegAddr) == 1, "Register address must be 1 byte long");
	static_assert(sizeof(RegVal) == 1, "Register value must be 1 byte long");

	uint8_t received_value;
	uint8_t address = static_cast<uint8_t>(reg) & 0x7F; /** set MSB to 0 to indicate read **/

	HAL_GPIO_WritePin(pinout_config.NSS.GPIOPort, pinout_config.NSS.GPIOPin, GPIO_PIN_RESET);

	HAL_SPI_Transmit(pinout_config.spi_handle, &address, sizeof(address), HAL_MAX_DELAY); /** send address **/
	while(HAL_SPI_GetState(pinout_config.spi_handle) != HAL_SPI_STATE_READY); /** wait for SPI to finish **/
	auto status = HAL_SPI_Receive(pinout_config.spi_handle, &received_value, sizeof(received_value), HAL_MAX_DELAY);
	while(HAL_SPI_GetState(pinout_config.spi_handle) != HAL_SPI_STATE_READY); /** wait for SPI to finish **/

	HAL_GPIO_WritePin(pinout_config.NSS.GPIOPort, pinout_config.NSS.GPIOPin, GPIO_PIN_SET);

	if(status == HAL_OK) {
		return static_cast<RegVal>(received_value);
	}
	return etl::nullopt;
}

template <typename RegAddr, typename RegValPtr>
bool radio::sx1278::SX1278::SPI_burstRead(RegAddr addr, RegValPtr* val, uint8_t length) {
	static_assert(sizeof(RegAddr) == 1, "Register address must be 1 byte long");
	static_assert(sizeof(RegValPtr) == 1, "Pointer to Register values must be 1 byte long");

	uint8_t address = static_cast<uint8_t>(addr) & 0x7F; /** set MSB to 0 to indicate read **/

	HAL_GPIO_WritePin(pinout_config.NSS.GPIOPort, pinout_config.NSS.GPIOPin, GPIO_PIN_RESET);

	HAL_SPI_Transmit(pinout_config.spi_handle, &address, sizeof(address), HAL_MAX_DELAY); /** send address **/
	while(HAL_SPI_GetState(pinout_config.spi_handle) != HAL_SPI_STATE_READY); /** wait for SPI to finish **/
	auto status = HAL_SPI_Receive(pinout_config.spi_handle, val, length, HAL_MAX_DELAY);
	while(HAL_SPI_GetState(pinout_config.spi_handle) != HAL_SPI_STATE_READY); /** wait for SPI to finish **/

	HAL_GPIO_WritePin(pinout_config.NSS.GPIOPort, pinout_config.NSS.GPIOPin, GPIO_PIN_SET);

	return status == HAL_OK;
}

/**
 * @brief Resets the SX1278 LoRa transceiver.
 *
 * This function performs a reset operation on the SX1278 LoRa transceiver by toggling the reset pin.
 *
 * @note The function first pulls the reset pin low, waits for a short duration, and then releases it.
 */

void radio::sx1278::SX1278::reset() const {
	HAL_GPIO_WritePin(pinout_config.RESET.GPIOPort, pinout_config.RESET.GPIOPin, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(pinout_config.RESET.GPIOPort, pinout_config.RESET.GPIOPin, GPIO_PIN_SET);
	HAL_Delay(10);
}


/**
 * @brief Transmits data using the SX1278 LoRa transceiver.
 *
 * This function prepares and requests data transmission using the SX1278 LoRa transceiver.
 *
 * @param data A pointer to the data to be transmitted.
 * @param length The length of the data to be transmitted.
 *
 * @note The function sets the transceiver to STDBY mode, configures the FIFO address and payload length registers,
 *       writes the data to be transmitted to the FIFO, and then sets the transceiver to TX mode for transmission.
 */
//TODO: change name
void radio::sx1278::SX1278::startTransmit(uint8_t *data, uint8_t length) {
	set_mode(lora::Mode::STDBY);

	SPI_write(lora::RegisterAddress::RegFifoAddrPtr, static_cast<uint8_t>(0x00)); // Always use entire FIFO for TX
	SPI_write(lora::RegisterAddress::RegPayloadLength, length);
	SPI_BurstWrite(RegisterAddress::RegFifo, data, length);

	set_mode(lora::Mode::TX);
}

/**
 * @brief Receives data using the SX1278 LoRa transceiver.
 *
 * This function receives data using the SX1278 LoRa transceiver and stores it in the provided buffer.
 *
 * @param data A pointer to the buffer where received data will be stored.
 * @param length The maximum length of data to be received and stored in the buffer.
 *
 * @return The actual number of bytes received and stored in the buffer.
 *
 * @note The function sets the transceiver to STDBY mode and checks for the RxDone interrupt flag.
 * @note If the RxDone flag is set, the function clears the interrupt flags, reads the number of received bytes,
 *       and reads the received data into the provided buffer.
 * @note The function sets the transceiver back to RXCONTINUOUS mode and returns the number of bytes received.
 */

// TODO: check IRQ mask
// TODO: PA ramp up time set

void radio::sx1278::SX1278::startReceive() {
	set_mode(lora::Mode::RXCONTINUOUS);
}

// Should only be called after RxDone
uint8_t radio::sx1278::SX1278::getReceivedData(uint8_t* data, uint8_t length) {
	// TODO: packet crc check
	// TODO: header crc check
	auto irq_flags = static_cast<IrqFlags>(SPI_read<uint8_t>(lora::RegisterAddress::RegIrqFlags).value());

	if (!(irq_flags & IrqFlags::RxDone))
		return 0; // TODO: error handling

	// TODO: notify about CRC error

	if (this->_header_mode == lora::HeaderMode::IMPLICIT && length == 0)
		return 0; // TODO: error handling, unknown length
		
	if (this->_header_mode == lora::HeaderMode::EXPLICIT) {
		length = SPI_read<uint8_t>(lora::RegisterAddress::RegRxNbBytes).value();
	}

	auto read = SPI_read<uint8_t>(lora::RegisterAddress::RegFiFoRxCurrentAddr).value();
	SPI_write(lora::RegisterAddress::RegFifoAddrPtr, read);

	SPI_burstRead(RegisterAddress::RegFifo, data, length);

	// for(int i = 0; i < length; i++) {
	// 	data[i] = SPI_read<uint8_t>(RegisterAddress::RegFifo).value();
	// }
	
	clear_irq_flags();

	return length;
}

/**
 * @brief Sets the frequency of the SX1278 LoRa transceiver.
 *
 * This function sets the operating frequency of the SX1278 LoRa transceiver by configuring its registers
 * based on the given frequency value.
 *
 * @param frequency The desired frequency in MegaHertz (MHz) to be set.
 *
 * @note This function uses a formula from the datasheet to calculate the register values needed to set the frequency.
 * @note The calculated values are written to the appropriate registers (RegFrMsb, RegFrMid, and RegFrLsb).
 */

void radio::sx1278::SX1278::set_frequency(uint32_t frequency) {
	uint32_t F = (frequency * 524288) >> 5;

	SPI_write(RegisterAddress::RegFrMsb, static_cast<uint8_t>((F >> 16) & 0xFF));
	SPI_write(RegisterAddress::RegFrMid, static_cast<uint8_t>((F >> 8) & 0xFF));
	SPI_write(RegisterAddress::RegFrLsb, static_cast<uint8_t>(F & 0xFF));

	this->_frequency = frequency;
}

/**
 * @brief Sets the spreading factor for LoRa modulation in the SX1278 LoRa transceiver.
 *
 * This function sets the spreading factor for LoRa modulation in the SX1278 LoRa transceiver
 * by configuring the appropriate registers.
 *
 * @param spreading_factor The desired spreading factor to be set.
 *
 * @note The current value of the ModemConfig2 register is read, and the spreading factor bits are updated
 *       based on the provided spreading factor parameter.
 * @note The updated value is then written back to the ModemConfig2 register.
 */

void radio::sx1278::SX1278::set_spreading_factor(radio::sx1278::lora::SpreadingFactor spreading_factor) {
	auto config_reg = SPI_read<uint8_t>(lora::RegisterAddress::RegModemConfig2);
	if(!config_reg.has_value()) {
		// TODO: Error handling
	}

	config_reg.value() &= 0x0F; /** clear SF bits **/
	config_reg.value() |= static_cast<uint8_t>(spreading_factor) << 4; /** set SF bits **/
	SPI_write(lora::RegisterAddress::RegModemConfig2, config_reg.value());

	auto detect_reg = SPI_read<uint8_t>(lora::RegisterAddress::RegDetectOptimize);
	if(!detect_reg.has_value()) {
		// TODO: Error handling
	}

	// SF6 required optimization
	if (spreading_factor == lora::SpreadingFactor::SF_6) {
		set_header_mode(lora::HeaderMode::IMPLICIT);
		SPI_write(lora::RegisterAddress::RegDetectionThreshold, static_cast<uint8_t>(0x0C));

		// Set DetectionOptimize field to 0x05
		detect_reg.value() &= ~0b111;
		detect_reg.value() |= 0x05;
		SPI_write(lora::RegisterAddress::RegDetectOptimize, detect_reg.value());
	} else {
		SPI_write(lora::RegisterAddress::RegDetectionThreshold, static_cast<uint8_t>(0x0A));

		// Set DetectionOptimize field to 0x03
		detect_reg.value() &= ~0b111;
		detect_reg.value() |= 0x03;
		SPI_write(lora::RegisterAddress::RegDetectOptimize, detect_reg.value());
	}

	this->_spreading_factor = spreading_factor;
}

/**
 * @brief Sets the bandwidth for LoRa modulation in the SX1278 LoRa transceiver.
 *
 * This function sets the bandwidth for LoRa modulation in the SX1278 LoRa transceiver
 * by configuring the appropriate registers.
 *
 * @param bandwidth The desired bandwidth to be set.
 *
 * @note The current value of the ModemConfig1 register is read, and the bandwidth bits are updated
 *       based on the provided bandwidth parameter.
 * @note The updated value is then written back to the ModemConfig1 register.
 */

void radio::sx1278::SX1278::set_bandwidth(radio::sx1278::lora::Bandwidth bandwidth) {
	auto reg_value = SPI_read<uint8_t>(lora::RegisterAddress::RegModemConfig1);

	if(reg_value.has_value()) {
		reg_value.value() &= 0x0F; /** clear BW bits **/
		reg_value.value() |= static_cast<uint8_t>(bandwidth) << 4; /** set BW bits **/
		SPI_write(lora::RegisterAddress::RegModemConfig1, reg_value.value());
	}

	this->_bandwidth = bandwidth;
}

/**
 * @brief Sets the operating mode of the SX1278 LoRa transceiver.
 *
 * This function sets the operating mode of the SX1278 LoRa transceiver by configuring the appropriate registers.
 *
 * @param mode The desired operating mode to be set.
 *
 * @note The current value of the OpMode register is read, and the mode bits are updated
 *       based on the provided mode parameter.
 * @note Depending on the mode, the DIO0 mapping is also adjusted to handle TxDone or RxDone interrupts.
 * @note The updated value is then written back to the OpMode register, and the current mode is updated accordingly.
 */

void radio::sx1278::SX1278::set_mode(radio::sx1278::lora::Mode mode) {
	auto reg_value = SPI_read<uint8_t>(RegisterAddress::RegOpMode);

	if(reg_value.has_value()) {
		// TODO: error handling
	}

	if(mode == lora::Mode::TX) {
		SPI_write(RegisterAddress::RegDioMapping1, static_cast<uint8_t>(0x40)); /** set DIO0 to TxDone **/
	} else if(mode == lora::Mode::RXCONTINUOUS) {
		SPI_write(RegisterAddress::RegDioMapping1, static_cast<uint8_t>(0x00)); /** set DIO0 to RxDone **/
	}

	reg_value.value() &= 0xF8; /** clear mode bits **/
	reg_value.value() |= static_cast<uint8_t>(mode); /** set mode bits **/
	SPI_write(RegisterAddress::RegOpMode, reg_value.value());

	this->_current_mode = mode;
}

/**
 * @brief Sets the payload CRC configuration for LoRa modulation.
 *
 * This function configures the payload CRC for LoRa modulation in the SX1278 LoRa transceiver
 * by updating the ModemConfig2 register.
 *
 * @param crc The desired CRC configuration (ON or OFF).
 *
 * @note The current value of the ModemConfig2 register is read, and the CRC bit is updated based on the provided CRC parameter.
 * @note The updated value is then written back to the ModemConfig2 register.
 */

void radio::sx1278::SX1278::set_payload_crc(lora::PayloadCRC crc) {
	auto reg_value = SPI_read<uint8_t>(lora::RegisterAddress::RegModemConfig2);

	if(reg_value.has_value()) {
		if(crc == lora::PayloadCRC::ON) {
			reg_value.value() |= 0x04;
		} else {
			reg_value.value() &= 0xFB;
		}
		SPI_write(lora::RegisterAddress::RegModemConfig2, reg_value.value());
	}

	this->_crc = crc;
}

/**
 * @brief Sets the Over-Current Protection (OCP) for the SX1278 LoRa transceiver.
 *
 * This function configures the Over-Current Protection (OCP) for the SX1278 LoRa transceiver
 * by calculating the appropriate value based on the maximum current provided and writing it to the OCP register.
 *
 * @param max_current The maximum current limit to set for OCP in milliamperes (mA), within the range of 45 mA to 240 mA.
 *
 * @note If the provided max_current value is outside the valid range, it is adjusted to the nearest valid value.
 * @note The OCP trim value is calculated based on the datasheet formula and written to the OCP register.
 */

void radio::sx1278::SX1278::set_ocp(uint8_t max_current) {
	uint8_t ocp_trim;

	/** making sure that max current is in range **/
	if(max_current > 240) {
		max_current = 240;
	} else if(max_current < 45) {
		max_current = 45;
	}

	/** calculate ocp trim based on formula from datasheet**/
	if(max_current <= 120) {
		ocp_trim = (max_current - 45) / 5;
	} else {
		ocp_trim = (max_current + 30) / 10;
	}

	SPI_write(RegisterAddress::RegOcp, ocp_trim);

	this->_max_current = max_current;
}

/**
 * @brief Sets the transmit power level for the SX1278 LoRa transceiver.
 *
 * This function sets the transmit power level for the SX1278 LoRa transceiver by configuring the PaConfig register.
 *
 * @param power The desired transmit power level to be set.
 */

void radio::sx1278::SX1278::set_power(lora::Power power) {
	SPI_write(RegisterAddress::RegPaConfig, static_cast<uint8_t>(power));

	this->_power = power;
}

/**
 * @brief Sets the preamble length for LoRa communication in the SX1278 LoRa transceiver.
 *
 * This function sets the preamble length for LoRa communication in the SX1278 LoRa transceiver
 * by configuring the Pthis->_coding_rate = coding_rate;reambleMsb and PreambleLsb registers.
 *
 * @param preamble_length The desired preamble length in number of symbols.
 *
 * @note The preamble length is specified as a 16-bit value and is divided into MSB and LSB parts.
 * @note The function sets the preamble length registers based on the provided preamble_length parameter.
 */

void radio::sx1278::SX1278::set_preamble_length(uint16_t preamble_length) {
	assert(preamble_length >= 6); // TODO: better error handling

	SPI_write(lora::RegisterAddress::RegPreambleMsb, static_cast<uint8_t>((preamble_length >> 8) & 0xFF));
	SPI_write(lora::RegisterAddress::RegPreambleLsb, static_cast<uint8_t>(preamble_length & 0xFF));

	this->_preamble_length = preamble_length;
}

/**
 * @brief Sets the coding rate for LoRa communication in the SX1278 LoRa transceiver.
 *
 * This function sets the coding rate for LoRa communication in the SX1278 LoRa transceiver
 * by configuring the ModemConfig1 register.
 *
 * @param coding_rate The desired coding rate to be set.
 *
 * @note The current value of the ModemConfig1 register is read, and the coding rate bits are updated
 *       based on the provided coding_rate parameter.
 * @note The updated value is then written back to the ModemConfig1 register.
 */

void radio::sx1278::SX1278::set_coding_rate(radio::sx1278::lora::CodingRate coding_rate) {
	auto reg_value = SPI_read<uint8_t>(lora::RegisterAddress::RegModemConfig1);

	if(reg_value.has_value()) {
		reg_value.value() &= 0xF1; /** clear CR bits **/
		reg_value.value() |= static_cast<uint8_t>(coding_rate) << 1; /** set CR bits **/
		SPI_write(lora::RegisterAddress::RegModemConfig1, reg_value.value());
	}

	this->_coding_rate = coding_rate;
}

/**
 * @brief Sets the timeout value for LoRa communication in the SX1278 LoRa transceiver.
 *
 * This function sets the timeout value for LoRa communication in the SX1278 LoRa transceiver
 * by configuring the SymbTimeoutLsb and ModemConfig2 registers.
 *
 * @param timeout The desired timeout value in symbols.
 *
 * @note The lower byte of the timeout value is written to the SymbTimeoutLsb register.
 * @note The upper 2 bits of the timeout value are written to the ModemConfig2 register.
 * @note The timeout value is specified in symbols.
 */

void radio::sx1278::SX1278::set_timeout(uint16_t timeout) {
	SPI_write(lora::RegisterAddress::RegSymbTimeoutLsb, static_cast<uint8_t>(timeout & 0xFF));

	auto read = SPI_read<uint8_t>(lora::RegisterAddress::RegModemConfig2);
	if(read.has_value()) {
		read.value() &= 0xFC;
		read.value() |= static_cast<uint8_t>((timeout >> 8) & 0x03);
		SPI_write(lora::RegisterAddress::RegModemConfig2, read.value());
	}

	this->_timeout = timeout;
}


/**
 * @brief Sets the header mode for LoRa communication in the SX1278 LoRa transceiver.
 *
 * This function sets the header mode for LoRa communication in the SX1278 LoRa transceiver
 * by configuring the ModemConfig1 register.
 *startReceive
 * @param header_mode The desired header mode to be set (EXPLICIT or IMPLICIT).
 *
 * @note The current value of the ModemConfig1 register is read, and the header mode bit is updated
 *       based on the provided header_mode parameter.
 * @note The updated value is then written back to the ModemConfig1 register.
 */

void radio::sx1278::SX1278::set_header_mode(radio::sx1278::lora::HeaderMode header_mode) {
	auto reg_value = SPI_read<uint8_t>(lora::RegisterAddress::RegModemConfig1);

	if(!(reg_value.has_value())) {
		// TODO: error handling
	}

	// SF6 requires implicit header mode
	if(this->_spreading_factor == lora::SpreadingFactor::SF_6)
		header_mode = lora::HeaderMode::IMPLICIT;

	if(header_mode == lora::HeaderMode::EXPLICIT) {
		reg_value.value() &= 0xFE;
	} else {
		reg_value.value() |= 0x01;
	}
	SPI_write(lora::RegisterAddress::RegModemConfig1, reg_value.value());

	this->_header_mode = header_mode;
}

/**
 * @brief Sets the LNA (Low-Noise Amplifier) gain for the SX1278 LoRa transceiver.
 *
 * This function sets the LNA (Low-Noise Amplifier) gain for the SX1278 LoRa transceiver
 * by configuring the Lna register.
 *
 * @param lna_gain The desired LNA gain to be set.
 *
 * @note The current value of the Lna register is read, and the LNA gain bits are updated
 *       based on the provided lna_gain parameter.
 * @note The updated value is then written back to the Lna register.
 */

// TODO: crosscheck how and if this function is necessary in user facing format
void radio::sx1278::SX1278::set_lna_gain(radio::sx1278::lora::LNAGain lna_gain) {
	auto reg_value = SPI_read<uint8_t>(RegisterAddress::RegLna);

	if(reg_value.has_value()) {
		reg_value.value() &= 0x1F;
		reg_value.value() |= static_cast<uint8_t>(lna_gain) << 5;
		SPI_write(RegisterAddress::RegLna, reg_value.value());
	}

	this->_lna_gain = lna_gain;
}

/**
 * @brief Gets the current operating mode of the SX1278 LoRa transceiver.
 *
 * This function retrieves and returns the current operating mode of the SX1278 LoRa transceiver.
 *
 * @return The current operating mode as a value from the lora::Mode enum.
 */
radio::sx1278::lora::Mode radio::sx1278::SX1278::get_mode() {
	return _current_mode;
}

/**
 * @brief Gets the version information from the SX1278 LoRa transceiver.
 *
 * This function retrieves the version information from the SX1278 LoRa transceiver by reading the Version register.
 *
 * @return The version information as an unsigned 8-bit integer, or 0 if the read operation fails.
 */

uint8_t radio::sx1278::SX1278::get_version() {
	auto reg_value = SPI_read<uint8_t>(RegisterAddress::RegVersion);

	if(reg_value.has_value()) {
		return reg_value.value();
	}
	return 0;
}

/**
 * @brief Gets the Received Signal Strength Indicator (RSSI) from the SX1278 LoRa transceiver.
 *
 * This function retrieves the Received Signal Strength Indicator (RSSI) from the SX1278 LoRa transceiver
 * by reading the RSSI Value register.
 *
 * @return The RSSI value as an integer, or 0 if the read operation fails.
 *
 * @note The formula used to calculate the RSSI value is valid only for low frequencies; refer to the datasheet for more information.
 * @note The returned RSSI value is an integer representing the signal strength in dBm.
 */

int radio::sx1278::SX1278::get_RSSI() {
	auto reg_value = SPI_read<uint8_t>(lora::RegisterAddress::RegRssiValue);

#warning "this formula is valid only for low frequencies - for more information see datasheet"
	if(reg_value.has_value()) {
		return -164 + reg_value.value();
	}
	return 0;

}


/**
 * @brief Clears interrupt flags in the SX1278 LoRa transceiver.
 *
 * This function clears interrupt flags in the SX1278 LoRa transceiver by writing 0xFF to the IrqFlags register.
 */

void radio::sx1278::SX1278::clear_irq_flags(IrqFlags flags) {
	SPI_write(lora::RegisterAddress::RegIrqFlags, static_cast<uint8_t>(flags));
}


/**
 * @brief Initializes and configures the SX1278 LoRa transceiver.
 *
 * This function initializes and configures the SX1278 LoRa transceiver with various settings such as frequency,
 * power, spreading factor, bandwidth, coding rate, header mode, LNA gain, CRC, preamble length, timeout, and OCP.
 *
 * @param frequency The desired operating frequency in Hertz (Hz).
 * @param power The desired transmit power level.
 * @param spreading_factor The desired spreading factor for LoRa modulation.
 * @param bandwidth The desired bandwidth for LoRa modulation.
 * @param coding_rate The desired coding rate for LoRa modulation.
 * @param header_mode The desired header mode (EXPLICIT or IMPLICIT).
 * @param lna_gain The desired LNA (Low-Noise Amplifier) gain.
 * @param crc The desired payload CRC configuration (ON or OFF).
 * @param preamble_length The desired preamble length in number of symbols.
 * @param timeout The desired timeout value in symbols.
 * @param max_current The desired Over-Current Protection (OCP) value in milliamperes (mA).
 *
 * @return The initialization status (OK or ERROR).
 *
 * @note The function performs various configuration steps for the SX1278 transceiver,
 *       including setting the operating mode, frequency, power, modulation parameters, and more.
 * @note It also sets the DIO mapping for RxDone and sets the transceiver to STDBY mode.
 * @note Depending on the version of the transceiver, it sets the mode to RXCONTINUOUS or RXSINGLE.
 */

radio::sx1278::Status radio::sx1278::SX1278::init(
		uint32_t frequency,
		lora::Power power,
		lora::SpreadingFactor spreading_factor,
		lora::Bandwidth bandwidth,
		lora::CodingRate coding_rate,
		lora::HeaderMode header_mode,
		lora::LNAGain lna_gain,
		lora::PayloadCRC crc,
		uint16_t preamble_length,
		uint16_t timeout,
		uint8_t max_current
) {
	uint8_t read;
	reset();

	/** Set LoRa mode **/
	read = SPI_read<uint8_t>(RegisterAddress::RegOpMode).value();
	read |= 0x80;
	SPI_write(RegisterAddress::RegOpMode, read);

	/** Set frequency **/
	set_frequency(frequency);

	/** Set output power gain **/
	set_power(power);

	/** Set spreading factor **/
	set_spreading_factor(spreading_factor);

	/** Set bandwidth **/
	set_bandwidth(bandwidth);

	/** Set coding rate **/
	set_coding_rate(coding_rate);

	/** Set preamble length **/
	set_preamble_length(preamble_length);

	/** Set timeout **/
	set_timeout(timeout);

	/** Enable CRC **/
	set_payload_crc(crc);

	/** Set OCP **/
	set_ocp(max_current);

	/** Set header mode **/		
	set_header_mode(header_mode);

	/** Set LNA gain **/
	set_lna_gain(lna_gain);

	/** DIO mapping: --> DIO: RxDone **/
	read = SPI_read<uint8_t>(RegisterAddress::RegDioMapping1).value();
	read |= 0x3F;
	SPI_write(RegisterAddress::RegDioMapping1, read);

	/** RX/TX FIFO **/
	// We always use the entire FIFO for TX/RX operation
	SPI_write(lora::RegisterAddress::RegFifoRxBaseAddr, static_cast<uint8_t>(0x00));
	SPI_write(lora::RegisterAddress::RegFifoTxBaseAddr, static_cast<uint8_t>(0x00));

	/** Set mode to standby **/
	set_mode(lora::Mode::STDBY);

	if(get_version() == 0x12) {
		return Status::OK;

	} else {
		return Status::ERROR;
	}

}

void radio::sx1278::SX1278::on_dio0_irq() {
	// TODO: call RX DONE handler and stop radio
	if (this->_current_mode == lora::Mode::TX) {
		this->_handle_txdone_irq();
	}
	else if (this->_current_mode == lora::Mode::RXCONTINUOUS) {		
		this->_handle_rxdone_irq();
	}
}

void radio::sx1278::SX1278::_handle_txdone_irq() {
	this->set_mode(lora::Mode::RXCONTINUOUS);
}

void radio::sx1278::SX1278::_handle_rxdone_irq() {
	if (this->on_rx != nullptr)
		this->on_rx();

	this->startReceive();
}
