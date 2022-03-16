// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    SNES bootlegs controlled by an MCS-51 core

    Skeleton driver

    The following systems are dumped:
    - Fatal Fury Special
    - Mortal Kombat 3
    - 4 Slot board (two different BIOS versions)
    - 4 Slot board with built-in NBA Jam

    Hardware (for the 4 slot switcher):
    - Unknown PLC44 chip (markings removed) near a 8192-word x 8-bit Static RAM
    - WD1016D-PL (MCS-51 MCU) or SAB 8051A-P
    - TC5565PL-15 (8k SRAM)
    - 12 MHz XTAL
    - 4 Position Dipswitch (for configuring the game time)

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

    Dumper's notes for MK3:
    "Ok can confirm with this style the MCU provides an overlay similar to the "NBA jam 4 slot" snes bootlegs.
    I have an onscreen insert coin display from the MCU but the underlying snes hardware is dead so actual game is not running."

    TODO: complete MCU hookup
***************************************************************************/

#include "emu.h"
#include "includes/snes.h"
#include "cpu/mcs51/mcs51.h"
#include "emupal.h"
#include "speaker.h"


namespace {

class snesb51_state : public snes_state
{
public:
	snesb51_state(const machine_config &mconfig, device_type type, const char *tag) :
		snes_state(mconfig, type, tag),
		m_mcu(*this, "mcu")
		{ }

	void base(machine_config &config);
	void mk3snes(machine_config &config);
	void snes4sl(machine_config &config);
	void snes4sln(machine_config &config);

	void init_fatfurspb();

protected:
	void machine_start() override;

private:
	required_device<mcs51_cpu_device> m_mcu;

	void mem_map(address_map &map);
	void io_map(address_map &map);
	void snes_map(address_map &map);
	void spc_map(address_map &map);

	void mcu_p1_w(uint8_t data);
	uint8_t mcu_p3_r();
	void mcu_p3_w(uint8_t data);

	void ram_address_high_w(uint8_t data);
	void ram_address_low_w(uint8_t data);
	void ram_w(uint8_t data);

	std::unique_ptr<uint8_t[]> m_ram;
	uint16_t m_ram_address;
};

void snesb51_state::snes_map(address_map &map)
{
	map(0x000000, 0x7dffff).rw(FUNC(snesb51_state::snes_r_bank1), FUNC(snesb51_state::snes_w_bank1));
	map(0x7e0000, 0x7fffff).ram().share(m_wram);                 // 8KB Low RAM, 24KB High RAM, 96KB Expanded RAM
	map(0x800000, 0xffffff).rw(FUNC(snesb51_state::snes_r_bank2), FUNC(snesb51_state::snes_w_bank2));    // Mirror and ROM
}

void snesb51_state::spc_map(address_map &map)
{
	map(0x0000, 0xffff).ram().share("aram");
}

void snesb51_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("mcu", 0);
}

void snesb51_state::io_map(address_map &map)
{
//  map(0x8000, 0x8000).r
//  map(0x8200, 0x8200).r
//  map(0x8401, 0x8401).w
	map(0x8c00, 0x8c00).w(FUNC(snesb51_state::ram_address_high_w));
	map(0x8e00, 0x8e00).w(FUNC(snesb51_state::ram_address_low_w));
//  map(0x9000, 0xafff).ram();
	map(0xa000, 0xa000).w(FUNC(snesb51_state::ram_w));
//  map(0xb000, 0xb000).w
//  map(0xd000, 0xd000).w
//  map(0xd400, 0xd400).w
//  map(0xd800, 0xd800).r
}

static INPUT_PORTS_START( mk3snes )
	PORT_START("SERIAL1_DATA1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Button B") PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P1 Button Y") PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("P1 Start")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Button A") PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P1 Button X") PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P1 Button L") PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Button R") PORT_PLAYER(1)
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SERIAL2_DATA1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Button B") PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P2 Button Y") PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("P2 Start")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Button A") PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P2 Button X") PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P2 Button L") PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P2 Button R") PORT_PLAYER(2)
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SERIAL1_DATA2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SERIAL2_DATA2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( snes4sl )
	PORT_INCLUDE(mk3snes)

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

uint8_t snesb51_state::mcu_p3_r()
{
	logerror("mcu_p3_r\n");
	return machine().rand();
}

void snesb51_state::mcu_p3_w(uint8_t data)
{
	logerror("mcu_p3_w: %02x\n", data);
}

