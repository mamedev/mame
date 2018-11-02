// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for Visual 102 display terminal.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/eeprompar.h"
#include "machine/input_merger.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/z80sio.h"
//#include "video/crt9006.h"
#include "video/crt9007.h"
//#include "video/crt9021.h"
#include "screen.h"

class v102_state : public driver_device
{
public:
	v102_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
	{ }

	void v102(machine_config &config);

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map);
	void kbd_map(address_map &map);
	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
};


u32 v102_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void v102_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).rom().region("maincpu", 0);
	map(0x8000, 0x8fff).ram();
	map(0xa000, 0xafff).ram();
	map(0xb800, 0xb9ff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write));
}

void v102_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x3f).rw("vpac", FUNC(crt9007_device::read), FUNC(crt9007_device::write));
	map(0x40, 0x43).rw("mpsc", FUNC(upd7201_new_device::ba_cd_r), FUNC(upd7201_new_device::ba_cd_w));
	map(0x60, 0x61).rw("usart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x80, 0x83).w("pit", FUNC(pit8253_device::write));
	map(0xa0, 0xa3).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
}

void v102_state::kbd_map(address_map &map)
{
	map(0x000, 0x7ff).rom().region("keyboard", 0);
}

static INPUT_PORTS_START(v102)
INPUT_PORTS_END

void v102_state::v102(machine_config &config)
{
	Z80(config, m_maincpu, 18.575_MHz_XTAL / 5); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &v102_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &v102_state::io_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(18.575_MHz_XTAL, 970, 0, 800, 319, 0, 300);
	//screen.set_raw(18.575_MHz_XTAL, 948, 0, 792, 319, 0, 300);
	screen.set_screen_update(FUNC(v102_state::screen_update));

	crt9007_device &vpac(CRT9007(config, "vpac", 18.575_MHz_XTAL / 10));
	vpac.set_character_width(10); // 6 in 132-column mode
	vpac.int_callback().set("mainirq", FUNC(input_merger_device::in_w<2>));

	EEPROM_2804(config, "eeprom");

	upd7201_new_device &mpsc(UPD7201_NEW(config, "mpsc", 18.575_MHz_XTAL / 5)); // divider not verified
	mpsc.out_int_callback().set("mainirq", FUNC(input_merger_device::in_w<0>));

	i8251_device &usart(I8251(config, "usart", 18.575_MHz_XTAL / 5)); // divider not verified
	usart.rxrdy_handler().set("mainirq", FUNC(input_merger_device::in_w<1>));

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	PIT8253(config, "pit", 0);

	I8255(config, "ppi");

	mcs48_cpu_device &kbdcpu(I8039(config, "kbdcpu", 11000000));
	kbdcpu.set_addrmap(AS_PROGRAM, &v102_state::kbd_map);
}


/**************************************************************************************************************

Visual 102. (VT-102 clone plus graphics)
Chips: D780C-1 (Z80), CRT9021B-018, COM8251A, D8255AC-5, 2x CRT9006-135, CRT9007, M5L8253P-5, X2804AP-35, D7201C
Crystals: 18.575000
Keyboard: TMP8039P-6

***************************************************************************************************************/

ROM_START( v102 )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD( "245-001.u1",  0x0000, 0x4000, CRC(c36cc525) SHA1(a45e75ded10979c8e3ad262e2cf5818e08db762c) )

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "260-001.u50", 0x0000, 0x1000, CRC(732f5b99) SHA1(d105bf9f3ed41109d7181bcf0223bb280afe3f0a) )

	ROM_REGION(0x0800, "keyboard", 0)
	ROM_LOAD( "150.kbd",     0x0000, 0x0800, CRC(afe55cff) SHA1(b26ebdde63ec0e94c08780285def39a282e128b3) )
ROM_END

COMP( 1984, v102, 0, 0, v102, v102, v102_state, empty_init, "Visual Technology", "Visual 102", MACHINE_IS_SKELETON )
