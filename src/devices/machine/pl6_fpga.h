// license:BSD-3-Clause
// copyright-holders: NaokiS
/*
	Heber Pluto 6 FPGA

	The FPGA is the backbone of the Pluto 6 system and handles a lot of items.

	Primarially it handles:
		* Part of the initial boot security in conjunction with the Pluto 6 CPLD.
		* Multiplexed inputs (32 or 64 through expansion)
		* Multiplexed lamps (256 or 512 through expansion)
		* Multiplexed LEDs (256 or 512 through expansion)
		* Register to I2S conversion
		* DMA requests for the DAC. This is probably an internally set timer at a fixed freq.
		* 2x UART controllers (probably styled on mc68681s for programmer convenience)
		* UART route mapping (all UART output ports are configurable)
		* Part of the game protection
		* GPU syncronisation

	Essentially, if you don't unlock it with a specific 256 byte (?), CRC obfuscated key,
	 you won't be able to do much of anything with the ColdFire, just read the CF card, PIC18
	 and the I2C bus, everything else is locked out.
	
	TODO:
		* Sound
		* UART
		* GPU Sync (?)
		* "receive" FPGA bitstream from PIC18
		* Should outputs be implemented directly in here, rather than a call back?

	"Bitstream" mappings:
		Each manufacturer could have their own bitstream made which would rearrange the register
		mapping, and change the CRC encoding and unlock key. It's possible this could be changed
		between games but it seems mostly just manufacturer based on a brief glance. To add a 
		mapping into this device, add a read and write function call and then add the manufacturer/ID
		to the PlutoFPGA enum. Finally make sure to update the dev_r and dev_w functions so that 
		the mapping can be used.

		It's possible that extra features could exist in some bitstreams, but it's unlikely this
		would have been done. Heber did a good job of shoehorning as much as they could into the chip.

		Supported bitstreams:
			* Developer 	- Note: Has no protection check, no CRC/key upload is needed.
			* Betcom		- Partial mapping
			* Top Dog		- Unimplemented
			* Astra			- Unimplemented
			* JPM			- Unimplemented

	Notes:
		FPGA UART routing is currently split into two functions. This is mainly due to the fact
			the FPGA has different register offsets for port 0 and port 1. ColdFire UART and the DUART
			share a common register and thus function.
*/

#ifndef MAME_MACHINE_PL6_FPGA_H
#define MAME_MACHINE_PL6_FPGA_H

#pragma once

#include "sound/dac.h"
#include "bus/rs232/rs232.h"

class pl6fpga_device : public device_t, public device_memory_interface
{
public:
	enum {
		developer_fpga,
		betcom_fpga,
		jpm_fpga,
		invalid_fpga
	};

	pl6fpga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// Outputs
	auto input_callback() { return input_cb.bind(); }
	auto lamp_callback() { return lamp_cb.bind(); }
	auto led_callback() { return led_cb.bind(); }
	auto output_callback() { return output_cb.bind(); }
	auto auxout_callback() { return auxout_cb.bind(); }

	// Duart device
	auto duart_r_callback() { return duart_r_cb.bind(); }
	auto duart_w_callback() { return duart_w_cb.bind(); }

	// RS232 ports
	auto cfuart_rx_a_callback() { return cfuart_rx_a_cb.bind(); }
	auto cfuart_rx_b_callback() { return cfuart_rx_b_cb.bind(); }
	auto duart_rx_a_callback() { return duart_rx_a_cb.bind(); }
	auto duart_rx_b_callback() { return duart_rx_b_cb.bind(); }
	void cfuart_tx1_w(int state){ uart_txd(coldfire_uart_0, state ); }
	void cfuart_tx2_w(int state){ uart_txd(coldfire_uart_1, state ); }
	void duart_tx_a_w(int state){ uart_txd(duart_uart_0, state ); }
	void duart_tx_b_w(int state){ uart_txd(duart_uart_1, state ); }

	void rs232a_tx_w(int state) { uart_txd(endpoint_rs232a, state); }
	void bacta_tx_w(int state) { uart_txd(endpoint_bacta, state); }
	void rs232c_tx_w(int state) { uart_txd(endpoint_rs232c, state); }
	void rs232d_tx_w(int state) { uart_txd(endpoint_rs232d, state); }
	void cctalk1_tx_w(int state) { uart_txd(endpoint_cctalk1, state); }
	void cctalk2_tx_w(int state) { uart_txd(endpoint_cctalk2, state); }
	void serttl_tx_w(int state) { uart_txd(endpoint_ttl, state); }
	void rs485_tx_w(int state) { uart_txd(endpoint_rs485, state); }

	void rs232a_rx_w(int state) { uart_rxd(endpoint_rs232a, state); }
	void bacta_rx_w(int state) { uart_rxd(endpoint_bacta, state); }
	void rs232c_rx_w(int state) { uart_rxd(endpoint_rs232c, state); }
	void rs232d_rx_w(int state) { uart_rxd(endpoint_rs232d, state); }
	void cctalk1_rx_w(int state) { uart_rxd(endpoint_cctalk1, state); }
	void cctalk2_rx_w(int state) { uart_rxd(endpoint_cctalk2, state); }
	void serttl_rx_w(int state) { uart_rxd(endpoint_ttl, state); }
	void rs485_rx_w(int state) { uart_rxd(endpoint_rs485, state); }

