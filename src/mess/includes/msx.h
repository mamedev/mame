// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
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
#include "machine/buffer.h"
#include "bus/centronics/ctronics.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/wave.h"
#include "sound/2413intf.h"
#include "video/v9938.h"
#include "video/tms9928a.h"
#include "imagedev/flopdrv.h"
#include "imagedev/cassette.h"
#include "formats/fmsx_cas.h"
#include "formats/msx_dsk.h"
#include "hashfile.h"
#include "machine/wd_fdc.h"
#include "imagedev/floppy.h"
#include "bus/msx_slot/slot.h"
#include "bus/msx_slot/rom.h"
#include "bus/msx_slot/ram.h"
#include "bus/msx_slot/cartridge.h"
#include "bus/msx_slot/ram_mm.h"
#include "bus/msx_slot/disk.h"
#include "bus/msx_slot/music.h"
#include "bus/msx_slot/bunsetsu.h"
#include "bus/msx_slot/fs4600.h"
#include "bus/msx_slot/panasonic08.h"
#include "bus/msx_slot/sony08.h"


#define TC8521_TAG  "rtc"

#define MCFG_MSX_LAYOUT_ROM(_tag, _prim, _sec, _page, _numpages, _region, _offset) \
	MCFG_MSX_SLOT_ROM_ADD(_tag, _page, _numpages, _region, _offset) \
	msx_state::install_slot_pages(*owner, _prim, _sec, _page, _numpages, device);

#define MCFG_MSX_LAYOUT_RAM(_tag, _prim, _sec, _page, _numpages) \
	MCFG_MSX_SLOT_RAM_ADD(_tag, _page, _numpages) \
	msx_state::install_slot_pages(*owner, _prim, _sec, _page, _numpages, device);

#define MCFG_MSX_LAYOUT_CARTRIDGE(_tag, _prim, _sec) \
	MCFG_MSX_SLOT_CARTRIDGE_ADD(_tag, WRITELINE(msx_state, msx_irq_source1)) \
	msx_state::install_slot_pages(*owner, _prim, _sec, 0, 4, device);

#define MCFG_MSX_LAYOUT_YAMAHA_EXPANSION(_tag, _prim, _sec, _default) \
	MCFG_MSX_SLOT_YAMAHA_EXPANSION_ADD(_tag, WRITELINE(msx_state, msx_irq_source2), _default) \
	msx_state::install_slot_pages(*owner, _prim, _sec, 0, 4, device);

#define MCFG_MSX_LAYOUT_RAM_MM(_tag, _prim, _sec, _total_size) \
	MCFG_MSX_SLOT_RAM_MM_ADD(_tag, _total_size) \
	msx_state::install_slot_pages(*owner, _prim, _sec, 0, 4, device);

#define MCFG_MSX_RAMIO_SET_BITS(_ramio_set_bits) \
	MCFG_MSX_SLOT_RAMM_SET_RAMIO_BITS(_ramio_set_bits)

#define MCFG_MSX_LAYOUT_DISK1(_tag, _prim, _sec, _page, _numpages, _region, _offset) \
	MCFG_MSX_SLOT_DISK1_ADD(_tag, _page, _numpages, _region, _offset, "fdc", "fdc:0", "fdc:1") \
	msx_state::install_slot_pages(*owner, _prim, _sec, _page, _numpages + 1, device);   /* Memory mapped FDC registers are also accessible through page 2 */

#define MCFG_MSX_LAYOUT_DISK2(_tag, _prim, _sec, _page, _numpages, _region, _offset) \
	MCFG_MSX_SLOT_DISK2_ADD(_tag, _page, _numpages, _region, _offset, "fdc", "fdc:0", "fdc:1") \
	msx_state::install_slot_pages(*owner, _prim, _sec, _page, _numpages + 1, device);   /* Memory mapped FDC registers are also accessible through page 2 */

#define MCFG_MSX_LAYOUT_DISK3(_tag, _prim, _sec, _page, _numpages, _region, _offset) \
	MCFG_MSX_SLOT_DISK3_ADD(_tag, _page, _numpages, _region, _offset, "fdc", "fdc:0", "fdc:1") \
	msx_state::install_slot_pages(*owner, _prim, _sec, _page, _numpages, device);

#define MCFG_MSX_LAYOUT_DISK4(_tag, _prim, _sec, _page, _numpages, _region, _offset) \
	MCFG_MSX_SLOT_DISK4_ADD(_tag, _page, _numpages, _region, _offset, "fdc", "fdc:0", "fdc:1") \
	msx_state::install_slot_pages(*owner, _prim, _sec, _page, _numpages, device);

