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
//#include "video/crt9007.h"
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

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void v102(machine_config &config);
	void io_map(address_map &map);
	void kbd_map(address_map &map);
	void mem_map(address_map &map);
private:
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
	//AM_RANGE(0x00, 0x3f) AM_DEVREADWRITE("vpac", crt9007_device, read, write)
	map(0x18, 0x19).nopw();
	map(0x40, 0x43).rw("mpsc", FUNC(upd7201_new_device::ba_cd_r), FUNC(upd7201_new_device::ba_cd_w));
	map(0x60, 0x60).rw("usart", FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0x61, 0x61).rw("usart", FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0x80, 0x83).w("pit", FUNC(pit8253_device::write));
	map(0xa0, 0xa3).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	//AM_RANGE(0xbf, 0xbf) ???
}

void v102_state::kbd_map(address_map &map)
{
	map(0x000, 0x7ff).rom().region("keyboard", 0);
}

static INPUT_PORTS_START( v102 )
INPUT_PORTS_END

MACHINE_CONFIG_START(v102_state::v102)
	MCFG_DEVICE_ADD("maincpu", Z80, XTAL(18'575'000) / 5) // divider not verified
	MCFG_DEVICE_PROGRAM_MAP(mem_map)
	MCFG_DEVICE_IO_MAP(io_map)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL(18'575'000), 970, 0, 800, 319, 0, 300)
	//MCFG_SCREEN_RAW_PARAMS(XTAL(18'575'000), 948, 0, 792, 319, 0, 300)
	MCFG_SCREEN_UPDATE_DRIVER(v102_state, screen_update)

	//MCFG_DEVICE_ADD("vpac", CRT9007, CRTC_CLOCK)
	//MCFG_CRT9007_CHARACTER_WIDTH(6 or 10)

	MCFG_EEPROM_2804_ADD("eeprom")

	MCFG_DEVICE_ADD("mpsc", UPD7201_NEW, XTAL(18'575'000) / 5) // divider not verified
	MCFG_Z80SIO_OUT_INT_CB(WRITELINE("mainirq", input_merger_device, in_w<0>))

	MCFG_DEVICE_ADD("usart", I8251, XTAL(18'575'000) / 5) // divider not verified
	MCFG_I8251_RXRDY_HANDLER(WRITELINE("mainirq", input_merger_device, in_w<1>))

	MCFG_INPUT_MERGER_ANY_HIGH("mainirq")
	MCFG_INPUT_MERGER_OUTPUT_HANDLER(INPUTLINE("maincpu", 0))

	MCFG_DEVICE_ADD("pit", PIT8253, 0)

	MCFG_DEVICE_ADD("ppi", I8255, 0)

	MCFG_DEVICE_ADD("kbdcpu", I8039, 12000000)
	MCFG_DEVICE_PROGRAM_MAP(kbd_map)
MACHINE_CONFIG_END


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
