// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/*******************************************************************************

    Buffered RAM circuit.

    Michael Zapf
    March 2020

*******************************************************************************/

#ifndef MAME_BUS_TI99_INTERNAL_BRAM_H
#define MAME_BUS_TI99_INTERNAL_BRAM_H

#pragma once

namespace bus::ti99::internal {

// =========== Buffered RAM ================
class buffered_ram_device : public device_t, public device_nvram_interface
{
public:
	buffered_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	buffered_ram_device &set_size(int size) { m_size = size; return *this; }

	// read/write
	uint8_t read(offs_t offset)         { return m_mem[offset % m_size]; }
	void write(offs_t offset, uint8_t data)  { m_mem[offset % m_size] = data; }

	void set_buffered(bool on) { nvram_enable_backup(on); }

private:
	virtual void device_start() override ATTR_COLD;

	// derived class overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	int m_size;
	std::unique_ptr<u8 []>      m_mem;
};


} // end namespace bus::ti99::internal

DECLARE_DEVICE_TYPE_NS(BUFF_RAM, bus::ti99::internal, buffered_ram_device)

#endif // MAME_BUS_TI99_INTERNAL_BRAM_H
