// license:BSD-3-Clause
// copyright-holders:
/*******************************************************************************

    TeleVideo TVI-912/TVI-920 terminals

    This was the first series of terminals from TeleVideo. The models differed
    from each other in the number and pattern of keys on the nondetachable
    keyboard. Those with a B suffix had a TTY-style keyboard, while the C
    suffix indicated a typewriter-style keyboard. The TVI-920 added a row of
    function keys but was otherwise mostly identical to the TVI-912.

*******************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
//#include "bus/rs232/rs232.h"
//#include "machine/ay31015.h"
#include "machine/bankdev.h"
#include "video/tms9927.h"
#include "screen.h"

#define CHAR_WIDTH 14

class tv912_state : public driver_device
{
public:
	tv912_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_crtc(*this, "crtc")
		, m_bankdev(*this, "bankdev")
		, m_dispram_bank(*this, "dispram")
		, m_p_chargen(*this, "chargen")
		, m_sw2(*this, "SW2")
	{ }

	DECLARE_WRITE8_MEMBER(p2_w);
	DECLARE_READ8_MEMBER(crtc_r);
	DECLARE_WRITE8_MEMBER(crtc_w);
	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_WRITE8_MEMBER(output_40c);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void tv912(machine_config &config);
private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_device<cpu_device> m_maincpu;
	required_device<tms9927_device> m_crtc;
	required_device<address_map_bank_device> m_bankdev;
	required_memory_bank m_dispram_bank;
	required_region_ptr<u8> m_p_chargen;
	required_ioport m_sw2;

	std::unique_ptr<u8[]> m_dispram;
};

WRITE8_MEMBER(tv912_state::p2_w)
{
	// P20-P23: Address Signals (4MSBS)
	m_bankdev->set_bank(data & 0x0f);

	// P24: +4Hz Flasher
	// P25: -DCR
	// P26: -PTR RDY
	// P27: -HALF DUPLEX
}

READ8_MEMBER(tv912_state::crtc_r)
{
	return m_crtc->read(space, bitswap<4>(offset, 5, 4, 1, 0));
}

WRITE8_MEMBER(tv912_state::crtc_w)
{
	m_crtc->write(space, bitswap<4>(offset, 5, 4, 1, 0), data);
}

READ8_MEMBER(tv912_state::keyboard_r)
{
	u8 result = 0xff;

	if (!BIT(m_sw2->read(), 0))
		result &= 0x7f;

	return result;
}

WRITE8_MEMBER(tv912_state::output_40c)
{
	// Bit 5: +FORCE BLANK
	// Bit 4: +SEL LPT
	// Bit 3: -BREAK
	// Bit 2: -RQS
	// Bit 1: +BEEP

	// Bit 0: +PG SEL
	m_dispram_bank->set_entry(BIT(data, 0));
}

u32 tv912_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void tv912_state::machine_start()
{
	m_dispram = make_unique_clear<u8[]>(0x1000);
	m_dispram_bank->configure_entries(0, 2, m_dispram.get(), 0x800);

	save_pointer(NAME(m_dispram.get()), 0x1000);
}

void tv912_state::machine_reset()
{
	m_dispram_bank->set_entry(0);
}

static ADDRESS_MAP_START( prog_map, AS_PROGRAM, 8, tv912_state )
	AM_RANGE(0x000, 0xfff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, tv912_state )
	AM_RANGE(0x00, 0xff) AM_DEVICE("bankdev", address_map_bank_device, amap8)
ADDRESS_MAP_END

static ADDRESS_MAP_START( bank_map, 0, 8, tv912_state )
	AM_RANGE(0x000, 0x0ff) AM_MIRROR(0x300) AM_RAM
	AM_RANGE(0x400, 0x403) AM_MIRROR(0x3c0) AM_SELECT(0x030) AM_READWRITE(crtc_r, crtc_w)
	AM_RANGE(0x40c, 0x40f) AM_MIRROR(0x3f0) AM_READ(keyboard_r)
	AM_RANGE(0x40c, 0x40c) AM_MIRROR(0x3f0) AM_WRITE(output_40c)
	AM_RANGE(0x800, 0xfff) AM_RAMBANK("dispram")
ADDRESS_MAP_END

static INPUT_PORTS_START( switches )
	PORT_START("SW2")
	PORT_DIPNAME(0x01, 0x01, "Refresh Rate") PORT_DIPLOCATION("S2:4")
	PORT_DIPSETTING(0x00, "50 Hz")
	PORT_DIPSETTING(0x01, "60 Hz")
ADDRESS_MAP_END

static INPUT_PORTS_START( tv912b )
	PORT_INCLUDE(switches)
INPUT_PORTS_END

static INPUT_PORTS_START( tv912c )
	PORT_INCLUDE(switches)
INPUT_PORTS_END

MACHINE_CONFIG_START(tv912_state::tv912)
	MCFG_CPU_ADD("maincpu", I8035, XTAL_23_814MHz / 4)
	MCFG_CPU_PROGRAM_MAP(prog_map)
	MCFG_CPU_IO_MAP(io_map)
	MCFG_MCS48_PORT_P2_OUT_CB(WRITE8(tv912_state, p2_w))
	MCFG_MCS48_PORT_T1_IN_CB(DEVREADLINE("crtc", tms9927_device, bl_r))

	MCFG_DEVICE_ADD("bankdev", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(bank_map)
	MCFG_ADDRESS_MAP_BANK_DATA_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDR_WIDTH(12)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x100)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_23_814MHz, 105 * CHAR_WIDTH, 0, 80 * CHAR_WIDTH, 270, 0, 240)
	MCFG_SCREEN_UPDATE_DRIVER(tv912_state, screen_update)

	MCFG_DEVICE_ADD("crtc", TMS9927, XTAL_23_814MHz)
	MCFG_TMS9927_CHAR_WIDTH(CHAR_WIDTH)
	MCFG_TMS9927_VSYN_CALLBACK(INPUTLINE("maincpu", MCS48_INPUT_IRQ))
MACHINE_CONFIG_END

/**************************************************************************************************************

Televideo TVI-912C.
Chips: i8035, TMS9927NL, AY5-1013A (COM2502)
Crystals: 23.814 (divide by 4 for CPU clock)
Other: 1x 8-sw DIP, 1x 10-sw DIP (internal), 2x 10-sw DIP (available to user at the back)

***************************************************************************************************************/

ROM_START( tv912c )
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD( "a49c1.bin",    0x0000, 0x1000, CRC(40068371) SHA1(44c32f8c3980acebe28fa48f98479910af2eb4ae) ) // MM52132

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD( "a3-2.bin",     0x0000, 0x0800, CRC(bb9a7fbd) SHA1(5f1c4d41b25bd3ca4dbc336873362935daf283da) ) // EA8316
ROM_END

ROM_START( tv912b )
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD( "televideo912b_rom_a49.bin", 0x0000, 0x1000, CRC(2c95e995) SHA1(77cda383d68b0bbbb783026d8fde679f10f9eded) ) // MM52132

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD( "televideo912b_rom_a3.bin", 0x0000, 0x0800, CRC(bb9a7fbd) SHA1(5f1c4d41b25bd3ca4dbc336873362935daf283da) ) // 2316E
ROM_END

COMP( 1978, tv912c, 0,      0, tv912, tv912c, tv912_state, 0, "TeleVideo Systems", "TVI-912C", MACHINE_IS_SKELETON )
COMP( 1978, tv912b, tv912c, 0, tv912, tv912b, tv912_state, 0, "TeleVideo Systems", "TVI-912B", MACHINE_IS_SKELETON )
