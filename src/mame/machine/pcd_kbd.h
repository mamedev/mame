// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef PCD_KBD_H_
#define PCD_KBD_H_

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "sound/speaker.h"

#define MCFG_PCD_KEYBOARD_OUT_TX_HANDLER(_devcb) \
	devcb = &pcd_keyboard_device::set_out_tx_handler(*device, DEVCB_##_devcb);

class pcd_keyboard_device :  public device_t
{
public:
	pcd_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_out_tx_handler(device_t &device, _Object object) { return downcast<pcd_keyboard_device &>(device).m_out_tx_handler.set_callback(object); }

	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	void device_start() override;

	DECLARE_READ8_MEMBER( bus_r );
	DECLARE_READ8_MEMBER( p1_r );
	DECLARE_WRITE8_MEMBER( p1_w );
	DECLARE_READ8_MEMBER( t0_r );
	DECLARE_WRITE_LINE_MEMBER( t0_w );
private:
	required_ioport_array<17> m_rows;
	UINT8 m_p1;
	bool m_t0;
	devcb_write_line m_out_tx_handler;
};

extern const device_type PCD_KEYBOARD;

#endif /* PCD_KBD_H_ */