#define MCFG_MSX_LAYOUT_DISK5(_tag, _prim, _sec, _page, _numpages, _region, _offset) \
	MCFG_MSX_SLOT_DISK5_ADD(_tag, _page, _numpages, _region, _offset, "fdc", "fdc:0", "fdc:1", "fdc:2", "fdc:3") \
	msx_state::install_slot_pages(*owner, _prim, _sec, _page, _numpages, device);

#define MCFG_MSX_LAYOUT_DISK6(_tag, _prim, _sec, _page, _numpages, _region, _offset) \
	MCFG_MSX_SLOT_DISK6_ADD(_tag, _page, _numpages, _region, _offset, "fdc", "fdc:0", "fdc:1") \
	msx_state::install_slot_pages(*owner, _prim, _sec, _page, _numpages, device);

#define MCFG_MSX_LAYOUT_MUSIC(_tag, _prim, _sec, _page, _numpages, _region, _offset) \
	MCFG_MSX_SLOT_MUSIC_ADD(_tag, _page, _numpages, _region, _offset, "ym2413" ) \
	msx_state::install_slot_pages(*owner, _prim, _sec, _page, _numpages, device);

#define MCFG_MSX_LAYOUT_BUNSETSU(_tag, _prim, _sec, _page, _numpages, _region, _offset, _bunsetsu_tag) \
	MCFG_MSX_SLOT_BUNSETSU_ADD(_tag, _page, _numpages, _region, _offset, _bunsetsu_tag) \
	msx_state::install_slot_pages(*owner, _prim, _sec, _page, _numpages, device);

#define MCFG_MSX_LAYOUT_FS4600(_tag, _prim, _sec, _page, _numpages, _region, _offset) \
	MCFG_MSX_SLOT_FS4600_ADD(_tag, _page, _numpages, _region, _offset) \
	msx_state::install_slot_pages(*owner, _prim, _sec, _page, _numpages, device);

#define MCFG_MSX_LAYOUT_PANASONIC08(_tag, _prim, _sec, _page, _numpages, _region, _offset) \
	MCFG_MSX_SLOT_PANASONIC08_ADD(_tag, _page, _numpages, _region, _offset) \
	msx_state::install_slot_pages(*owner, _prim, _sec, _page, _numpages, device);

#define MCFG_MSX_LAYOUT_SONY08(_tag, _prim, _sec, _page, _numpages, _region, _offset) \
	MCFG_MSX_SLOT_SONY08_ADD(_tag, _page, _numpages, _region, _offset) \
	msx_state::install_slot_pages(*owner, _prim, _sec, _page, _numpages, device);


class msx_state : public driver_device
{
public:
	msx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_v9938(*this, "v9938")
		, m_v9958(*this, "v9958")
		, m_cassette(*this, "cassette")
		, m_ay8910(*this, "ay8910")
		, m_dac(*this, "dac")
		, m_rtc(*this, TC8521_TAG)
		, m_switched_device_as_config("switched_device", ENDIANNESS_LITTLE, 8, 16, 0, address_map_delegate(FUNC(msx_state::switched_device_map), this))
		, m_region_maincpu(*this, "maincpu")
		, m_region_kanji(*this, "kanji")
		, m_io_joy0(*this, "JOY0")
		, m_io_joy1(*this, "JOY1")
		, m_io_dsw(*this, "DSW")
		, m_io_mouse0(*this, "MOUSE0")
		, m_io_mouse1(*this, "MOUSE1")
		, m_io_key0(*this, "KEY0")
		, m_io_key1(*this, "KEY1")
		, m_io_key2(*this, "KEY2")
		, m_io_key3(*this, "KEY3")
		, m_io_key4(*this, "KEY4")
		, m_io_key5(*this, "KEY5")
		, m_psg_b(0)
		, m_rtc_latch(0)
		, m_kanji_latch(0)
		, m_primary_slot(0)
		, m_port_c_old(0)
		, m_keylatch(0)
		, m_current_switched_device(0)
	{
		for (int prim = 0; prim < 4; prim++ )
		{
			m_slot_expanded[prim] = false;
			m_secondary_slot[prim] = 0;
			for (int sec = 0; sec < 4; sec++ )
			{
				for (int page = 0; page < 4; page++ )
				{
					m_all_slots[prim][sec][page] = NULL;
				}
			}
		}
		m_mouse[0] = m_mouse[1] = 0;
		m_mouse_stat[0] = m_mouse_stat[1] = 0;
		for (int i = 0; i < ARRAY_LENGTH(m_irq_state); i++)
		{
			m_irq_state[i] = CLEAR_LINE;
		}
	}

