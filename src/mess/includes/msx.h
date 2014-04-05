/*****************************************************************************
 *
 * includes/msx.h
 *
 ****************************************************************************/

#ifndef __MSX_H__
#define __MSX_H__

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/rp5c01.h"
#include "machine/wd17xx.h"
#include "machine/buffer.h"
#include "bus/centronics/ctronics.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/wave.h"
#include "sound/k051649.h"
#include "sound/2413intf.h"
#include "video/v9938.h"
#include "video/tms9928a.h"
#include "imagedev/flopdrv.h"
#include "imagedev/cartslot.h"
#include "imagedev/cassette.h"
#include "formats/basicdsk.h"
#include "formats/fmsx_cas.h"
#include "formats/msx_dsk.h"
//#include "osdepend.h"
#include "hashfile.h"
#include "includes/msx_slot.h"

#define MSX_MAX_CARTS   (2)

#define TC8521_TAG  "rtc"

class msx_state : public driver_device
{
public:
	msx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_v9938(*this, "v9938"),
		m_v9958(*this, "v9958"),
		m_cassette(*this, "cassette"),
		m_ay8910(*this, "ay8910"),
		m_ym(*this, "ym2413"),
		m_k051649(*this, "k051649"),
		m_dac(*this, "dac"),
		m_rtc(*this, TC8521_TAG),
		m_wd179x(*this, "wd179x"),
		m_bank1(*this, "bank1"),
		m_bank2(*this, "bank2"),
		m_bank3(*this, "bank3"),
		m_bank4(*this, "bank4"),
		m_bank5(*this, "bank5"),
		m_bank6(*this, "bank6"),
		m_bank7(*this, "bank7"),
		m_bank8(*this, "bank8"),
		m_bank9(*this, "bank9"),
		m_bank10(*this, "bank10"),
		m_bank11(*this, "bank11"),
		m_region_maincpu(*this, "maincpu"),
		m_io_joy0(*this, "JOY0"),
		m_io_joy1(*this, "JOY1"),
		m_io_dsw(*this, "DSW"),
		m_io_mouse0(*this, "MOUSE0"),
		m_io_mouse1(*this, "MOUSE1"),
		m_io_key0(*this, "KEY0"),
		m_io_key1(*this, "KEY1"),
		m_io_key2(*this, "KEY2"),
		m_io_key3(*this, "KEY3"),
		m_io_key4(*this, "KEY4"),
		m_io_key5(*this, "KEY5") { }

