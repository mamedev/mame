// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

Skeleton driver for HP-2620 series display terminals.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/mos6551.h"
#include "machine/nvram.h"
//#include "video/dp8350.h"
#include "screen.h"

class hp2620_state : public driver_device
{
public:
	hp2620_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
		, m_nvram(*this, "nvram")
	{ }

	DECLARE_READ8_MEMBER(nvram_r);
	DECLARE_WRITE8_MEMBER(nvram_w);
	DECLARE_READ8_MEMBER(keystat_r);
	DECLARE_WRITE8_MEMBER(keydisp_w);
	DECLARE_READ8_MEMBER(sysstat_r);
	DECLARE_WRITE8_MEMBER(modem_w);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void hp2622(machine_config &config);
	void io_map(address_map &map);
	void mem_map(address_map &map);
private:
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
	required_shared_ptr<u8> m_nvram;
};


u32 hp2620_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

READ8_MEMBER(hp2620_state::nvram_r)
{
	return 0xf0 | m_nvram[offset];
}

WRITE8_MEMBER(hp2620_state::nvram_w)
{
	m_nvram[offset] = data & 0x0f;
}

READ8_MEMBER(hp2620_state::keystat_r)
{
	return 0xff;
}

WRITE8_MEMBER(hp2620_state::keydisp_w)
{
}

READ8_MEMBER(hp2620_state::sysstat_r)
{
	return 0xff;
}

WRITE8_MEMBER(hp2620_state::modem_w)
{
}

ADDRESS_MAP_START(hp2620_state::mem_map)
	AM_RANGE(0x0000, 0xbfff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0xc000, 0xffff) AM_RAM
ADDRESS_MAP_END

ADDRESS_MAP_START(hp2620_state::io_map)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x7f) AM_READWRITE(nvram_r, nvram_w) AM_SHARE("nvram")
	AM_RANGE(0x80, 0x80) AM_READ(keystat_r)
	AM_RANGE(0x90, 0x90) AM_READ(sysstat_r)
	AM_RANGE(0xa0, 0xa3) AM_DEVWRITE("acia", mos6551_device, write)
	AM_RANGE(0xa4, 0xa7) AM_DEVREAD("acia", mos6551_device, read)
	AM_RANGE(0xa8, 0xa8) AM_WRITE(modem_w)
	AM_RANGE(0xb8, 0xb8) AM_WRITE(keydisp_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( hp2622 )
INPUT_PORTS_END

MACHINE_CONFIG_START(hp2620_state::hp2622)
	MCFG_CPU_ADD("maincpu", Z80, XTAL(25'771'500) / 7) // 3.68 MHz
	MCFG_CPU_PROGRAM_MAP(mem_map)
	MCFG_CPU_IO_MAP(io_map)

	MCFG_NVRAM_ADD_0FILL("nvram") // 5101 (A7 tied to GND) + battery (+ wait states)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL(25'771'500), 1035, 0, 720, 415, 0, 390) // 498 total lines in 50 Hz mode
	MCFG_SCREEN_UPDATE_DRIVER(hp2620_state, screen_update)

	//MCFG_DEVICE_ADD("crtc", DP8367, XTAL(25'771'500))

	MCFG_DEVICE_ADD("acia", MOS6551, 0) // SY6551
	MCFG_MOS6551_XTAL(XTAL(25'771'500) / 14) // 1.84 MHz
	MCFG_MOS6551_IRQ_HANDLER(INPUTLINE("maincpu", 0))
MACHINE_CONFIG_END

/**************************************************************************************************************

Hewlett-Packard HP-2622A.
Chips: National 8367 CRTC (labeled B8250), SY6551 (labeled 8251), Z8400A (Z80)
Crystal: 25.7715

***************************************************************************************************************/

ROM_START( hp2622a )
	ROM_REGION(0xc000, "maincpu", 0)
	ROM_LOAD( "1818-1685.xu63", 0x0000, 0x2000, CRC(a57ffe5e) SHA1(4d7844320deba916d9ec289927af987fea025b02) )
	ROM_LOAD( "1818-1686.xu64", 0x2000, 0x2000, CRC(bee9274c) SHA1(20796c559031a91cb2666776fcf7ffdb52a0a318) )
	ROM_LOAD( "1818-1687.xu65", 0x4000, 0x2000, CRC(e9ecd489) SHA1(9b249b8d066d256069ccdb8809bb808c414f106a) )
	// XU66-XU68 are empty

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD( "1818-1489.xu311", 0x0000, 0x2000, CRC(9879b153) SHA1(fc1705d6de38eb6d3a67f1ae439e359e5124d028) )
ROM_END

COMP( 1982, hp2622a, 0, 0, hp2622, hp2622, hp2620_state, 0, "HP", "HP-2622A", MACHINE_IS_SKELETON )
