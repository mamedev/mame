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
	msx_matsushita_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_turbo_callback(device_t &device, _Object object) { return downcast<msx_matsushita_device &>(device).m_turbo_out_cb.set_callback(object); }

	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	virtual UINT8 get_id() override;

	virtual DECLARE_READ8_MEMBER(io_read) override;
	virtual DECLARE_WRITE8_MEMBER(io_write) override;

private:
	required_ioport m_io_config;
	required_device<nvram_device> m_nvram;
	devcb_write_line m_turbo_out_cb;
	UINT16 m_address;
	dynamic_buffer m_sram;
	UINT8 m_nibble1;
	UINT8 m_nibble2;
	UINT8 m_pattern;
};

#endif
