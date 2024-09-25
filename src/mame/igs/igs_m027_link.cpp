// license:BSD-3-Clause
// copyright-holders:

/*
IGS ARM7 (IGS027A) based mahjong / gambling platform(s) with link support
Keeping them separate from igs_m027.cpp and igs017.cpp for now.

Linking has been verified to work fine on real hardware for both games.

The two dumped extension games are really similar to mgdh in igs017.cpp code-wise,
you can get them to the wait link screen just by implementing the mgdh's IGS MUX
stuff.

*********************************************************************************

Manguan Caishen 2, HOST PCB (IGS 1999)
Cai Jin Shen Long, HOST PCB (IGS 1999)

These boards are housed in a metal box.
The top of the box has an opening for a 6-digit 7-seg LED display and a keypad with numbers laid out like this....

XXXXXX

1 2 3
4 5 6
7 8 9
* 0 #

There are 2 locks on the side of the box that simply close 2 pins on the PCB. They will be service or operator
switches similar to those present on gambling cabinets.
The PCB has a 62-way connector on it but it not accessible to the outside of the metal box.

PCB Layout
----------

IGS PCB 0219-04 (Manguan Caishen 2, HOST)
IGS PCB 0219-1 (Cai Jin Shen Long, HOST)
|------------------------------------------------------------------|
|DIN5         DB9                                             DB25 |
|POWER     PC817                                                   |
|          PC817                                                   |
|JP3                                            LM2933             |
|JP4                                                      IGS025   |
|                                                                  |
|                                                                  |
|                                                                  |
|                                                                  |
|                                           IGS027A                |
|                                                          32.768kHz
|                                                                 *|
|                                                             V3021|
| T518B                                             EPROM.U13      |
|                                       BATTERY                    |
|   SW1                  22MHz                               62256 |
|                JP11   JP10            |-|    62-WAY       |-|    |
|---------------------------------------| |-----------------| |----|
Notes:
        * - On Cai Jin Shen Long, the V3021 and 32.768kHz crystal are replaced with a ST M48T08 NVRAM (the battery is dead and holds no data).
EPROM.U13 - Manguan Caishen 2 - labelled 'V206CMMBOX'
          - Cai Jin Shen Long - labelled 'V-106CSM'
    T518B - Reset IC
  JP10/11 - 2 pins (each) that get tied together when the key switches on the side of the metal box are locked
    JP3/4 - 2 pin (each) for 5V and GND
    62256 - 32kx8-bit SRAM
      SW1 - 8-position DIP Switch
    V3021 - NVRAM Battery Supervisor IC
   IGS025 - Manguan Caishen 2 - labelled 'S8'
          - Cai Jin Shen Long - labelled 'S2'
  IGS027A - Manguan Caishen 2 - labelled 'Y7'
          - Cai Jin Shen Long - labelled 'S2'

*********************************************************************************

*********************************************************************************

Manguan Caishen 2 (Link Version, Extension), IGS 1999
Cai Jin Shen Long (Link Version, Extension), IGS 1999

These boards connect to the HOST board running the same game.
At power-on the display shows "WAIT LINK"

PCB Layout
----------

IGS PCB 0199-03 (both games same PCB#)
|-------------------------------------------|
|                JAMMA                      |
| PC817(x20)                         TDA1020|
|                    SW2              78L05 |
|1     IGS025               M6295           |
|8                                          |
|W                                 S2002.U22|
|A                                          |
|Y      PAL                      S-xxxCN.U19|
|                                           |
|       PAL        68000             6264   |
|                                           |
|   M2001.U4                                |
|                                           |
|1                                   PAL    |
|0                 IGS031                   |
|W  T2003.U6                       61256    |
|A                                          |
|Y    22MHz                                 |
|  JP3                                      |
|       SW1       PAL   11.059MHz       SW3 |
|-------------------------------------------|
Notes:
      SW1/2 - 8-Position DIP Switch
        SW3 - Reset / Clear NVRAM Switch
        JP3 - 4-pin connector
      68000 - Clock 11MHz [22/2]
      62256 - 32kBx8-bit SRAM
       6264 - 8kBx8-bit SRAM
      M6295 - Oki Sound Chip. Clock input 1.000MHz [22/22]. Pin 7 HIGH.

*********************************************************************************/



#include "emu.h"

#include "igs017_igs031.h"
#include "igs025.h"
#include "pgmcrypt.h"

#include "cpu/arm7/arm7.h"
#include "cpu/m68000/m68000.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "sound/okim6295.h"

#include "screen.h"
#include "speaker.h"


namespace {

class host_state : public driver_device
{
public:
	host_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void host(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void host_map(address_map &map) ATTR_COLD;
};

class extension_state : public driver_device
{
public:
	extension_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_igs017_igs031(*this, "igs017_igs031"),
		m_screen(*this, "screen")
	{ }

	void cjsll(machine_config &config);
	void mgcs2l(machine_config &config);