	// Driver device
	uint32_t dev_r(offs_t offset, uint32_t mem_mask);
	void dev_w(offs_t offset, uint32_t mem_mask, uint32_t data);

	void set_fpga_type(int type);

	static constexpr feature_type unemulated_features() { return feature::SOUND; }
	static constexpr feature_type imperfect_features() { return feature::PROTECTION; }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual ioport_constructor device_input_ports() const override;
	virtual space_config_vector memory_space_config() const override;

private:
	address_space_config m_space_config;

	// DACs are here really because the FPGA has direct control over the audio and its already silly with callbacks
	required_device<dac_word_interface> m_ldac;
	required_device<dac_word_interface> m_rdac;

	// Need a serial port?
	required_device<rs232_port_device> m_rs232a;
	required_device<rs232_port_device> m_bacta;
	required_device<rs232_port_device> m_rs232c;
	required_device<rs232_port_device> m_rs232d;
	required_device<rs232_port_device> m_cctalk1;
	required_device<rs232_port_device> m_cctalk2;
	required_device<rs232_port_device> m_serttl;
	required_device<rs232_port_device> m_rs485;

	void uart_txd(uint8_t device, int state);
	void uart_rxd(uint8_t device, int state);

	uint8_t uart_route_map[6] = {
		endpoint_invalid,	// ColdFire 0
		endpoint_invalid,	// ColdFire 1
		endpoint_invalid,	// FPGA UART 0
		endpoint_invalid,	// FPGA UART 1
		endpoint_invalid,	// DUART 0
		endpoint_invalid	// DUART 1
	};

	devcb_read32 input_cb;
	devcb_write8 lamp_cb;
	devcb_write8 led_cb;
	devcb_write8 output_cb;
	devcb_write8 auxout_cb;

	devcb_write_line write_irq, cfuart_rx_a_cb, cfuart_rx_b_cb, duart_rx_a_cb, duart_rx_b_cb;
	devcb_read8 duart_r_cb;
	devcb_write8 duart_w_cb;

	required_ioport m_in1;

	enum{
		GSYNC_EN 	= 0x10,
		MPX_EN 		= 0x20,
		FINT_EN 	= 0x40,
		SFX_EN 		= 0x80
	};

	// FPGA
	int fpga_type;
	
	uint8_t xcra_reg;
	uint8_t xuctrl_reg;
	uint8_t xdurt_reg;
	uint8_t xfpurt0_reg;
	uint8_t xfpurt1_reg;

	// Lamps
	uint8_t mpx_row;				// 0x00-0x0F range
	uint32_t mpx_regs[16][8];		// MPX Lamp/LED state and period data.

	// Mappings
	void developer_map(address_map &map);
	void betcom_map(address_map &map);
	void jpm_map(address_map &map);

	// Read
	uint8_t fpga_id() { return 0x4d; }
	uint8_t fpga_iss() { return 0x40; }
	uint8_t xcra_read();
	uint8_t xuctrl_read(){ return xuctrl_reg; }
	uint8_t xdurt_read(){ return xdurt_reg; }
	uint8_t xfpurt0_read(){ return xfpurt0_reg; }
	uint8_t xfpurt1_read(){ return xfpurt1_reg; }

	// Write
	void xcra_write(uint8_t data);
	void sfxv_write(uint8_t data);
	void sfxr1_write(uint16_t data);
	void sfxr2_write(uint16_t data);
	void xuctrl_write(uint8_t data);
	void xdurt_write(uint8_t data);
	void xfpurt0_write(uint8_t data);
	void xfpurt1_write(uint8_t data);
	void xmpx_write(uint8_t data);
	void aux_write(offs_t offset, uint8_t data);
	void lamp_write(offs_t offset, uint32_t data);
	void output_write(offs_t offset, uint32_t data);
	void crcchk_write(uint8_t data){}

	uint8_t duart_r(offs_t offset){ return duart_r_cb(offset); }
	void duart_w(offs_t offset, uint8_t data) { duart_w_cb(offset, data); }
	uint32_t input_r(offs_t offset, uint32_t mem_mask = ~0){ return input_cb(offset); }

	void map_uart_endpoint(uint8_t device, uint8_t route);

	enum {
		endpoint_bacta 		= 0x00,
		endpoint_rs232a 	= 0x01,
		endpoint_rs232c 	= 0x02,
		endpoint_rs232d 	= 0x03,
		endpoint_cctalk1 	= 0x04,
		endpoint_cctalk2 	= 0x05,
		endpoint_ttl		= 0x06,
		endpoint_rs485		= 0x07,
		endpoint_invalid
	};

	enum {
		coldfire_uart_0,
		coldfire_uart_1,
		fpga_uart_0,
		fpga_uart_1,
		duart_uart_0,
		duart_uart_1,
		device_invalid
	};

};

DECLARE_DEVICE_TYPE(HEBER_PLUTO6_FPGA, pl6fpga_device)

#endif // MAME_MACHINE_PL6_FPGA_H