void snesb51_state::ram_address_high_w(uint8_t data)
{
	data &= 0x1f;
	m_ram_address = (data << 8) | (m_ram_address & 0x00ff);
}

void snesb51_state::ram_address_low_w(uint8_t data)
{
	m_ram_address = (m_ram_address & 0xff00) | data;
}

void snesb51_state::ram_w(uint8_t data)
{
	m_ram[m_ram_address] = data;
}

void snesb51_state::machine_start()
{
	m_ram = std::make_unique<uint8_t[]>(0x2000);
	save_pointer(NAME(m_ram), 0x2000);

	snes_state::machine_start();
}

void snesb51_state::base(machine_config &config)
{
	// basic machine hardware
	_5A22(config, m_maincpu, 3580000 * 6);
	m_maincpu->set_addrmap(AS_PROGRAM, &snesb51_state::snes_map);

	S_SMP(config, m_soundcpu, XTAL(24'576'000) / 12);
	m_soundcpu->set_addrmap(AS_DATA, &snesb51_state::spc_map);
	m_soundcpu->dsp_io_read_callback().set(m_s_dsp, FUNC(s_dsp_device::dsp_io_r));
	m_soundcpu->dsp_io_write_callback().set(m_s_dsp, FUNC(s_dsp_device::dsp_io_w));

	config.set_perfect_quantum(m_maincpu);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(DOTCLK_NTSC * 2, SNES_HTOTAL * 2, 0, SNES_SCR_WIDTH * 2, SNES_VTOTAL_NTSC, 0, SNES_SCR_HEIGHT_NTSC);
	m_screen->set_video_attributes(VIDEO_VARIABLE_WIDTH);
	m_screen->set_screen_update(FUNC(snes_state::screen_update));

	SNES_PPU(config, m_ppu, MCLK_NTSC);
	m_ppu->open_bus_callback().set([this] { return snes_open_bus_r(); });
	m_ppu->set_screen("screen");

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	S_DSP(config, m_s_dsp, XTAL(24'576'000) / 12);
	m_s_dsp->set_addrmap(0, &snesb51_state::spc_map);
	m_s_dsp->add_route(0, "lspeaker", 1.00);
	m_s_dsp->add_route(1, "rspeaker", 1.00);
}

void snesb51_state::mk3snes(machine_config &config)
{
	base(config);

	I8751(config, m_mcu, 12_MHz_XTAL);
	m_mcu->set_addrmap(AS_IO, &snesb51_state::io_map);
	m_mcu->port_out_cb<1>().set(FUNC(snesb51_state::mcu_p1_w));
	m_mcu->port_in_cb<3>().set(FUNC(snesb51_state::mcu_p3_r));
	m_mcu->port_out_cb<3>().set(FUNC(snesb51_state::mcu_p3_w));
}

void snesb51_state::snes4sl(machine_config &config)
{
	base(config);

	// exact type unknown
	I8031(config, m_mcu, 12_MHz_XTAL);
	m_mcu->set_addrmap(AS_PROGRAM, &snesb51_state::mem_map);
	m_mcu->set_addrmap(AS_IO, &snesb51_state::io_map);
	m_mcu->port_out_cb<1>().set(FUNC(snesb51_state::mcu_p1_w));
	m_mcu->port_in_cb<3>().set(FUNC(snesb51_state::mcu_p3_r));
	m_mcu->port_out_cb<3>().set(FUNC(snesb51_state::mcu_p3_w));

	PALETTE(config, "palette", palette_device::MONOCHROME);

	GFXDECODE(config, "gfxdecode", "palette", chars);
}

void snesb51_state::snes4sln(machine_config &config)
{
	base(config);

	I8051(config, m_mcu, 12_MHz_XTAL); // SAB 8051A-P
	m_mcu->set_addrmap(AS_PROGRAM, &snesb51_state::mem_map);
	m_mcu->set_addrmap(AS_IO, &snesb51_state::io_map);
	m_mcu->port_out_cb<1>().set(FUNC(snesb51_state::mcu_p1_w));
	m_mcu->port_in_cb<3>().set(FUNC(snesb51_state::mcu_p3_r));
	m_mcu->port_out_cb<3>().set(FUNC(snesb51_state::mcu_p3_w));
}