	void init_cjsll();
	void init_mgcs2l();

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<igs017_igs031_device> m_igs017_igs031;
	required_device<screen_device> m_screen;

	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);

	void cjsll_map(address_map &map) ATTR_COLD;
	void mgcs2l_map(address_map &map) ATTR_COLD;

	void decrypt();
};


void extension_state::video_start()
{
	m_igs017_igs031->video_start();
}


void host_state::host_map(address_map &map)
{
	map(0x00000000, 0x00003fff).rom(); // Internal ROM
	map(0x08000000, 0x0800ffff).rom().region("user1", 0); // Game ROM (does it really map here? it appears to be connected indirectly via the 025)

	// TODO: IGS025?
	//map(0x28007000, 0x28007000).w(m_igs_mux, FUNC(igs_mux_device::address_w));
	//map(0x28007001, 0x28007001).rw(m_igs_mux, FUNC(igs_mux_device::data_r), FUNC(igs_mux_device::data_w));
}

void extension_state::cjsll_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x600000, 0x603fff).ram();

	// TODO: IGS025?
	//map(0x876000, 0x876001).nopr().w(m_igs_mux, FUNC(igs_mux_device::address_w)).umask16(0x00ff); // clr.w dummy read
	//map(0x876002, 0x876003).rw(m_igs_mux, FUNC(igs_mux_device::data_r), FUNC(igs_mux_device::data_w)).umask16(0x00ff);

	map(0xa00000, 0xa0ffff).rw(m_igs017_igs031, FUNC(igs017_igs031_device::read), FUNC(igs017_igs031_device::write)).umask16(0x00ff);

	//map(0xa12001, 0xa12001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

void extension_state::mgcs2l_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x600000, 0x603fff).ram();

	// TODO: IGS025?
	//map(0x893000, 0x893001).nopr().w(m_igs_mux, FUNC(igs_mux_device::address_w)).umask16(0x00ff); // clr.w dummy read
	//map(0x893002, 0x893003).rw(m_igs_mux, FUNC(igs_mux_device::data_r), FUNC(igs_mux_device::data_w)).umask16(0x00ff);

	map(0xa00000, 0xa0ffff).rw(m_igs017_igs031, FUNC(igs017_igs031_device::read), FUNC(igs017_igs031_device::write)).umask16(0x00ff);

	//map(0xa12001, 0xa12001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}


TIMER_DEVICE_CALLBACK_MEMBER(extension_state::interrupt)
{
	int const scanline = param;

	if (scanline == 240 && m_igs017_igs031->get_irq_enable())
		m_maincpu->set_input_line(1, HOLD_LINE);

	if (scanline == 0 && m_igs017_igs031->get_nmi_enable())
		m_maincpu->set_input_line(3, HOLD_LINE);
}


static INPUT_PORTS_START( host )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")
INPUT_PORTS_END

static INPUT_PORTS_START( extension )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

	PORT_START("SW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")
INPUT_PORTS_END


void host_state::host(machine_config &config)
{
	ARM7(config, m_maincpu, 22_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &host_state::host_map);

//  NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	IGS025(config, "igs025", 0);
}

void extension_state::cjsll(machine_config &config)
{
	M68000(config, m_maincpu, 22_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &extension_state::cjsll_map);

	TIMER(config, "scantimer").configure_scanline(FUNC(extension_state::interrupt), "screen", 0, 1);

//  NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(512, 256);
	m_screen->set_visarea(0, 512-1, 0, 240-1);
	m_screen->set_screen_update("igs017_igs031", FUNC(igs017_igs031_device::screen_update));
	m_screen->set_palette("igs017_igs031:palette");

	IGS025(config, "igs025", 0);

	IGS017_IGS031(config, m_igs017_igs031, 0);
	m_igs017_igs031->set_text_reverse_bits(true);

	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 22_MHz_XTAL / 22, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.5);
}

void extension_state::mgcs2l(machine_config &config)
{
	cjsll(config),

	m_maincpu->set_addrmap(AS_PROGRAM, &extension_state::mgcs2l_map);
}


ROM_START( mgcs2h )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A type G ARM based MCU
	ROM_LOAD( "y7_027a.bin", 0x00000, 0x4000, CRC(3e726eeb) SHA1(41b4e5f8a9d35b82b1a62029b34c1e19e188a3bc) )

	ROM_REGION32_LE( 0x10000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "v206cmmbox.u13", 0x00000, 0x10000, CRC(2bfdeeeb) SHA1(c92f8994f75e0eefb4dbf25daa0d62ad72a7ddfa) )
ROM_END

ROM_START( cjslh )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A type G ARM based MCU
	ROM_LOAD( "s2_027a.bin", 0x00000, 0x4000, CRC(6be397fd) SHA1(ccd2469995a0b90800e891c39f4b3eaa033783ec) )

	ROM_REGION32_LE( 0x10000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "v-106csm.u13", 0x00000, 0x10000, CRC(5b3f3446) SHA1(1d5b9523ac7f221eb7cc2e5db90cc859c640cc18) )
