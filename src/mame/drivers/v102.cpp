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


ADDRESS_MAP_START(v102_state::mem_map)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x8000, 0x8fff) AM_RAM
	AM_RANGE(0xa000, 0xafff) AM_RAM
	AM_RANGE(0xb800, 0xb9ff) AM_DEVREADWRITE("eeprom", eeprom_parallel_28xx_device, read, write)
ADDRESS_MAP_END

ADDRESS_MAP_START(v102_state::io_map)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	//AM_RANGE(0x00, 0x3f) AM_DEVREADWRITE("vpac", crt9007_device, read, write)
	AM_RANGE(0x18, 0x19) AM_WRITENOP
	AM_RANGE(0x40, 0x43) AM_DEVREADWRITE("mpsc", upd7201_new_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0x60, 0x60) AM_DEVREADWRITE("usart", i8251_device, data_r, data_w)
	AM_RANGE(0x61, 0x61) AM_DEVREADWRITE("usart", i8251_device, status_r, control_w)
	AM_RANGE(0x80, 0x83) AM_DEVWRITE("pit", pit8253_device, write)
	AM_RANGE(0xa0, 0xa3) AM_DEVREADWRITE("ppi", i8255_device, read, write)
	//AM_RANGE(0xbf, 0xbf) ???
ADDRESS_MAP_END

ADDRESS_MAP_START(v102_state::kbd_map)
	AM_RANGE(0x000, 0x7ff) AM_ROM AM_REGION("keyboard", 0)
ADDRESS_MAP_END

static INPUT_PORTS_START( v102 )
INPUT_PORTS_END

MACHINE_CONFIG_START(v102_state::v102)
	MCFG_CPU_ADD("maincpu", Z80, XTAL(18'575'000) / 5) // divider not verified
	MCFG_CPU_PROGRAM_MAP(mem_map)
	MCFG_CPU_IO_MAP(io_map)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL(18'575'000), 970, 0, 800, 319, 0, 300)
	//MCFG_SCREEN_RAW_PARAMS(XTAL(18'575'000), 948, 0, 792, 319, 0, 300)
	MCFG_SCREEN_UPDATE_DRIVER(v102_state, screen_update)

	//MCFG_DEVICE_ADD("vpac", CRT9007, CRTC_CLOCK)
	//MCFG_CRT9007_CHARACTER_WIDTH(6 or 10)

	MCFG_EEPROM_2804_ADD("eeprom")

	MCFG_DEVICE_ADD("mpsc", UPD7201_NEW, XTAL(18'575'000) / 5) // divider not verified
	MCFG_Z80SIO_OUT_INT_CB(DEVWRITELINE("mainirq", input_merger_device, in_w<0>))

	MCFG_DEVICE_ADD("usart", I8251, XTAL(18'575'000) / 5) // divider not verified
	MCFG_I8251_RXRDY_HANDLER(DEVWRITELINE("mainirq", input_merger_device, in_w<1>))

	MCFG_INPUT_MERGER_ANY_HIGH("mainirq")
	MCFG_INPUT_MERGER_OUTPUT_HANDLER(INPUTLINE("maincpu", 0))

	MCFG_DEVICE_ADD("pit", PIT8253, 0)

	MCFG_DEVICE_ADD("ppi", I8255, 0)

	MCFG_CPU_ADD("kbdcpu", I8039, 12000000)
	MCFG_CPU_PROGRAM_MAP(kbd_map)
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

COMP( 1984, v102, 0, 0, v102, v102, v102_state, 0, "Visual Technology", "Visual 102", MACHINE_IS_SKELETON )
