// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    SNES bootlegs controlled by an MCS-51 core

	Skeleton driver

	Currently there are dumps for a Mortal Kombat 3 board and a board with
	4 slots that supports various games directly and also has a generic
	timer based mode.

	Hardware (for the 4 slot switcher):
	- MCS-51 based CPU 44-pin (markings removed)
	- WD1016D-PL (?)
	- TC5565PL-15 (8k SRAM)
	- 12 MHz XTAL
	- 4 Position Dipswitch

	Connector pinout:

                 JAMMA                      P1 (from SNES)
    GND          A | 1           GND         1 - GND
    GND          B | 2           GND         2 -
    +5v          C | 3           +5v         3 -
    +5v          D | 4           +5v         4 -
    -5v          E | 5           -5v         5 -
    +12v         F | 6          +12v         6 -
                 H | 7                       7 -
                 J | 8                       8 -
                 K | 9                       9 -
    SND-         L | 10         SND+        10 -
    P2 Button R  M | 11  P1 Button R        11 -
    Video GREEN  N | 12    Video RED        12 -
    Video SYNC   P | 13   Video BLUE
                 R | 14    Video GND        P2 (unknown)
    P2 Button L  S | 15  P2 Button L        1 -
                 T | 16      Coin #1        2 -
    P2 Start     U | 17     P1 Start        3 -
    P2 Up        V | 18        P1 Up
    P2 Down      W | 19      P1 Down        P3 (unknown)
    P2 Left      X | 20      P1 Left        1 - +5v
    P2 Right     Y | 21     P1 Right        2 - GND
    P2 Button X  Z | 22  P1 Button X
    P2 Button Y  a | 23  P1 Button Y        P4 (unknown)
    P2 Button A  b | 24  P1 Button A        1 - +5v
    P2 Button B  c | 25  P1 Button B        2 -
    P2 Select    d | 26    P1 Select        3 -
    GND          e | 27          GND        4 -
    GND          f | 28          GND        5 - N/C
                                            6 -
    * Note - P5 ribbon to SNES controls     7 - GND

***************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "emupal.h"

class snesb51_state : public driver_device
{
public:
	snesb51_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_mcu(*this, "mcu")
		{ }

	void mk3snes(machine_config &config);
	void snes4sl(machine_config &config);

protected:
	void machine_start() override;

private:
	required_device<mcs51_cpu_device> m_mcu;

	void mem_map(address_map &map);

	void mcu_p1_w(uint8_t data);
	void mcu_p3_w(uint8_t data);
};

void snesb51_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("mcu", 0);
}

static INPUT_PORTS_START( mk3snes )
INPUT_PORTS_END

static INPUT_PORTS_START( snes4sl )
	PORT_START("S1")
	PORT_DIPNAME(0x03, 0x00, "Game Time")    PORT_DIPLOCATION("S1:1,2")
	PORT_DIPSETTING(   0x00, "5 Minutes")
	PORT_DIPSETTING(   0x01, "15 Minutes")
	PORT_DIPSETTING(   0x02, "20 Minutes")
	PORT_DIPSETTING(   0x03, "10 Minutes")
	PORT_DIPUNKNOWN(0x04, 0x00)              PORT_DIPLOCATION("S1:3")
	PORT_DIPNAME(0x08, 0x00, "Mode")         PORT_DIPLOCATION("S1:4")
	PORT_DIPSETTING(   0x00, "Credit/Timer")
	PORT_DIPSETTING(   0x08, "Timer")
INPUT_PORTS_END

const gfx_layout char_layout =
{
	8,16,
	38,
	1,
	{ RGN_FRAC(0,1) },
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	8*16
};

static GFXDECODE_START(chars)
	GFXDECODE_ENTRY("mcu", 0x3ec8, char_layout, 0, 1)
GFXDECODE_END

void snesb51_state::mcu_p1_w(uint8_t data)
{
	logerror("mcu_p1_w: %02x\n", data);
}

