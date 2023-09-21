#include "SX1278.h"

/**
 * @brief  Initializes a new SX1278 module with default configuration values.
 * @note   After calling this function, certain hardware-specific parameters such as SPI interface
 *         and GPIO pins need to be set manually. These parameters include hSPIx, NSS_port, NSS_pin,
 *         RESET_port, RESET_pin, DIO0_port, and DIO0_pin.
 * @retval SX1278 A new SX1278 module instance with default configuration values.
 */
SX1278 SX1278_new(void)
{
	SX1278 newModule;

	newModule.frequency = 433;
	newModule.spredingFactor = SX1278_SF_7;
	newModule.bandWidth = SX1278_BW_250KHz;
	newModule.crcRate = SX1278_CR_4_5;
	newModule.power = SX1278_POWER_20db;
	newModule.overCurrentProtection = 100;
	newModule.preamble = 8;

	return newModule;
}

/**
 * @brief  Performs a hardware reset on the SX1278 module.
 * @note   This function will pull the RESET pin low for a short duration, then release it,
 *         effectively resetting the SX1278 module. After releasing the RESET pin,
 *         it waits for a longer duration to ensure the module has fully initialized.
 * @param  module Pointer to an SX1278 structure that contains the configuration
 *                information for the specified module, including the GPIO port and pin
 *                associated with the RESET function.
 * @retval None
 */
