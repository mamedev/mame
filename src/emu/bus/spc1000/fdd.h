// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __SPC1000_FDD_H__
#define __SPC1000_FDD_H__

#include "exp.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/upd765.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> spc1000_fdd_exp_device

class spc1000_fdd_exp_device : public device_t,
						public device_spc1000_card_interface
{
public:
	// construction/destruction
	spc1000_fdd_exp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual const rom_entry *device_rom_region() const;

public:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	virtual DECLARE_READ8_MEMBER(read);
	virtual DECLARE_WRITE8_MEMBER(write);

	DECLARE_READ8_MEMBER(tc_r);
	DECLARE_WRITE8_MEMBER(control_w);
	DECLARE_WRITE8_MEMBER(i8255_b_w);
	DECLARE_READ8_MEMBER(i8255_c_r);
	DECLARE_WRITE8_MEMBER(i8255_c_w);

private:
	// internal state
	required_device<z80_device> m_cpu;
	required_device<upd765a_device> m_fdc;
	required_device<i8255_device> m_pio;

	floppy_image_device *m_fd0;
	floppy_image_device *m_fd1;

	emu_timer *m_timer_tc;

	UINT8 m_i8255_0_pc;
	UINT8 m_i8255_1_pc;
	UINT8 m_i8255_portb;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	static const device_timer_id TIMER_TC = 0;
};


// device type definition
extern const device_type SPC1000_FDD_EXP;

#endif  /* __SPC1000_FDD_H__ */