void snesb51_state::mcu_p3_w(uint8_t data)
{
	logerror("mcu_p3_w: %02x\n", data);
}

void snesb51_state::machine_start()
{

}

void snesb51_state::mk3snes(machine_config &config)
{
	I8751(config, m_mcu, 12_MHz_XTAL / 6);
	m_mcu->port_out_cb<1>().set(FUNC(snesb51_state::mcu_p1_w));
	m_mcu->port_out_cb<3>().set(FUNC(snesb51_state::mcu_p3_w));
}

void snesb51_state::snes4sl(machine_config &config)
{
	// exact type and clock unknown
	I8031(config, m_mcu, 12_MHz_XTAL / 6);
	m_mcu->set_addrmap(AS_PROGRAM, &snesb51_state::mem_map);
	m_mcu->port_out_cb<1>().set(FUNC(snesb51_state::mcu_p1_w));
	m_mcu->port_out_cb<3>().set(FUNC(snesb51_state::mcu_p3_w));

	PALETTE(config, "palette", palette_device::MONOCHROME);

	GFXDECODE(config, "gfxdecode", "palette", chars);
}

// this is identical to the snes release apart from a single byte
ROM_START( mk3snes )
	ROM_REGION(0x400000, "game", 0)
	ROM_LOAD("5.u5", 0x000000, 0x080000, CRC(c21ee1ac) SHA1(12fc526e39b0b998b39d558fbe5660e72c7fad14))
	ROM_LOAD("6.u6", 0x080000, 0x080000, CRC(0e064323) SHA1(a11175516892beb862c7cc1e186034ef1b55ee8f))
	ROM_LOAD("7.u7", 0x100000, 0x080000, CRC(7db6b7be) SHA1(a7653c04f5321fd83062425a492c7ed0a4f1fdb0))
	ROM_LOAD("8.u8", 0x180000, 0x080000, CRC(28771750) SHA1(d6c469ca2640935b6687f5bf5f6e85275157abb0))
	ROM_LOAD("1.u1", 0x200000, 0x080000, CRC(4cab6332) SHA1(3c417ba6d35532b4e2ca9ae4a3b730c589d26aee))
	ROM_LOAD("2.u2", 0x280000, 0x080000, CRC(0327999b) SHA1(dc6bb11a925e893453e0e5e5d88b8ace8d6cf859))
	ROM_LOAD("3.u3", 0x300000, 0x080000, CRC(229af2de) SHA1(1bbb02aec08afab979ffbe4b68a48dc4cc923f73))
	// this rom has 1 byte changed compared to sns-a3me-0.u1 (mk3u in snes softlist)
	ROM_LOAD("4.u4", 0x380000, 0x080000, CRC(b51930d9) SHA1(220f00d64809a6218015a738e53f11d8dc81578f))

	ROM_REGION(0x1000, "mcu", 0)
	ROM_LOAD("d87c51.u9", 0x0000, 0x1000, CRC(f447620a) SHA1(ac0d78c7b339f13d5f96a6727a0f2147158697f9))

	ROM_REGION(0x100, "audiocpu", 0)
	ROM_LOAD("spc700.rom", 0x00, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0))
ROM_END

ROM_START( snes4sl )
	ROM_REGION(0x8000, "mcu", 0)
	ROM_LOAD("27c256_11-03.bin", 0x0000, 0x8000, CRC(4e471581) SHA1(0f23ad065d448097f56ab45c3850d53cf85f3670))
ROM_END

//    YEAR  NAME     PARENT   MACHINE  INPUT    CLASS          INIT        ROT   COMPANY    FULLNAME                          FLAGS
GAME( 199?, mk3snes, 0,       mk3snes, mk3snes, snesb51_state, empty_init, ROT0, "bootleg", "Mortal Kombat 3 (SNES bootleg)", MACHINE_IS_SKELETON )
GAME( 1993, snes4sl, 0,       snes4sl, snes4sl, snesb51_state, empty_init, ROT0, "bootleg", "SNES 4 Slot arcade switcher",    MACHINE_IS_SKELETON )