	DECLARE_WRITE8_MEMBER(msx_page0_w);
	DECLARE_WRITE8_MEMBER(msx_page0_1_w);
	DECLARE_WRITE8_MEMBER(msx_page1_w);
	DECLARE_WRITE8_MEMBER(msx_page1_1_w);
	DECLARE_WRITE8_MEMBER(msx_page1_2_w);
	DECLARE_WRITE8_MEMBER(msx_page2_w);
	DECLARE_WRITE8_MEMBER(msx_page2_1_w);
	DECLARE_WRITE8_MEMBER(msx_page2_2_w);
	DECLARE_WRITE8_MEMBER(msx_page2_3_w);
	DECLARE_WRITE8_MEMBER(msx_page3_w);
	DECLARE_WRITE8_MEMBER(msx_page3_1_w);
	DECLARE_WRITE8_MEMBER(msx_sec_slot_w);
	DECLARE_READ8_MEMBER(msx_sec_slot_r);
	DECLARE_WRITE8_MEMBER(msx_ram_mapper_w);
	DECLARE_READ8_MEMBER(msx_ram_mapper_r);
	DECLARE_READ8_MEMBER(msx_kanji_r);
	DECLARE_WRITE8_MEMBER(msx_kanji_w);
	DECLARE_WRITE8_MEMBER(msx_90in1_w);
	DECLARE_WRITE8_MEMBER(msx_ppi_port_a_w);
	DECLARE_WRITE8_MEMBER(msx_ppi_port_c_w);
	DECLARE_READ8_MEMBER(msx_ppi_port_b_r);
	DECLARE_WRITE8_MEMBER(msx_fmpac_w);
	DECLARE_READ8_MEMBER(msx_rtc_reg_r);
	DECLARE_WRITE8_MEMBER(msx_rtc_reg_w);
	DECLARE_WRITE8_MEMBER(msx_rtc_latch_w);
	DECLARE_WRITE_LINE_MEMBER(msx_wd179x_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(msx_wd179x_drq_w);

	/* PSG */
	int m_psg_b;
	int m_opll_active;
	/* mouse */
	UINT16 m_mouse[2];
	int m_mouse_stat[2];
	/* rtc */
	int m_rtc_latch;
	/* disk */
	UINT8 m_dsk_stat;
	/* kanji */
	UINT8 *m_kanji_mem;
	int m_kanji_latch;
	/* memory */
	const msx_slot_layout *m_layout;
	slot_state *m_cart_state[MSX_MAX_CARTS];
	slot_state *m_state[4];
	const msx_slot *m_slot[4];
	UINT8 *m_ram_pages[4];
	UINT8 *m_empty, m_ram_mapper[4];
	UINT8 m_ramio_set_bits;
	slot_state *m_all_state[4][4][4];
	int m_slot_expanded[4];
	UINT8 m_primary_slot;
	UINT8 m_secondary_slot[4];
	UINT8 m_superloderunner_bank;
	UINT8 m_korean90in1_bank;
	UINT8 *m_top_page;
	int m_port_c_old;
	int keylatch;
	void msx_memory_map_all ();
	void msx_memory_map_page (UINT8 page);
	void msx_ch_reset_core ();
	void msx_memory_reset ();

	required_device<z80_device> m_maincpu;
	optional_device<v9938_device> m_v9938;
	optional_device<v9958_device> m_v9958;
	required_device<cassette_image_device> m_cassette;
	required_device<ay8910_device> m_ay8910;
	required_device<ym2413_device> m_ym;
	optional_device<k051649_device> m_k051649;
	required_device<dac_device> m_dac;
	optional_device<rp5c01_device> m_rtc;
	optional_device<fd1793_device> m_wd179x;
	DECLARE_READ8_MEMBER(msx_psg_port_a_r);
	DECLARE_READ8_MEMBER(msx_psg_port_b_r);
	DECLARE_WRITE8_MEMBER(msx_psg_port_a_w);
	DECLARE_WRITE8_MEMBER(msx_psg_port_b_w);
	DECLARE_DRIVER_INIT(msx);
	DECLARE_MACHINE_START(msx);
	DECLARE_MACHINE_RESET(msx);
	DECLARE_MACHINE_START(msx2);
	DECLARE_MACHINE_RESET(msx2);
	INTERRUPT_GEN_MEMBER(msx_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(msx2_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(msx2p_interrupt);
	DECLARE_WRITE8_MEMBER(msx_ay8910_w);
	void msx_memory_init();
	void msx_memory_set_carts();

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( msx_cart );
	DECLARE_DEVICE_IMAGE_UNLOAD_MEMBER( msx_cart );
	DECLARE_WRITE_LINE_MEMBER(msx_vdp_interrupt);

	// from msx_slot
	DECLARE_READ8_MEMBER(konami_scc_bank5);
	DECLARE_READ8_MEMBER(msx_diskrom_page1_r);
	DECLARE_READ8_MEMBER(msx_diskrom_page2_r);
	DECLARE_READ8_MEMBER(msx_diskrom2_page1_r);
	DECLARE_READ8_MEMBER(msx_diskrom2_page2_r);
	DECLARE_READ8_MEMBER(soundcartridge_scc);
	DECLARE_READ8_MEMBER(soundcartridge_sccp);

	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	required_memory_bank m_bank3;
	required_memory_bank m_bank4;
	required_memory_bank m_bank5;
	required_memory_bank m_bank6;
	required_memory_bank m_bank7;
	required_memory_bank m_bank8;
	required_memory_bank m_bank9;
	required_memory_bank m_bank10;
	required_memory_bank m_bank11;

protected:
	required_memory_region m_region_maincpu;
	required_ioport m_io_joy0;
	required_ioport m_io_joy1;
	required_ioport m_io_dsw;
	required_ioport m_io_mouse0;
	required_ioport m_io_mouse1;
	required_ioport m_io_key0;
	required_ioport m_io_key1;
	required_ioport m_io_key2;
	required_ioport m_io_key3;
	required_ioport m_io_key4;
	required_ioport m_io_key5;
};


/*----------- defined in machine/msx.c -----------*/

extern const i8255_interface msx_ppi8255_interface;
extern const wd17xx_interface msx_wd17xx_interface;
/* start/stop functions */

/* I/O functions */

#endif /* __MSX_H__ */
