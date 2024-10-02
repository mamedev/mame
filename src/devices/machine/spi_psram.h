// license:BSD-3-Clause
// copyright-holders:Vas Crabb
// RAM with SPI/SDI/SQI/QPI interface
#ifndef MAME_MACHINE_SPI_PSRAM_H
#define MAME_MACHINE_SPI_PSRAM_H

#pragma once

#include <memory>


class spi_psram_device : public device_t
{
public:
	spi_psram_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0U);
	virtual ~spi_psram_device();

	void set_size(u32 size) { m_size = size; }
	auto sio_cb() { return m_sio_cb.bind(); }

	void ce_w(int state);
	void sclk_w(int state);
	void sio_w(offs_t offset, u8 data, u8 mem_mask);
	void si_w(int state) { sio_w(0, state ? 0xf : 0xe, 0x1); }

protected:
	virtual void device_validity_check(validity_checker &valid) const override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	void start_command();
	void address_complete();
	void next_address();

private:
	enum class phase : u8;
	enum command : u8;

	devcb_write8 m_sio_cb;

	std::unique_ptr<u8 []> m_ram;
	u32 m_size;

	u32 m_wrap_mask, m_addr;
	u32 m_buffer;
	u8 m_cmd_width, m_data_width;
	u8 m_bits;

	u8 m_ce, m_sclk, m_sio;
	phase m_phase;
	u8 m_cmd;
};


DECLARE_DEVICE_TYPE(SPI_PSRAM, spi_psram_device)

#endif // MAME_MACHINE_SPI_PSRAM_H
