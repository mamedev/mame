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

	uint8_t m_b2m_8255_porta;
	uint8_t m_b2m_video_scroll;
	uint8_t m_b2m_8255_portc;

	uint8_t m_b2m_video_page;
	uint8_t m_b2m_drive;
	uint8_t m_b2m_side;

	uint8_t m_b2m_romdisk_lsb;
	uint8_t m_b2m_romdisk_msb;

	uint8_t m_b2m_color[4];
	uint8_t m_b2m_localmachine;
	uint8_t m_vblank_state;
	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<pit8253_device> m_pit;
	required_device<ram_device> m_ram;
	required_device<palette_device> m_palette;

	/* devices */
	fd1793_t *m_fdc;
	pic8259_device *m_pic;
	uint8_t b2m_keyboard_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void b2m_palette_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t b2m_palette_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void b2m_localmachine_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t b2m_localmachine_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void init_b2m();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_b2m(palette_device &palette);
	uint32_t screen_update_b2m(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void b2m_vblank_interrupt(device_t &device);
	void bm2_pit_out1(int state);
	void b2m_8255_porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void b2m_8255_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void b2m_8255_portc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t b2m_8255_portb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void b2m_ext_8255_portc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t b2m_romdisk_porta_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void b2m_romdisk_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void b2m_romdisk_portc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void b2m_fdc_drq(int state);
	DECLARE_FLOPPY_FORMATS( b2m_floppy_formats );
	void b2m_postload();
	void b2m_set_bank(int bank);
};

#endif
