// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_MACHINE_MACKBD_H
#define MAME_MACHINE_MACKBD_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mackbd_device

class mackbd_device :  public device_t
{
public:
	// config helper
	auto clkout_handler() { return m_clkout_handler.bind(); }
	auto dataout_handler() { return m_dataout_handler.bind(); }
	void datain_w(int state);

	// construction/destruction
	mackbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	required_device<cpu_device> m_maincpu;
	required_ioport_array<9> m_col;
	devcb_write_line m_clkout_handler;
	devcb_write_line m_dataout_handler;
	emu_timer *m_sync;

	u8 p0, p1, p2;
	bool datain;

	void scan_kbd_col(int col);

	u8 p0_r();
	void p0_w(u8 data);
	u8 p1_r();
	void p1_w(u8 data);
	u8 p2_r();
	void p2_w(u8 data);
};

// device type definition
DECLARE_DEVICE_TYPE(MACKBD, mackbd_device)

#endif // MAME_MACHINE_MACKBD_H
