// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/**  Konami 053252  **/
/* CRT and interrupt control unit */
#ifndef MAME_MACHINE_K053252_H
#define MAME_MACHINE_K053252_H

#pragma once

#include "screen.h"

class k053252_device : public device_t, public device_video_interface
{
public:
	k053252_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto int1_en() { return m_int1_en_cb.bind(); }
	auto int2_en() { return m_int2_en_cb.bind(); }
	auto int1_ack() { return m_int1_ack_cb.bind(); }
	auto int2_ack() { return m_int2_ack_cb.bind(); }
	auto int_time() { return m_int_time_cb.bind(); }
	void set_offsets(int offsx, int offsy) { m_offsx = offsx; m_offsy = offsy; }

	uint8_t read(offs_t offset);  // CCU registers
	void write(offs_t offset, uint8_t data);

	void res_change();

	void set_slave_screen(const char *tag) { m_slave_screen.set_tag(tag); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override { reset_internal_state(); }
	void reset_internal_state();

	private:
	// internal state
	uint8_t   m_regs[16];
	uint16_t  m_hc,m_hfp,m_hbp;
	uint16_t  m_vc,m_vfp,m_vbp;
	uint8_t   m_vsw,m_hsw;

	devcb_write_line   m_int1_en_cb;
	devcb_write_line   m_int2_en_cb;
	devcb_write_line   m_int1_ack_cb;
	devcb_write_line   m_int2_ack_cb;
	devcb_write8       m_int_time_cb;
	int                m_offsx;
	int                m_offsy;

	optional_device<screen_device> m_slave_screen;
};

DECLARE_DEVICE_TYPE(K053252, k053252_device)

#endif  // MAME_MACHINE_K053252_H