// This is identical to the SNES release apart from a single byte
ROM_START( mk3snes )
	ROM_REGION(0x400000, "user3", 0)
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
	ROM_REGION(0x400000, "user3", ROMREGION_ERASEFF)

	ROM_REGION(0x8000, "mcu", 0)
	ROM_SYSTEM_BIOS(0, "1207", "12-07") // Found on PCB with Siemens SAB 8051A-P (4KBytes internal ROM undumped)
	ROMX_LOAD("27c256_12-07.bin", 0x0000, 0x8000, CRC(0922314d) SHA1(04f1265ddc753111e6fcd56162a917ae1791c164), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "1103", "11-03") // Found on PCB with WD1016D-PL
	ROMX_LOAD("27c256_11-03.bin", 0x0000, 0x8000, CRC(4e471581) SHA1(0f23ad065d448097f56ab45c3850d53cf85f3670), ROM_BIOS(1))

	ROM_DEFAULT_BIOS("1207")
ROM_END

ROM_START( snes4sln )
	// identical to NBA Jam (nbajamua)
	ROM_REGION(0x400000, "user3", 0)
	ROM_LOAD("4.bin", 0x000000, 0x080000, CRC(d27bbebe) SHA1(fcf614fcd3c0fb06037038514d828f0cb597838e))
	ROM_LOAD("3.bin", 0x080000, 0x080000, CRC(c0ca8c3c) SHA1(af5caa1b0254f6b42e4f7f3ba07d0af904017e3c))
	ROM_LOAD("2.bin", 0x100000, 0x080000, CRC(4de70641) SHA1(1f575282a1bb842afcc80e29cebc13d96f8158a4))
	ROM_LOAD("1.bin", 0x180000, 0x080000, CRC(9d59835f) SHA1(061872d9a2b39d271fbfc90b9e8f9abefcfa4282))

	ROM_REGION(0x8000, "mcu", 0)
	ROM_LOAD("5.bin", 0x0000, 0x8000, CRC(af8a64e3) SHA1(f13187d213fe7c2a0edcb88d4e828bd24112e812))
ROM_END

ROM_START( fatfurspb )
	ROM_REGION( 0x400000, "user3", 0 )
	ROM_LOAD("1.u14", 0x000000, 0x100000, CRC(7cb9192c) SHA1(0247e303902e86eaa9443f2a39d352430df5f46f))
	ROM_LOAD("2.u15", 0x100000, 0x100000, CRC(440e3017) SHA1(7d1a2077032c761676bff7f841ab1fc669d322fa))
	ROM_LOAD("3.u16", 0x200000, 0x100000, CRC(dbbe10de) SHA1(27c590bff5a762a2528d0819b7544914ab6cae7c))
	ROM_LOAD("4.u17", 0x300000, 0x100000, CRC(a356e60c) SHA1(c403eff4e7c7deefed68a34a0dbeefadac8c7a0e))

	ROM_REGION(0x1000, "mcu", 0)
	ROM_LOAD("d87c51.u9", 0x0000, 0x1000, NO_DUMP)
ROM_END


void snesb51_state::init_fatfurspb()
{
	uint8_t *rom = memregion("user3")->base();

	for (uint32_t i = 0; i < 0x400000; i++)
	{
		rom[i] = bitswap<8>(rom[i], 5, 0, 6, 1, 7, 4, 3, 2);
	}

	init_snes_hirom();
}

} // anonymous namespace


//    YEAR  NAME       PARENT  MACHINE   INPUT    CLASS          INIT             ROT   COMPANY    FULLNAME                                 FLAGS
GAME( 199?, mk3snes,   0,      mk3snes,  mk3snes, snesb51_state, init_snes_hirom, ROT0, "bootleg", "Mortal Kombat 3 (SNES bootleg)",        MACHINE_IS_SKELETON )
GAME( 1993, snes4sl,   0,      snes4sl,  snes4sl, snesb51_state, init_snes,       ROT0, "bootleg", "SNES 4 Slot arcade switcher",           MACHINE_IS_SKELETON )
GAME( 1994, snes4sln,  0,      snes4sln, snes4sl, snesb51_state, init_snes,       ROT0, "bootleg", "SNES 4 Slot arcade switcher (NBA Jam)", MACHINE_IS_SKELETON )
GAME( 199?, fatfurspb, 0,      mk3snes,  mk3snes, snesb51_state, init_fatfurspb,  ROT0, "bootleg", "Fatal Fury Special (SNES bootleg)",     MACHINE_IS_SKELETON )