void SX1278_reset(SX1278 *module)
{
	HAL_GPIO_WritePin(module->RESET_port, module->RESET_pin, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(module->RESET_port, module->RESET_pin, GPIO_PIN_SET);
	HAL_Delay(100);
}

/**
 * @brief  Changes the operational mode of the SX1278 module.
 * @note   This function updates the SX1278 module's mode based on the provided mode parameter.
 *         It reads the current mode from the SX1278_RegOpMode register, modifies the relevant bits
 *         to set the desired mode, and then writes the updated value back to the register.
 *         Additionally, for specific modes, it sets the DIO0 interrupt to the appropriate event.
 *
 * @param  module Pointer to an SX1278 structure that contains the configuration
 *                information for the specified module.
 * @param  mode The desired operational mode for the SX1278 module. Valid modes are:
 *              - SX1278_SLEEP_MODE
 *              - SX1278_STNBY_MODE
 *              - SX1278_TX_MODE
 *              - SX1278_RXCONTIN_MODE
 *
 * @retval None
 */
void SX1278_changeMode(SX1278 *module, int mode)
{
	uint8_t read;
	uint8_t data;

	read = SX1278_read(module, SX1278_RegOpMode);
	data = read;

	if (mode == SX1278_SLEEP_MODE)
	{
		data = (read & 0xF8) | 0x00;
		module->current_mode = SX1278_SLEEP_MODE;
	}
	else if (mode == SX1278_STNBY_MODE)
	{
		data = (read & 0xF8) | 0x01;
		module->current_mode = SX1278_STNBY_MODE;
	}
	else if (mode == SX1278_TX_MODE)
	{
		data = (read & 0xF8) | 0x03;
		module->current_mode = SX1278_TX_MODE;
		SX1278_write(module, SX1278_RegDioMapping1, 0x40); // set dio0 IRQ to TxDone
	}
	else if (mode == SX1278_RXCONTIN_MODE)
	{
		data = (read & 0xF8) | 0x05;
		module->current_mode = SX1278_RXCONTIN_MODE;
		SX1278_write(module, SX1278_RegDioMapping1, 0x00); // set dio0 IRQ to RxDone
	}

	SX1278_write(module, SX1278_RegOpMode, data);
}

/**
 * @brief  Reads data from a specific register of the SX1278 module using SPI communication.
 * @note   This function initiates an SPI transaction to read data from the SX1278 module.
 *         It first pulls the NSS (Chip Select) pin low to start the SPI transaction,
 *         then transmits the address of the register to be read, waits for the SPI
 *         transmission to complete, receives the data from the module, waits for the
 *         SPI reception to complete, and finally releases the NSS pin to end the transaction.
 *
 * @param  module   Pointer to an SX1278 structure that contains the configuration
 *                  information for the specified module.
 * @param  address  Pointer to the address of the register to be read.
 * @param  r_length Length of the address data to be transmitted.
 * @param  output   Pointer to the buffer where the received data will be stored.
 * @param  w_length Length of the data to be received.
 *
 * @retval None
 */
static void SX1278_readReg(SX1278 *module, uint8_t *address, uint16_t r_length, uint8_t *output, uint16_t w_length)
{
	HAL_GPIO_WritePin(module->NSS_port, module->NSS_pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(module->hSPIx, address, r_length, SX1278_TX_TIMEOUT);
	while (HAL_SPI_GetState(module->hSPIx) != HAL_SPI_STATE_READY)
		;
	HAL_SPI_Receive(module->hSPIx, output, w_length, SX1278_RX_TIMEOUT);
	while (HAL_SPI_GetState(module->hSPIx) != HAL_SPI_STATE_READY)
		;
	HAL_GPIO_WritePin(module->NSS_port, module->NSS_pin, GPIO_PIN_SET);
}

/**
 * @brief  Writes data to a specific register of the SX1278 module using SPI communication.
 * @note   This function initiates an SPI transaction to write data to the SX1278 module.
 *         It first pulls the NSS (Chip Select) pin low to start the SPI transaction,
 *         then transmits the address of the register to be written to, waits for the SPI
 *         transmission to complete, transmits the values to be written to the register,
 *         waits for the SPI transmission to complete, and finally releases the NSS pin
 *         to end the transaction.
 *
 * @param  module   Pointer to an SX1278 structure that contains the configuration
 *                  information for the specified module.
 * @param  address  Pointer to the address of the register to be written to.
 * @param  r_length Length of the address data to be transmitted.
 * @param  values   Pointer to the data values to be written to the register.
 * @param  w_length Length of the data values to be transmitted.
 *
 * @retval None
 */
static void SX1278_writeReg(SX1278 *module, uint8_t *address, uint16_t r_length, uint8_t *values, uint16_t w_length)
{
	HAL_GPIO_WritePin(module->NSS_port, module->NSS_pin, GPIO_PIN_RESET);

	HAL_SPI_Transmit(module->hSPIx, address, r_length, SX1278_TX_TIMEOUT);
	while (HAL_SPI_GetState(module->hSPIx) != HAL_SPI_STATE_READY)
		; // wait for SPI

	HAL_SPI_Transmit(module->hSPIx, values, w_length, SX1278_TX_TIMEOUT);
	while (HAL_SPI_GetState(module->hSPIx) != HAL_SPI_STATE_READY)
		; // wait for SPI

	HAL_GPIO_WritePin(module->NSS_port, module->NSS_pin, GPIO_PIN_SET);
}

/**
 * @brief  Sets the operating frequency of the SX1278 module.
 * @note   This function calculates the frequency value based on the provided frequency
 *         and writes it to the appropriate SX1278 registers (Msb, Mid, Lsb). The frequency
 *         is set by writing to three registers: RegFrMsb, RegFrMid, and RegFrLsb.
 *         Delays are added between consecutive writes to ensure proper setting.
 *
 * @param  module Pointer to an SX1278 structure that contains the configuration
 *                information for the specified module.
 * @param  freq   Desired operating frequency in MHz (e.g., 433 for 433MHz).
 *
 * @retval None
 */
void SX1278_setFrequency(SX1278 *module, int freq)
{
	uint8_t data;
	uint32_t F;
	F = (freq * 524288) >> 5;

	// write Msb:
	data = F >> 16;
	SX1278_write(module, SX1278_RegFrMsb, data);
	HAL_Delay(5);

	// write Mid:
	data = F >> 8;
	SX1278_write(module, SX1278_RegFrMid, data);
	HAL_Delay(5);

	// write Lsb:
	data = F >> 0;
	SX1278_write(module, SX1278_RegFrLsb, data);
	HAL_Delay(5);
}


/**
  * @brief  Sets the spreading factor for the SX1278 module.
  * @note   The spreading factor determines the duration of a single symbol, 
  *         which affects the data rate and range of the transmission. This function 
  *         ensures that the provided spreading factor is within the valid range (7-12) 
  *         and then sets it in the SX1278_RegModemConfig2 register.
  * 
  * @param  module Pointer to an SX1278 structure that contains the configuration 
  *                information for the specified module.
  * @param  SF     Desired spreading factor value (should be between 7 and 12 inclusive).
  * 
  * @retval None
  */
void SX1278_setSpreadingFactor(SX1278 *module, int SF)
{
	uint8_t data;
	uint8_t read;

	if (SF > 12)
		SF = 12;
	if (SF < 7)
		SF = 7;

	read = SX1278_read(module, SX1278_RegModemConfig2);
	HAL_Delay(10);

	data = (SF << 4) + (read & 0x0F);
	SX1278_write(module, SX1278_RegModemConfig2, data);
	HAL_Delay(10);
}

/**
  * @brief  Sets the transmission power for the SX1278 module.
  * @note   This function writes the desired power value to the SX1278_RegPaConfig 
  *         register to adjust the module's transmission power. A delay is added 
  *         after writing to ensure the setting takes effect.
  * 
  * @param  module Pointer to an SX1278 structure that contains the configuration 
  *                information for the specified module.
  * @param  power  Desired power value to set for transmission.
  * 
  * @retval None
  */
void SX1278_setPower(SX1278 *module, uint8_t power)
{
	SX1278_write(module, SX1278_RegPaConfig, power);
	HAL_Delay(10);
}

/**
  * @brief  Sets the Over Current Protection (OCP) value for the SX1278 module.
  * @note   This function configures the OCP value based on the provided current.
  *         The function ensures the current value is within the valid range (45-240 mA)
  *         and calculates the OcpTrim value accordingly. The OcpTrim value is then written 
  *         to the SX1278_RegOcp register to set the OCP. A delay is added after writing 
  *         to ensure the setting takes effect.
  * 
  * @param  module  Pointer to an SX1278 structure that contains the configuration 
  *                 information for the specified module.
  * @param  current Desired current value (in mA) for OCP. Valid range: 45 to 240 mA.
  * 
  * @retval None
  */
void SX1278_setOCP(SX1278 *module, uint8_t current)
{
	uint8_t OcpTrim = 0;

	if (current < 45)
		current = 45;
	if (current > 240)
		current = 240;

	if (current <= 120)
		OcpTrim = (current - 45) / 5;
	else if (current <= 240)
		OcpTrim = (current + 30) / 10;

	OcpTrim = OcpTrim + (1 << 5);
	SX1278_write(module, SX1278_RegOcp, OcpTrim);
	HAL_Delay(10);
}

/**
  * @brief  Sets the Time-On-Air MSB and enables CRC on the SX1278 module.
  * @note   This function reads the current value of the SX1278_RegModemConfig2 register,
  *         modifies the three least significant bits to '1' (indicating Time-On-Air MSB and 
  *         enabling CRC), and then writes the modified value back to the register. 
  *         A delay is added after writing to ensure the setting takes effect.
  * 
  * @param  module  Pointer to an SX1278 structure that contains the configuration 
  *                 information for the specified module.
  * 
  * @retval None
  */
void SX1278_setTOMsb_setCRCon(SX1278 *module)
{
	uint8_t read, data;

	read = SX1278_read(module, SX1278_RegModemConfig2);

	data = read | 0x07;
	SX1278_write(module, SX1278_RegModemConfig2, data);
	HAL_Delay(10);
}

/**
  * @brief  Reads a byte of data from the specified address of the SX1278 module.
  * @note   This function first masks the provided address with 0x7F to ensure it's 
  *         in the valid range for reading. It then calls the SX1278_readReg function 
  *         to perform the actual SPI read operation.
  * 
  * @param  module   Pointer to an SX1278 structure that contains the configuration 
  *                  information for the specified module.
  * @param  address  The address of the register in the SX1278 module from which 
  *                  the data needs to be read.
  * 
  * @retval read_data The byte of data read from the specified address.
  */
uint8_t SX1278_read(SX1278 *module, uint8_t address)
{
	uint8_t read_data;
	uint8_t data_addr;

	data_addr = address & 0x7F;
	SX1278_readReg(module, &data_addr, 1, &read_data, 1);

	return read_data;
}

/**
  * @brief  Writes a byte of data to the specified address of the SX1278 module.
  * @note   This function first sets the MSB of the provided address to ensure it's 
  *         in write mode. It then calls the SX1278_writeReg function to perform 
  *         the actual SPI write operation.
  * 
  * @param  module   Pointer to an SX1278 structure that contains the configuration 
  *                  information for the specified module.
  * @param  address  The address of the register in the SX1278 module to which 
  *                  the data needs to be written.
  * @param  value    The byte of data to be written to the specified address.
  */
void SX1278_write(SX1278 *module, uint8_t address, uint8_t value)
{
	uint8_t data;
	uint8_t addr;

	addr = address | 0x80;
	data = value;
	SX1278_writeReg(module, &addr, 1, &data, 1);
}

/**
  * @brief  Writes a sequence of bytes to the specified address of the SX1278 module.
  * @note   This function is used for burst writing, allowing multiple bytes to be 
  *         written in a single operation. It first sets the MSB of the provided 
  *         address to ensure it's in write mode. The function then performs the 
  *         SPI write operation for the given length of data.
  * 
  * @param  module   Pointer to an SX1278 structure that contains the configuration 
  *                  information for the specified module.
  * @param  address  The starting address of the register in the SX1278 module to 
  *                  which the data needs to be written.
  * @param  value    Pointer to the sequence of bytes to be written to the specified address.
  * @param  length   The number of bytes to be written.
  */
void SX1278_BurstWrite(SX1278 *module, uint8_t address, uint8_t *value, uint8_t length)
{
	uint8_t addr;
	addr = address | 0x80;

	HAL_GPIO_WritePin(module->NSS_port, module->NSS_pin, GPIO_PIN_RESET);

	HAL_SPI_Transmit(module->hSPIx, &addr, 1, SX1278_TX_TIMEOUT);
	while (HAL_SPI_GetState(module->hSPIx) != HAL_SPI_STATE_READY)
		; // wait for SPI

	HAL_SPI_Transmit(module->hSPIx, value, length, SX1278_TX_TIMEOUT);
	while (HAL_SPI_GetState(module->hSPIx) != HAL_SPI_STATE_READY)
		; // wait for SPI

	HAL_GPIO_WritePin(module->NSS_port, module->NSS_pin, GPIO_PIN_SET);
}

/**
  * @brief  Transmits a sequence of bytes using the SX1278 module.
  * @note   This function prepares the SX1278 module for transmission by setting it 
  *         to standby mode. It then sets the FIFO pointer to the TX base address 
  *         and specifies the payload length. After writing the data to the FIFO, 
  *         the function changes the module mode to transmit mode to start the transmission.
  * 
  * @param  module   Pointer to an SX1278 structure that contains the configuration 
  *                  information for the specified module.
  * @param  data     Pointer to the sequence of bytes to be transmitted.
  * @param  length   The number of bytes to be transmitted.
  */
void SX1278_transmit(SX1278 *module, uint8_t *data, uint8_t length)
{
	uint8_t read;

	SX1278_changeMode(module, SX1278_STNBY_MODE);
	read = SX1278_read(module, SX1278_RegFiFoTxBaseAddr);
	SX1278_write(module, SX1278_RegFiFoAddPtr, read);
	SX1278_write(module, SX1278_RegPayloadLength, length);
	SX1278_BurstWrite(module, SX1278_RegFiFo, data, length);
	SX1278_changeMode(module, SX1278_TX_MODE);
}

/**
  * @brief  Receives a sequence of bytes using the SX1278 module.
  * @note   This function prepares the SX1278 module for reception by setting it 
  *         to standby mode. It then checks the IRQ flags to determine if a packet 
  *         has been received. If a packet is available, the function reads the 
  *         number of received bytes and the current address in the FIFO where 
  *         the last packet was written. It then sets the FIFO pointer to this address 
  *         and reads the received data into the provided buffer. After reading the data, 
  *         the function sets the module back to continuous reception mode.
  * 
  * @param  module   Pointer to an SX1278 structure that contains the configuration 
  *                  information for the specified module.
  * @param  data     Pointer to the buffer where the received data will be stored.
  * @param  length   The maximum number of bytes that can be stored in the buffer.
  * 
  * @return The number of bytes actually received and stored in the buffer.
  */
uint8_t SX1278_receive(SX1278 *module, uint8_t *data, uint8_t length)
{
	uint8_t read;
	uint8_t number_of_bytes;
	uint8_t min = 0;

	memset(data, 0, length);

	SX1278_changeMode(module, SX1278_STNBY_MODE);
	read = SX1278_read(module, SX1278_RegIrqFlags);
	if ((read & 0x40) != 0)
	{
		SX1278_write(module, SX1278_RegIrqFlags, 0xFF);
		number_of_bytes = SX1278_read(module, SX1278_RegRxNbBytes);
		read = SX1278_read(module, SX1278_RegFiFoRxCurrentAddr);
		SX1278_write(module, SX1278_RegFiFoAddPtr, read);
		min = length >= number_of_bytes ? number_of_bytes : length;
		for (int i = 0; i < min; i++)
			data[i] = SX1278_read(module, SX1278_RegFiFo);
	}
	SX1278_changeMode(module, SX1278_RXCONTIN_MODE);
	return min;
}

/**
  * @brief  Retrieves the RSSI (Received Signal Strength Indicator) value of the last received packet.
  * @note   The RSSI value is read from the SX1278's RegPktRssiValue register. The actual RSSI 
  *         value in dBm is calculated by adding the read value to -164.
  * 
  * @param  module   Pointer to an SX1278 structure that contains the configuration 
  *                  information for the specified module.
  * 
  * @return The RSSI value of the last received packet in dBm.
  */
int SX1278_getRSSI(SX1278 *module)
{
	uint8_t read = SX1278_read(module, SX1278_RegPktRssiValue);
	return -164 + read;
}

/**
  * @brief  Initializes the SX1278 module with the specified configuration.
  * @note   This function performs the necessary steps to configure the SX1278 module 
  *         based on the parameters set in the SX1278 structure. It sets the module's 
  *         operating mode, frequency, power gain, over current protection, LNA gain, 
  *         spreading factor, bandwidth, coding rate, preamble length, and DIO mapping.
  *         After configuration, it checks the version of the SX1278 module to ensure 
  *         that the module is correctly initialized.
  * 
  * @param  module   Pointer to an SX1278 structure that contains the configuration 
  *                  information for the specified module.
  * 
  * @return SX1278_OK if the module's version matches the expected value, indicating 
  *         successful initialization. Returns SX1278_NOT_FOUND if the version does not match.
  */
uint16_t SX1278_init(SX1278 *module)
{
	uint8_t data;
	uint8_t read;

	// goto sleep mode:
	SX1278_changeMode(module, SX1278_SLEEP_MODE);
	HAL_Delay(10);

	// turn on SX1278 mode:
	read = SX1278_read(module, SX1278_RegOpMode);
	HAL_Delay(10);
	data = read | 0x80;
	SX1278_write(module, SX1278_RegOpMode, data);
	HAL_Delay(100);

	// set frequency:
	SX1278_setFrequency(module, module->frequency);

	// set output power gain:
	SX1278_setPower(module, module->power);

	// set over current protection:
	SX1278_setOCP(module, module->overCurrentProtection);

	// set LNA gain:
	SX1278_write(module, SX1278_RegLna, 0x23);

	// set spreading factor, CRC on, and Timeout Msb:
	SX1278_setTOMsb_setCRCon(module);
	SX1278_setSpreadingFactor(module, module->spredingFactor);

	// set Timeout Lsb:
	SX1278_write(module, SX1278_RegSymbTimeoutL, 0xFF);

	// set bandwidth, coding rate and expilicit mode:
	data = 0;
	data = (module->bandWidth << 4) + (module->crcRate << 1);
	SX1278_write(module, SX1278_RegModemConfig1, data);

	// set preamble:
	SX1278_write(module, SX1278_RegPreambleMsb, module->preamble >> 8);
	SX1278_write(module, SX1278_RegPreambleLsb, module->preamble >> 0);

	// DIO mapping:   --> DIO: RxDone
	read = SX1278_read(module, SX1278_RegDioMapping1);
	data = read | 0x3F;
	SX1278_write(module, SX1278_RegDioMapping1, data);

	// goto standby mode:
	SX1278_changeMode(module, SX1278_STNBY_MODE);
	module->current_mode = SX1278_STNBY_MODE;
	HAL_Delay(10);

	read = SX1278_read(module, SX1278_RegVersion);
	if (read == 0x12)
		return SX1278_OK;
	else
		return SX1278_NOT_FOUND;
}