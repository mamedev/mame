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
#include "machine/ieee488.h"
#include "machine/ram.h"
#include "machine/wd17xx.h"
#include "imagedev/flopdrv.h"
#include "formats/basicdsk.h"

class osborne1_state : public driver_device
{
public:
	osborne1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_pia0(*this, "pia_0"),
	m_pia1(*this, "pia_1"),
	m_fdc(*this, "mb8877"),
	m_beep(*this, BEEPER_TAG),
	m_ram(*this, RAM_TAG),
	m_ieee(*this, IEEE488_TAG)
	{ }

	virtual void video_start();

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	bitmap_ind16 m_bitmap;

	required_device<cpu_device> m_maincpu;
	required_device<pia6821_device> m_pia0;
	required_device<pia6821_device> m_pia1;
	required_device<device_t> m_fdc;
	required_device<device_t> m_beep;
	required_device<ram_device> m_ram;
	required_device<ieee488_device> m_ieee;

	DECLARE_WRITE8_MEMBER(osborne1_0000_w);
	DECLARE_WRITE8_MEMBER(osborne1_1000_w);
	DECLARE_READ8_MEMBER(osborne1_2000_r);
	DECLARE_WRITE8_MEMBER(osborne1_2000_w);
	DECLARE_WRITE8_MEMBER(osborne1_3000_w);
	DECLARE_WRITE8_MEMBER(osborne1_videoram_w);
	DECLARE_WRITE8_MEMBER(osborne1_bankswitch_w);
	DECLARE_WRITE_LINE_MEMBER(ieee_pia_irq_a_func);
	DECLARE_READ8_MEMBER(ieee_pia_pb_r);
	DECLARE_WRITE8_MEMBER(ieee_pia_pb_w);
	DECLARE_WRITE8_MEMBER(video_pia_out_cb2_dummy);
	DECLARE_WRITE8_MEMBER(video_pia_port_a_w);
	DECLARE_WRITE8_MEMBER(video_pia_port_b_w);
	DECLARE_WRITE_LINE_MEMBER(video_pia_irq_a_func);
	DECLARE_DIRECT_UPDATE_MEMBER(osborne1_opbase);

	bool m_bank2_enabled;
	bool m_bank3_enabled;
	UINT8	*m_bank4_ptr;
	UINT8	*m_empty_4K;
	/* IRQ states */
	bool m_pia_0_irq_state;
	bool m_pia_1_irq_state;
	/* video related */
	UINT8	m_new_start_x;
	UINT8	m_new_start_y;
	emu_timer *m_video_timer;
	UINT8	*m_p_chargen;
	/* bankswitch setting */
	UINT8	m_bankswitch;
	bool m_in_irq_handler;
	bool m_beep_state;
	DECLARE_DRIVER_INIT(osborne1);
	virtual void machine_reset();
	virtual void palette_init();
};


/*----------- defined in machine/osborne1.c -----------*/

extern const pia6821_interface osborne1_ieee_pia_config;
extern const pia6821_interface osborne1_video_pia_config;




// ======================> osborne1_daisy_device

class osborne1_daisy_device :	public device_t,
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
