// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

2017-11-03 Skeleton

Altos II terminal. Green screen.

Chips: Z80A, 2x Z80DART, Z80CTC, X2210D, 2x CRT9006, CRT9007, CRT9021A, 8x 6116

Other: Beeper.  Crystals: 4.9152, 8.000, 40.000

Keyboard: P8035L CPU, undumped 2716 labelled "358_2758", XTAL marked "4608-300-107 KSS4C"

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80dart.h"
#include "machine/clock.h"
#include "machine/x2212.h"
//#include "video/crt9007.h"
//#include "video/crt9021.h"
#include "screen.h"

class altos2_state : public driver_device
{
public:
	altos2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_novram(*this, "novram")
		, m_p_chargen(*this, "chargen")
		, m_p_videoram(*this, "videoram")
	{ }

	DECLARE_WRITE8_MEMBER(video_mode_w);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void altos2(machine_config &config);
	void io_map(address_map &map);
	void mem_map(address_map &map);
private:
	virtual void machine_reset() override;

	required_device<cpu_device> m_maincpu;
	required_device<x2210_device> m_novram;
	required_region_ptr<u8> m_p_chargen;
	required_shared_ptr<u8> m_p_videoram;
};

void altos2_state::machine_reset()
{
	m_novram->recall(ASSERT_LINE);
	m_novram->recall(CLEAR_LINE);
}

WRITE8_MEMBER(altos2_state::video_mode_w)
{
	// D5 = 1 for 132-column mode (6-pixel char width)
	// D5 = 0 for 80-column mode (10-pixel char width)
	logerror("Writing %02X to mode register at %s\n", data, machine().describe_context());
}

u32 altos2_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

ADDRESS_MAP_START(altos2_state::mem_map)
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0xc000, 0xffff) AM_RAM AM_SHARE("videoram")
ADDRESS_MAP_END

ADDRESS_MAP_START(altos2_state::io_map)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("dart1", z80dart_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("dart2", z80dart_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE("ctc", z80ctc_device, read, write)
	AM_RANGE(0x0c, 0x0c) AM_WRITE(video_mode_w)
	AM_RANGE(0x40, 0x7f) AM_DEVREADWRITE("novram", x2210_device, read, write)
	//AM_RANGE(0x80, 0xff) AM_DEVREADWRITE_MOD("vpac", crt9007_device, read, write, rshift<1>)
ADDRESS_MAP_END

static INPUT_PORTS_START( altos2 )
INPUT_PORTS_END

static const z80_daisy_config daisy_chain[] =
{
	{ "dart1" },
	{ "dart2" },
	{ "ctc" },
	{ nullptr }
};

MACHINE_CONFIG_START(altos2_state::altos2)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL(4'000'000)) // unknown clock
	MCFG_CPU_PROGRAM_MAP(mem_map)
	MCFG_CPU_IO_MAP(io_map)
	MCFG_Z80_DAISY_CHAIN(daisy_chain)

	MCFG_DEVICE_ADD("ctc_clock", CLOCK, 4915200) // ctc & dart connections are guesswork
	MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE("ctc", z80ctc_device, trg0))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("ctc", z80ctc_device, trg1))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("ctc", z80ctc_device, trg2))

	MCFG_DEVICE_ADD("ctc", Z80CTC, XTAL(4'000'000))
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(DEVWRITELINE("dart1", z80dart_device, txca_w))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("dart1", z80dart_device, rxca_w))
	MCFG_Z80CTC_ZC1_CB(DEVWRITELINE("dart1", z80dart_device, rxtxcb_w))
	MCFG_Z80CTC_ZC2_CB(DEVWRITELINE("dart2", z80dart_device, rxca_w))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("dart2", z80dart_device, txca_w))

	MCFG_DEVICE_ADD("dart1", Z80DART, XTAL(4'000'000))
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_DEVICE_ADD("dart2", Z80DART, XTAL(4'000'000)) // channel B not used for communications
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80DART_OUT_DTRB_CB(DEVWRITELINE("novram", x2210_device, store)) MCFG_DEVCB_INVERT // FIXME: no inverter should be needed

	MCFG_DEVICE_ADD("novram", X2210, 0)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL(40'000'000) / 2, 960, 0, 800, 347, 0, 325)
	MCFG_SCREEN_UPDATE_DRIVER(altos2_state, screen_update)

	//MCFG_DEVICE_ADD("vpac", CRT9007, VPAC_CLOCK)
	//MCFG_CRT9007_INT_CALLBACK(DEVWRITELINE("ctc", z80ctc_device, trg3))
	//MCFG_VIDEO_SET_SCREEN("screen")
MACHINE_CONFIG_END

ROM_START( altos2 )
	ROM_REGION( 0x4000, "roms", 0 )
	ROM_LOAD( "us_v1.2_15732.u32", 0x0000, 0x2000, CRC(a85f7be0) SHA1(3cfa954c916258d86f7f745d10ec2ff5e33261b3) )
	ROM_LOAD( "us_v1.2_15733.u19", 0x2000, 0x2000, CRC(45ebe88a) SHA1(33f16b382a2b365122ebf5e5f7312f8afa45ad15) )

	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "us_v1.1_14410.u34", 0x0000, 0x2000, CRC(0ebb78bf) SHA1(96a1f7d34ff35037cbbc93049c0e2b9c9f11f1db) )
ROM_END

COMP( 1983, altos2, 0, 0, altos2, altos2, altos2_state, 0, "Altos", "Altos II Terminal", MACHINE_IS_SKELETON )
