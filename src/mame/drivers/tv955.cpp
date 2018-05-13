// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for "third generation" TeleVideo terminals (905, 955, 9220).

************************************************************************************************************************************/

#include "emu.h"
//#include "bus/rs232/rs232.h"
#include "cpu/m6502/m65c02.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/input_merger.h"
#include "machine/mos6551.h"
#include "machine/nvram.h"
#include "video/scn2674.h"
#include "screen.h"

class tv955_state : public driver_device
{
public:
	tv955_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_crtc(*this, "crtc")
		, m_hostuart(*this, "hostuart")
		, m_p_chargen(*this, "chargen")
	{ }

	void tv955(machine_config &config);
private:
	SCN2674_DRAW_CHARACTER_MEMBER(draw_character);

	DECLARE_WRITE8_MEMBER(control_latch_w);

	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<scn2674_device> m_crtc;
	required_device<mos6551_device> m_hostuart;
	required_region_ptr<u8> m_p_chargen;
};

SCN2674_DRAW_CHARACTER_MEMBER(tv955_state::draw_character)
{
}

WRITE8_MEMBER(tv955_state::control_latch_w)
{
	m_hostuart->set_xtal(BIT(data, 1) ? 3.6864_MHz_XTAL : 3.6864_MHz_XTAL / 2);

	// CPU clock is inverted relative to character clock (and divided by two for 132-column mode)
	if (BIT(data, 7))
	{
		// 132-column mode
		m_maincpu->set_unscaled_clock(31.684_MHz_XTAL / 18);
		m_crtc->set_unscaled_clock(31.684_MHz_XTAL / 9);
	}
	else
	{
		// 80-column mode
		m_maincpu->set_unscaled_clock(19.3396_MHz_XTAL / 9);
		m_crtc->set_unscaled_clock(19.3396_MHz_XTAL / 9);
	}
}

void tv955_state::mem_map(address_map &map)
{
	// verified from maintenance manual (131968-00-C)
	map(0x0000, 0x07ff).mirror(0x0800).ram().share("nvram");
	map(0x1100, 0x1100).mirror(0x00ff).w(this, FUNC(tv955_state::control_latch_w));
	map(0x1200, 0x1203).mirror(0x00fc).rw("keybuart", FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0x1400, 0x1403).mirror(0x00fc).rw("printuart", FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0x1800, 0x1803).mirror(0x00fc).rw("hostuart", FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0x2000, 0x2007).mirror(0x0ff8).rw("crtc", FUNC(scn2674_device::read), FUNC(scn2674_device::write));
	map(0x3000, 0x3fff).rom().region("option", 0);
	map(0x4000, 0x7fff).ram().share("attrram");
	map(0x8000, 0xbfff).ram().share("charram");
	map(0xc000, 0xffff).rom().region("system", 0);
}

static INPUT_PORTS_START( tv955 )
INPUT_PORTS_END

MACHINE_CONFIG_START(tv955_state::tv955)
	MCFG_DEVICE_ADD("maincpu", M65C02, 19.3396_MHz_XTAL / 9)
	MCFG_DEVICE_PROGRAM_MAP(mem_map)

	MCFG_INPUT_MERGER_ANY_HIGH("mainirq")
	MCFG_INPUT_MERGER_OUTPUT_HANDLER(INPUTLINE("maincpu", m6502_device::IRQ_LINE))

	MCFG_DEVICE_ADD("keyboard", I8049, 5.7143_MHz_XTAL)

	MCFG_NVRAM_ADD_0FILL("nvram") // HM6116LP-4 + 3.2V battery

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(19.3396_MHz_XTAL, 846, 0, 720, 381, 0, 364)
	//MCFG_SCREEN_RAW_PARAMS(31.684_MHz_XTAL, 1386, 0, 1188, 381, 0, 364)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", scn2674_device, screen_update)

	MCFG_DEVICE_ADD("crtc", SCN2674, 19.3396_MHz_XTAL / 9)
	// Character clock is 31.684_MHz_XTAL / 9 in 132-column mode
	// Character cells are 9 pixels wide by 14 pixels high
	MCFG_SCN2674_CHARACTER_WIDTH(9)
	MCFG_SCN2674_DRAW_CHARACTER_CALLBACK_OWNER(tv955_state, draw_character)
	MCFG_SCN2674_INTR_CALLBACK(INPUTLINE("maincpu", m6502_device::NMI_LINE))
	MCFG_VIDEO_SET_SCREEN("screen")

	MCFG_DEVICE_ADD("hostuart", MOS6551, 0)
	MCFG_MOS6551_XTAL(3.6864_MHz_XTAL)
	MCFG_MOS6551_IRQ_HANDLER(WRITELINE("mainirq", input_merger_device, in_w<0>))

	MCFG_DEVICE_ADD("printuart", MOS6551, 0)
	MCFG_MOS6551_XTAL(3.6864_MHz_XTAL / 2)
	MCFG_MOS6551_IRQ_HANDLER(WRITELINE("mainirq", input_merger_device, in_w<1>))

	MCFG_DEVICE_ADD("keybuart", MOS6551, 0)
	MCFG_MOS6551_XTAL(3.6864_MHz_XTAL / 2)
	MCFG_MOS6551_IRQ_HANDLER(WRITELINE("mainirq", input_merger_device, in_w<2>))
MACHINE_CONFIG_END

/**************************************************************************************************************

Televideo TVI-955 (132160-00 Rev. M)
Chips: G65SC02P-3, 3x S6551AP, SCN2674B, AMI 131406-00 (unknown 40-pin DIL), 2x TMM2064P-10 (near two similar empty sockets), HM6116LP-4, round silver battery
Crystals: 19.3396, 31.684, 3.6864
Keyboard: M5L8049-230P-6, 5.7143, Beeper

***************************************************************************************************************/

ROM_START( tv955 )
	ROM_REGION(0x4000, "system", 0)
	ROM_LOAD( "t180002-88d_955.u4",  0x0000, 0x4000, CRC(5767fbe7) SHA1(49a2241612af5c3af09778ffa541ac0bc186e05a) )

	ROM_REGION(0x1000, "option", 0)
	ROM_LOAD( "t180002-91a_calc.u5", 0x0000, 0x1000, CRC(f86c103a) SHA1(fa3ada3a5d8913e519e2ea4817e96166c1fedd32) )
	ROM_CONTINUE( 0x0000, 0x1000 ) // first half is all FF (and not addressable)

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "t180002-26b.u45",     0x0000, 0x1000, CRC(69c9ebc7) SHA1(32282c816ec597a7c45e939acb7a4155d35ea584) )

	ROM_REGION(0x0800, "keyboard", 0)
	ROM_LOAD( "8049.kbd",            0x0000, 0x0800, CRC(bc86e349) SHA1(0b62003ab7931822f1bcac8370517c685849f62c) )
ROM_END

COMP( 1985, tv955, 0, 0, tv955, tv955, tv955_state, empty_init, "TeleVideo Systems", "TeleVideo 955", MACHINE_IS_SKELETON )
