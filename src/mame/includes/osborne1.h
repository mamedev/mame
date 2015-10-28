// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*****************************************************************************
 *
 * includes/osborne1.h
 *
 ****************************************************************************/

#ifndef OSBORNE1_H_
#define OSBORNE1_H_

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "sound/beep.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "bus/ieee488/ieee488.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"

class osborne1_state : public driver_device
{
public:
	enum
	{
		TIMER_VIDEO,
		TIMER_SETUP
	};

	osborne1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pia0(*this, "pia_0"),
		m_pia1(*this, "pia_1"),
		m_fdc(*this, "mb8877"),
		m_beep(*this, "beeper"),
		m_ram(*this, RAM_TAG),
		m_ieee(*this, IEEE488_TAG),
		m_floppy0(*this, "mb8877:0:525ssdd"),
		m_floppy1(*this, "mb8877:1:525ssdd"),
		m_row0(*this, "ROW0"),
		m_row1(*this, "ROW1"),
		m_row2(*this, "ROW2"),
		m_row3(*this, "ROW3"),
		m_row4(*this, "ROW4"),
		m_row5(*this, "ROW5"),
		m_row6(*this, "ROW6"),
		m_row7(*this, "ROW7"),
		m_cnf(*this, "CNF"),
		m_bank1(*this, "bank1"),
		m_bank2(*this, "bank2"),
		m_bank3(*this, "bank3"),
		m_region_maincpu(*this, "maincpu") { }

	virtual void video_start();

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	bitmap_ind16 m_bitmap;

	required_device<cpu_device> m_maincpu;
	required_device<pia6821_device> m_pia0;
	required_device<pia6821_device> m_pia1;
	required_device<mb8877_t> m_fdc;
	required_device<beep_device> m_beep;
	required_device<ram_device> m_ram;
	required_device<ieee488_device> m_ieee;
	required_device<floppy_image_device> m_floppy0;
	required_device<floppy_image_device> m_floppy1;

	DECLARE_WRITE8_MEMBER(osborne1_0000_w);
	DECLARE_WRITE8_MEMBER(osborne1_1000_w);
	DECLARE_READ8_MEMBER(osborne1_2000_r);
	DECLARE_WRITE8_MEMBER(osborne1_2000_w);
	DECLARE_WRITE8_MEMBER(osborne1_videoram_w);
	DECLARE_WRITE8_MEMBER(osborne1_bankswitch_w);
	DECLARE_WRITE_LINE_MEMBER(ieee_pia_irq_a_func);
	DECLARE_READ8_MEMBER(ieee_pia_pb_r);
	DECLARE_WRITE8_MEMBER(ieee_pia_pb_w);
	DECLARE_WRITE_LINE_MEMBER(video_pia_out_cb2_dummy);
	DECLARE_WRITE8_MEMBER(video_pia_port_a_w);
	DECLARE_WRITE8_MEMBER(video_pia_port_b_w);
	DECLARE_WRITE_LINE_MEMBER(video_pia_irq_a_func);
	DECLARE_DIRECT_UPDATE_MEMBER(osborne1_opbase);

	bool m_bank2_enabled;
	UINT8   m_bit_9;
	/* IRQ states */
	bool m_pia_0_irq_state;
	bool m_pia_1_irq_state;
	/* video related */
	UINT8   m_screen_pac;
	UINT8   m_resolution;
	UINT8   m_hc_left;
	UINT8   m_new_start_x;
	UINT8   m_new_start_y;
	emu_timer *m_video_timer;
	UINT8   *m_p_chargen;
	/* bankswitch setting */
	UINT8   m_bankswitch;
	bool m_in_irq_handler;
	bool m_beep_state;
	DECLARE_DRIVER_INIT(osborne1);
	virtual void machine_reset();
	TIMER_CALLBACK_MEMBER(osborne1_video_callback);
	TIMER_CALLBACK_MEMBER(setup_osborne1);

protected:
	required_ioport m_row0;
	required_ioport m_row1;
	required_ioport m_row2;
	required_ioport m_row3;
	required_ioport m_row4;
	required_ioport m_row5;
	required_ioport m_row6;
	required_ioport m_row7;
	required_ioport m_cnf;
	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	required_memory_bank m_bank3;
	required_memory_region m_region_maincpu;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
};


// ======================> osborne1_daisy_device

class osborne1_daisy_device :   public device_t,
						public device_z80daisy_interface
{
public:
	// construction/destruction
	osborne1_daisy_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

private:
	virtual void device_start();
	// z80daisy_interface overrides
	virtual int z80daisy_irq_state();
	virtual int z80daisy_irq_ack();
	virtual void z80daisy_irq_reti();
};

extern const device_type OSBORNE1_DAISY;

#endif /* OSBORNE1_H_ */
