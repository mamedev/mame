// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef __MSX_MATSUSHITA_H
#define __MSX_MATSUSHITA_H


#include "msx_switched.h"
#include "machine/nvram.h"


extern const device_type MSX_MATSUSHITA;


#define MCFG_MSX_MATSUSHITA_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MSX_MATSUSHITA, 0)

#define MCFG_MSX_MATSUSHITA_TURBO_CB(_devcb) \
	devcb = &msx_matsushita_device::set_turbo_callback(*device, DEVCB_##_devcb);


class msx_matsushita_device : public msx_switched_device
{
public:
	msx_matsushita_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &set_turbo_callback(device_t &device, _Object object) { return downcast<msx_matsushita_device &>(device).m_turbo_out_cb.set_callback(object); }

	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	virtual uint8_t get_id() override;

	virtual uint8_t io_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void io_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

private:
	required_ioport m_io_config;
	required_device<nvram_device> m_nvram;
	devcb_write_line m_turbo_out_cb;
	uint16_t m_address;
	std::vector<uint8_t> m_sram;
	uint8_t m_nibble1;
	uint8_t m_nibble2;
	uint8_t m_pattern;
};

#endif
