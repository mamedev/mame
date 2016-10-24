// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/pk8020.h
 *
 ****************************************************************************/

#ifndef PK8020_H_
#define PK8020_H_

#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "machine/i8251.h"
#include "machine/wd_fdc.h"
#include "imagedev/cassette.h"
#include "sound/speaker.h"
#include "sound/wave.h"
#include "machine/ram.h"


class pk8020_state : public driver_device
{
public:
	pk8020_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ppi8255_1(*this, "ppi8255_1"),
		m_ppi8255_2(*this, "ppi8255_2"),
		m_ppi8255_3(*this, "ppi8255_3"),
		m_rs232(*this, "rs232"),
		m_lan(*this, "lan"),
		m_ram(*this, RAM_TAG),
		m_wd1793(*this, "wd1793"),
		m_floppy0(*this, "wd1793:0"),
		m_floppy1(*this, "wd1793:1"),
		m_floppy2(*this, "wd1793:2"),
		m_floppy3(*this, "wd1793:3"),
		m_pit8253(*this, "pit8253"),
		m_pic8259(*this, "pic8259"),
		m_speaker(*this, "speaker"),
		m_region_maincpu(*this, "maincpu"),
		m_region_gfx1(*this, "gfx1"),
		m_palette(*this, "palette")  { }

	DECLARE_FLOPPY_FORMATS(floppy_formats);

	uint8_t m_color;
	uint8_t m_video_page;
	uint8_t m_wide;
	uint8_t m_font;
	uint8_t m_attr;
	uint8_t m_text_attr;
	uint8_t m_takt;
	uint8_t m_video_page_access;
	uint8_t m_portc_data;
	uint8_t m_sound_gate;
	uint8_t m_sound_level;
	uint8_t keyboard_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t sysreg_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sysreg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t text_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void text_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t gzu_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void gzu_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t devices_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void devices_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_pk8020(palette_device &palette);
	uint32_t screen_update_pk8020(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void pk8020_interrupt(device_t &device);
	uint8_t pk8020_porta_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pk8020_portc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pk8020_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pk8020_portc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pk8020_2_portc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pk8020_pit_out0(int state);
	void pk8020_pit_out1(int state);

protected:
	required_device<cpu_device> m_maincpu;
	required_device<i8255_device> m_ppi8255_1;
	required_device<i8255_device> m_ppi8255_2;
	required_device<i8255_device> m_ppi8255_3;
	required_device<i8251_device> m_rs232;
	required_device<i8251_device> m_lan;
	required_device<ram_device> m_ram;
	required_device<fd1793_t> m_wd1793;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;
	required_device<pit8253_device> m_pit8253;
	required_device<pic8259_device> m_pic8259;
	required_device<speaker_sound_device> m_speaker;
	required_memory_region m_region_maincpu;
	required_memory_region m_region_gfx1;
	ioport_port *m_io_port[16];
	required_device<palette_device> m_palette;
	void pk8020_set_bank(uint8_t data);
};

#endif /* pk8020_H_ */