	// static configuration helpers
	static void install_slot_pages(device_t &owner, UINT8 prim, UINT8 sec, UINT8 page, UINT8 numpages, device_t *device);

	virtual void driver_start();
	virtual void machine_start();
	virtual void machine_reset();

	DECLARE_ADDRESS_MAP(switched_device_map, 8);
	DECLARE_WRITE8_MEMBER(msx_sec_slot_w);
	DECLARE_READ8_MEMBER(msx_sec_slot_r);
	DECLARE_READ8_MEMBER(msx_kanji_r);
	DECLARE_WRITE8_MEMBER(msx_kanji_w);
	DECLARE_WRITE8_MEMBER(msx_ppi_port_a_w);
	DECLARE_WRITE8_MEMBER(msx_ppi_port_c_w);
	DECLARE_READ8_MEMBER(msx_ppi_port_b_r);
	DECLARE_READ8_MEMBER(msx_rtc_reg_r);
	DECLARE_WRITE8_MEMBER(msx_rtc_reg_w);
	DECLARE_WRITE8_MEMBER(msx_rtc_latch_w);
	DECLARE_READ8_MEMBER(msx_mem_read);
	DECLARE_WRITE8_MEMBER(msx_mem_write);
	DECLARE_READ8_MEMBER(msx_switched_r);
	DECLARE_WRITE8_MEMBER(msx_switched_w);
	DECLARE_WRITE_LINE_MEMBER(turbo_w);

	void msx_memory_map_all();
	void msx_memory_map_page(UINT8 page);
	void msx_memory_reset();

	DECLARE_FLOPPY_FORMATS(floppy_formats);

	DECLARE_READ8_MEMBER(msx_psg_port_a_r);
	DECLARE_READ8_MEMBER(msx_psg_port_b_r);
	DECLARE_WRITE8_MEMBER(msx_psg_port_a_w);
	DECLARE_WRITE8_MEMBER(msx_psg_port_b_w);
	INTERRUPT_GEN_MEMBER(msx_interrupt);
	DECLARE_WRITE8_MEMBER(msx_ay8910_w);
	void msx_memory_init();
	void post_load();

	DECLARE_WRITE_LINE_MEMBER(msx_irq_source0) { msx_irq_source(0, state); }  // usually tms9918/v9938/v9958
	DECLARE_WRITE_LINE_MEMBER(msx_irq_source1) { msx_irq_source(1, state); }  // usually first cartridge slot
	DECLARE_WRITE_LINE_MEMBER(msx_irq_source2) { msx_irq_source(2, state); }  // usually second cartridge slot
	DECLARE_WRITE_LINE_MEMBER(msx_irq_source3) { msx_irq_source(3, state); }  // sometimes expansion slot

protected:
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const { return (spacenum == 0) ? &m_switched_device_as_config : NULL; }

private:
	required_device<z80_device> m_maincpu;
	optional_device<v9938_device> m_v9938;
	optional_device<v9958_device> m_v9958;
	required_device<cassette_image_device> m_cassette;
	required_device<ay8910_device> m_ay8910;
	required_device<dac_device> m_dac;
	optional_device<rp5c01_device> m_rtc;
	address_space_config m_switched_device_as_config;
	required_memory_region m_region_maincpu;
	optional_memory_region m_region_kanji;
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

	/* PSG */
	int m_psg_b;
	/* mouse */
	UINT16 m_mouse[2];
	int m_mouse_stat[2];
	/* rtc */
	int m_rtc_latch;
	/* kanji */
	int m_kanji_latch;
	/* memory */
	msx_internal_slot_interface m_empty_slot;
	msx_internal_slot_interface *m_all_slots[4][4][4];
	msx_internal_slot_interface *m_current_page[4];
	bool m_slot_expanded[4];
	UINT8 m_primary_slot;
	UINT8 m_secondary_slot[4];
	int m_port_c_old;
	int m_keylatch;
	UINT8 m_current_switched_device;

	int m_irq_state[4];

	void msx_irq_source(int source, int level);
	void check_irq();
};


#endif /* __MSX_H__ */