ROM_END

ROM_START( mgcs2l )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "p2000.u19", 0x00000, 0x80000, CRC(05a065e8) SHA1(5302fcb272c1561380bce57d840ddb5cc45b9497) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "m2001.u4", 0x000000, 0x400000, CRC(b650cfcd) SHA1(3a4dd17965b1adf51a3c2b3ce3d55c37df1078b1) ) // FIXED BITS (xxxxxxx0xxxxxxxx)

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "t2003.u6", 0x00000, 0x80000, CRC(156c03d9) SHA1(daef148969d939a6ecc2f364d5b268a7cc82fa52) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "s2002.u22", 0x00000, 0x80000, CRC(9070c8ee) SHA1(43852ae1891b4d6c00a6fbe6a822e49d9e97ee97) )
ROM_END

ROM_START( cjsll )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "s-111cn.u19", 0x00000, 0x80000, CRC(d11d4fd7) SHA1(4c5fbf36ae1c28a0fa28225aa101c13640545bdb) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "m2001.u4", 0x000000, 0x400000, CRC(b650cfcd) SHA1(3a4dd17965b1adf51a3c2b3ce3d55c37df1078b1) ) // FIXED BITS (xxxxxxx0xxxxxxxx)

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "t2003.u6", 0x00000, 0x80000, CRC(156c03d9) SHA1(daef148969d939a6ecc2f364d5b268a7cc82fa52) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "s2002.u22", 0x00000, 0x80000, CRC(9070c8ee) SHA1(43852ae1891b4d6c00a6fbe6a822e49d9e97ee97) )
ROM_END


// TODO: reduce this monstrosity. Still some imperfections?
void extension_state::decrypt()
{
	const int rom_size = memregion("maincpu")->bytes();
	u16 * const rom = (u16 *)memregion("maincpu")->base();

	for (int i = 0; i < rom_size / 2; i++)
	{
		u16 x = rom[i];

		switch (i & 0x5000 / 2)
		{
			case 0x0000 / 2:
				if (!(i & 0x0300 / 2))
				{
					if (!(i & 0x0002 / 2))
						x ^= 0x0001;
					else
						if (!(i & 0x0020 / 2))
							x ^= 0x0001;
				}
				else
					if ((i & 0x0022 / 2) == (0x0022 / 2))
						x ^= 0x0001;
				break;
			case 0x1000 / 2:
				if (i & 0x300 / 2)
				{
					if ((i & 0x0022 / 2) == (0x0022 / 2))
						x ^= 0x0001;
				}
				else
				{
					if ((i & 0x0022 / 2) != (0x0022 / 2))
						x ^= 0x0001;
				}
				break;
			case 0x4000 / 2:
			case 0x5000 / 2: if ((i & 0x0022 / 2) == (0x0022 / 2)) x ^= 0x0001; break;
		}

		switch (i & 0x6000 / 2)
		{
			case 0x0000 / 2: if ((i & 0x280 / 2) == (0x280 / 2)) x ^= 0x0800; break;
			case 0x2000 / 2: if ((!(i & 0x800 / 2)) && ((i & 0x2c0 / 2) == (0x2c0 / 2))) x ^= 0x0800; break;
			case 0x4000 / 2: x ^= 0x0800; break;
			case 0x6000 / 2: if ((!(i & 0x800 / 2)) && (i & 0x40 / 2)) x ^= 0x0800; break;
		}

		if (i & 0x60000 / 2)
			x ^= 0x0100;

		rom[i] = x;
	}

	// TODO: tiles are scrambled in a different way from what's already supported, sprites to be verified
}

void extension_state::init_cjsll()
{
	decrypt();

	u16 * const rom = (u16 *)memregion("maincpu")->base();

	// game id check
	rom[0x3a994 / 2] = 0x4e71;
}

void extension_state::init_mgcs2l()
{
	decrypt();

	u16 * const rom = (u16 *)memregion("maincpu")->base();

	// game id check
	rom[0x3a48e / 2] = 0x4e71;
}

} // anonymous namespace


// hosts
GAME( 1999, mgcs2h, 0, host, host, host_state, empty_init, ROT0, "IGS", "Manguan Caishen 2 (link version, host)", MACHINE_IS_SKELETON )
GAME( 1999, cjslh,  0, host, host, host_state, empty_init, ROT0, "IGS", "Cai Jin Shen Long (link version, host)", MACHINE_IS_SKELETON )

// extensions
GAME( 1999, mgcs2l, 0, mgcs2l, extension, extension_state, init_mgcs2l, ROT0, "IGS", "Manguan Caishen 2 (link version, extension, S110CN)", MACHINE_IS_SKELETON )
GAME( 1999, cjsll,  0, cjsll,  extension, extension_state, init_cjsll,  ROT0, "IGS", "Cai Jin Shen Long (link version, extension, S111CN)", MACHINE_IS_SKELETON )
