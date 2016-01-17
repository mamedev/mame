// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/vector06.h
 *
 ****************************************************************************/

#ifndef VECTOR06_H_
#define VECTOR06_H_

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/i8085/i8085.h"
#include "sound/wave.h"
#include "machine/i8255.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "imagedev/cassette.h"
#include "imagedev/flopdrv.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"


class vector06_state : public driver_device
{
public:
	vector06_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cassette(*this, "cassette"),
		m_cart(*this, "cartslot"),
		m_fdc(*this, "wd1793"),
		m_floppy0(*this, "wd1793:0"),
		m_floppy1(*this, "wd1793:1"),
		m_ppi(*this, "ppi8255"),
		m_ppi2(*this, "ppi8255_2"),
		m_ram(*this, RAM_TAG),
		m_palette(*this, "palette"),
		m_bank1(*this, "bank1"),
		m_bank2(*this, "bank2"),
		m_bank3(*this, "bank3"),
		m_bank4(*this, "bank4"),
		m_region_maincpu(*this, "maincpu"),
		m_line(*this, "LINE"),
		m_reset(*this, "RESET")
	{ }

	DECLARE_FLOPPY_FORMATS(floppy_formats);

	DECLARE_READ8_MEMBER(vector06_8255_portb_r);
	DECLARE_READ8_MEMBER(vector06_8255_portc_r);
	DECLARE_WRITE8_MEMBER(vector06_8255_porta_w);
	DECLARE_WRITE8_MEMBER(vector06_8255_portb_w);
	DECLARE_WRITE8_MEMBER(vector06_color_set);
	DECLARE_READ8_MEMBER(vector06_romdisk_portb_r);
	DECLARE_WRITE8_MEMBER(vector06_romdisk_porta_w);
	DECLARE_WRITE8_MEMBER(vector06_romdisk_portc_w);
	DECLARE_READ8_MEMBER(vector06_8255_1_r);
	DECLARE_WRITE8_MEMBER(vector06_8255_1_w);
	DECLARE_READ8_MEMBER(vector06_8255_2_r);
	DECLARE_WRITE8_MEMBER(vector06_8255_2_w);
	DECLARE_WRITE8_MEMBER(vector06_disc_w);
	void vector06_set_video_mode(int width);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(vector06);
	UINT32 screen_update_vector06(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vector06_interrupt);
	TIMER_CALLBACK_MEMBER(reset_check_callback);
	IRQ_CALLBACK_MEMBER(vector06_irq_callback);

private:
	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_device<generic_slot_device> m_cart;
	required_device<fd1793_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<i8255_device> m_ppi;
	required_device<i8255_device> m_ppi2;
	required_device<ram_device> m_ram;
	required_device<palette_device> m_palette;
	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	required_memory_bank m_bank3;
	required_memory_bank m_bank4;
	required_memory_region m_region_maincpu;
	required_ioport_array<9> m_line;
	required_ioport m_reset;

	UINT8 m_keyboard_mask;
	UINT8 m_color_index;
	UINT8 m_video_mode;
	UINT8 m_romdisk_msb;
	UINT8 m_romdisk_lsb;
	UINT8 m_vblank_state;

};

#endif /* VECTOR06_H_ */
