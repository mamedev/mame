// license:BSD-3-Clause
// copyright-holders:smf, windyfairy
/*
 * Konami 573 Memory Card Reader
 *
 */
#ifndef MAME_KONAMI_K573_MCR_H
#define MAME_KONAMI_K573_MCR_H

#pragma once

#include "bus/psx/ctlrport.h"
#include "machine/jvsdev.h"
#include "machine/timer.h"

class k573mcr_device : public jvs_device
{
public:
	template <typename T>
	k573mcr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&jvs_host_tag)
		: k573mcr_device(mconfig, tag, owner, clock)
	{
		host.set_tag(std::forward<T>(jvs_host_tag));
	}

	k573mcr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	void write_rxd(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// JVS device overrides
	virtual const char *device_id() override;
	virtual uint8_t command_format_version() override;
	virtual uint8_t jvs_standard_version() override;
	virtual uint8_t comm_method_version() override;
	virtual int handle_message(const uint8_t *send_buffer, uint32_t send_size, uint8_t *&recv_buffer) override;

private:
	enum {
		MEMCARD_UNINITIALIZED = 0x0000, // Default value, also used after writing?
		MEMCARD_ERROR         = 0x0002,
		MEMCARD_UNAVAILABLE   = 0x0008, // Card is not inserted
		MEMCARD_READING       = 0x0200, // Read request is executing
		MEMCARD_WRITING       = 0x0400, // Write request is executing
		MEMCARD_AVAILABLE     = 0x8000  // Can be combined with MEMCARD_READING and MEMCARD_WRITING for busy state
	};

	enum {
		RAM_SIZE = 0x400000,
		MEMCARD_BLOCK_SIZE = 128
	};

	void controller_set_port(uint32_t port_no);
	uint8_t controller_port_send_byte(uint8_t data);
	bool pad_read(uint32_t port_no, uint8_t *output);
	bool memcard_read(uint32_t port_no, uint16_t block_addr, uint8_t *output);
	bool memcard_write(uint32_t port_no, uint16_t block_addr, uint8_t *input);

	std::unique_ptr<uint8_t[]> m_ram;
	uint16_t m_status;
	bool m_is_memcard_initialized;
	uint8_t m_psx_rx_data, m_psx_rx_bit;
	bool m_psx_clock;

	required_device<psxcontrollerports_device> m_controllers;
	required_ioport m_meta;
};

DECLARE_DEVICE_TYPE(KONAMI_573_MEMORY_CARD_READER, k573mcr_device)

#endif // MAME_KONAMI_K573_MCR_H
