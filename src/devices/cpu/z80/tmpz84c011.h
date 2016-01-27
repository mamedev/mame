// license:BSD-3-Clause
// copyright-holders:David Haywood,hap
/***************************************************************************

    Toshiba TMPZ84C011, MPUZ80/TLCS-Z80 ASSP Family
    Z80 CPU, CTC, CGC, I/O8x5

***************************************************************************/

#pragma once

#ifndef __TMPZ84C011__
#define __TMPZ84C011__

#include "emu.h"
#include "z80.h"
#include "machine/z80ctc.h"


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

// For daisy chain configuration, insert this:
#define TMPZ84C011_DAISY_INTERNAL { "tmpz84c011_ctc" }

// CTC callbacks
#define MCFG_TMPZ84C011_ZC0_CB(_devcb) \
	devcb = &tmpz84c011_device::set_zc0_callback(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C011_ZC1_CB(_devcb) \
	devcb = &tmpz84c011_device::set_zc1_callback(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C011_ZC2_CB(_devcb) \
	devcb = &tmpz84c011_device::set_zc2_callback(*device, DEVCB_##_devcb);


// I/O callbacks
#define MCFG_TMPZ84C011_PORTA_READ_CB(_devcb) \
	devcb = &tmpz84c011_device::set_inportsa_cb(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C011_PORTB_READ_CB(_devcb) \
	devcb = &tmpz84c011_device::set_inportsb_cb(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C011_PORTC_READ_CB(_devcb) \
	devcb = &tmpz84c011_device::set_inportsc_cb(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C011_PORTD_READ_CB(_devcb) \
	devcb = &tmpz84c011_device::set_inportsd_cb(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C011_PORTE_READ_CB(_devcb) \
	devcb = &tmpz84c011_device::set_inportse_cb(*device, DEVCB_##_devcb);


#define MCFG_TMPZ84C011_PORTA_WRITE_CB(_devcb) \
	devcb = &tmpz84c011_device::set_outportsa_cb(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C011_PORTB_WRITE_CB(_devcb) \
	devcb = &tmpz84c011_device::set_outportsb_cb(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C011_PORTC_WRITE_CB(_devcb) \
	devcb = &tmpz84c011_device::set_outportsc_cb(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C011_PORTD_WRITE_CB(_devcb) \
	devcb = &tmpz84c011_device::set_outportsd_cb(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C011_PORTE_WRITE_CB(_devcb) \
	devcb = &tmpz84c011_device::set_outportse_cb(*device, DEVCB_##_devcb);


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class tmpz84c011_device : public z80_device
{
public:
	tmpz84c011_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);

	// static configuration helpers
	template<class _Object> static devcb_base &set_zc0_callback(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_zc0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_zc1_callback(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_zc1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_zc2_callback(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_zc2_cb.set_callback(object); }

	template<class _Object> static devcb_base & set_outportsa_cb(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_outportsa.set_callback(object); }
	template<class _Object> static devcb_base & set_outportsb_cb(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_outportsb.set_callback(object); }
	template<class _Object> static devcb_base & set_outportsc_cb(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_outportsc.set_callback(object); }
	template<class _Object> static devcb_base & set_outportsd_cb(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_outportsd.set_callback(object); }
	template<class _Object> static devcb_base & set_outportse_cb(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_outportse.set_callback(object); }

	template<class _Object> static devcb_base & set_inportsa_cb(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_inportsa.set_callback(object); }
	template<class _Object> static devcb_base & set_inportsb_cb(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_inportsb.set_callback(object); }
	template<class _Object> static devcb_base & set_inportsc_cb(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_inportsc.set_callback(object); }
	template<class _Object> static devcb_base & set_inportsd_cb(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_inportsd.set_callback(object); }
	template<class _Object> static devcb_base & set_inportse_cb(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_inportse.set_callback(object); }

	// CTC public interface
	DECLARE_WRITE_LINE_MEMBER( trg0 ) { m_ctc->trg0(state); }
	DECLARE_WRITE_LINE_MEMBER( trg1 ) { m_ctc->trg1(state); }
	DECLARE_WRITE_LINE_MEMBER( trg2 ) { m_ctc->trg2(state); }
	DECLARE_WRITE_LINE_MEMBER( trg3 ) { m_ctc->trg3(state); }

	/////////////////////////////////////////////////////////

	DECLARE_READ8_MEMBER( tmpz84c011_pa_r ) { return (m_inportsa() & ~m_pio_dir[0]) | (m_pio_latch[0] & m_pio_dir[0]); }
	DECLARE_READ8_MEMBER( tmpz84c011_pb_r ) { return (m_inportsb() & ~m_pio_dir[1]) | (m_pio_latch[1] & m_pio_dir[1]); }
	DECLARE_READ8_MEMBER( tmpz84c011_pc_r ) { return (m_inportsc() & ~m_pio_dir[2]) | (m_pio_latch[2] & m_pio_dir[2]); }
	DECLARE_READ8_MEMBER( tmpz84c011_pd_r ) { return (m_inportsd() & ~m_pio_dir[3]) | (m_pio_latch[3] & m_pio_dir[3]); }
	DECLARE_READ8_MEMBER( tmpz84c011_pe_r ) { return (m_inportse() & ~m_pio_dir[4]) | (m_pio_latch[4] & m_pio_dir[4]); }
	DECLARE_WRITE8_MEMBER( tmpz84c011_pa_w ) { m_pio_latch[0] = data; m_outportsa(data & m_pio_dir[0]); }
	DECLARE_WRITE8_MEMBER( tmpz84c011_pb_w ) { m_pio_latch[1] = data; m_outportsb(data & m_pio_dir[1]); }
	DECLARE_WRITE8_MEMBER( tmpz84c011_pc_w ) { m_pio_latch[2] = data; m_outportsc(data & m_pio_dir[2]); }
	DECLARE_WRITE8_MEMBER( tmpz84c011_pd_w ) { m_pio_latch[3] = data; m_outportsd(data & m_pio_dir[3]); }
	DECLARE_WRITE8_MEMBER( tmpz84c011_pe_w ) { m_pio_latch[4] = data; m_outportse(data & m_pio_dir[4]); }

	DECLARE_READ8_MEMBER( tmpz84c011_dir_pa_r ) { return m_pio_dir[0]; }
	DECLARE_READ8_MEMBER( tmpz84c011_dir_pb_r ) { return m_pio_dir[1]; }
	DECLARE_READ8_MEMBER( tmpz84c011_dir_pc_r ) { return m_pio_dir[2]; }
	DECLARE_READ8_MEMBER( tmpz84c011_dir_pd_r ) { return m_pio_dir[3]; }
	DECLARE_READ8_MEMBER( tmpz84c011_dir_pe_r ) { return m_pio_dir[4]; }
	DECLARE_WRITE8_MEMBER( tmpz84c011_dir_pa_w ) { m_pio_dir[0] = data; }
	DECLARE_WRITE8_MEMBER( tmpz84c011_dir_pb_w ) { m_pio_dir[1] = data; }
	DECLARE_WRITE8_MEMBER( tmpz84c011_dir_pc_w ) { m_pio_dir[2] = data; }
	DECLARE_WRITE8_MEMBER( tmpz84c011_dir_pd_w ) { m_pio_dir[3] = data; }
	DECLARE_WRITE8_MEMBER( tmpz84c011_dir_pe_w ) { m_pio_dir[4] = data; }

	DECLARE_WRITE_LINE_MEMBER( zc0_cb_trampoline_w ) { m_zc0_cb(state); }
	DECLARE_WRITE_LINE_MEMBER( zc1_cb_trampoline_w ) { m_zc1_cb(state); }
	DECLARE_WRITE_LINE_MEMBER( zc2_cb_trampoline_w ) { m_zc2_cb(state); }

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

	const address_space_config m_io_space_config;

	const address_space_config *memory_space_config(address_spacenum spacenum) const override
	{
		switch (spacenum)
		{
			case AS_IO: return &m_io_space_config;
			default: return z80_device::memory_space_config(spacenum);
		}
	}

private:
	// devices/pointers
	required_device<z80ctc_device> m_ctc;

	// internal state
	UINT8 m_pio_dir[5];
	UINT8 m_pio_latch[5];

	// callbacks
	devcb_write8 m_outportsa;
	devcb_write8 m_outportsb;
	devcb_write8 m_outportsc;
	devcb_write8 m_outportsd;
	devcb_write8 m_outportse;

	devcb_read8 m_inportsa;
	devcb_read8 m_inportsb;
	devcb_read8 m_inportsc;
	devcb_read8 m_inportsd;
	devcb_read8 m_inportse;

	devcb_write_line m_zc0_cb;
	devcb_write_line m_zc1_cb;
	devcb_write_line m_zc2_cb;
};


// device type definition
extern const device_type TMPZ84C011;


#endif /// __TMPZ84C011__
