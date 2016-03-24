// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/b2m.h
 *
 ****************************************************************************/

#ifndef B2M_H_
#define B2M_H_

#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "machine/wd_fdc.h"
#include "sound/speaker.h"
#include "sound/wave.h"
#include "machine/ram.h"

class b2m_state : public driver_device
{
public:
	b2m_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker"),
		m_pit(*this, "pit8253"),
		m_ram(*this, RAM_TAG),
		m_palette(*this, "palette")  { }

	UINT8 m_b2m_8255_porta;
	UINT8 m_b2m_video_scroll;
	UINT8 m_b2m_8255_portc;

	UINT8 m_b2m_video_page;
	UINT8 m_b2m_drive;
	UINT8 m_b2m_side;

	UINT8 m_b2m_romdisk_lsb;
	UINT8 m_b2m_romdisk_msb;

	UINT8 m_b2m_color[4];
	UINT8 m_b2m_localmachine;
	UINT8 m_vblank_state;
	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<pit8253_device> m_pit;
	required_device<ram_device> m_ram;
	required_device<palette_device> m_palette;

	/* devices */
	fd1793_t *m_fdc;
	pic8259_device *m_pic;
	DECLARE_READ8_MEMBER(b2m_keyboard_r);
	DECLARE_WRITE8_MEMBER(b2m_palette_w);
	DECLARE_READ8_MEMBER(b2m_palette_r);
	DECLARE_WRITE8_MEMBER(b2m_localmachine_w);
	DECLARE_READ8_MEMBER(b2m_localmachine_r);
	DECLARE_DRIVER_INIT(b2m);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(b2m);
	UINT32 screen_update_b2m(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(b2m_vblank_interrupt);
	DECLARE_WRITE_LINE_MEMBER(bm2_pit_out1);
	DECLARE_WRITE8_MEMBER(b2m_8255_porta_w);
	DECLARE_WRITE8_MEMBER(b2m_8255_portb_w);
	DECLARE_WRITE8_MEMBER(b2m_8255_portc_w);
	DECLARE_READ8_MEMBER(b2m_8255_portb_r);
	DECLARE_WRITE8_MEMBER(b2m_ext_8255_portc_w);
	DECLARE_READ8_MEMBER(b2m_romdisk_porta_r);
	DECLARE_WRITE8_MEMBER(b2m_romdisk_portb_w);
	DECLARE_WRITE8_MEMBER(b2m_romdisk_portc_w);
	DECLARE_WRITE_LINE_MEMBER(b2m_fdc_drq);
	DECLARE_FLOPPY_FORMATS( b2m_floppy_formats );
	void b2m_postload();
	void b2m_set_bank(int bank);
};

#endif
