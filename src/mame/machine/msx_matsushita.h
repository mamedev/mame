// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_MACHINE_MSX_MATSUSHITA_H
#define MAME_MACHINE_MSX_MATSUSHITA_H


#include "msx_switched.h"


DECLARE_DEVICE_TYPE(MSX_MATSUSHITA, msx_matsushita_device)


class msx_matsushita_device : public device_t,
	public msx_switched_interface,
	public device_nvram_interface
{
public:
	msx_matsushita_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto turbo_callback() { return m_turbo_out_cb.bind(); }

	virtual uint8_t switched_read(offs_t offset) override;
	virtual void switched_write(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual ioport_constructor device_input_ports() const override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	required_ioport m_io_config;
	devcb_write_line m_turbo_out_cb;
	bool m_selected;
	uint16_t m_address;
	std::vector<uint8_t> m_sram;
	uint8_t m_nibble1;
	uint8_t m_nibble2;
	uint8_t m_pattern;
};

#endif // MAME_MACHINE_MSX_MATSUSHITA_H
