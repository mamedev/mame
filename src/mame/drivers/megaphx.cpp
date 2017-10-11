// license: BSD-3-Clause
// copyright-holders: David Haywood, Dirk Best
/*
   Dinamic / Inder arcade hardware

   Mega Phoenix

 also known to exist on this hardware:
   Hammer Boy
   Nonamed 2 (ever finished? only code seen has 1991 date and is vastly incomplete) (versions exist for Amstrad CPC, MSX and Spectrum)
   After The War



  trivia: Test mode graphics are the same as Little Robin(?!), TMS is very similar too, suggesting they share a common codebase.

 PIC16C54 info:
 - The PIC has 5 functions:
   * Read dip switches (serially connected) [cmd 0x82 0x86]
   * Read the two start buttons [returned with all commands]
   * Provide 4 security codes. For the dumped PIC those are:
     0x4a 0x6f 0x61 0x6e (Joan). Not used by Mega Phoenix. [cmd 0x8a 0x8e 0x92 0x96]
   * Watchdog enable. Not used by Mega Phoenix. [cmd 9a]
   * Provide PIC software version to the game. This is 0x11 here. [all other cmds]
 - Communication with the game is achieved using a 8255 PPI on port C that is
   connected serially to the PIC. For port assignments see the code below.
 - The game sends an 8-bit command. After each bit, the PIC sends an answer bit.
   The start buttons are always bits 2 and 3 here.
 - All sent commands look like this: 1ccccc10
 - After the command was received, the PIC will send an additional 8 bits
   with the result.



 ToDo:
  - where should roms 6/7 map, they contain the 68k vectors, but the game expects RAM at 0, and it doesn't seem to read any of the other data from those roms.. they contain
    a cross hatch pattern amongst other things?
 Sound:
  - how does banking work? when the irq callbacks happen for each irq level? currently no way to access this because it's a daisy chain setup with the ctc?
  - even if i hack that the title screen speech doesn't work properly - is there a timing register like little robin?
 I/O:
  - Verify when m_ppi_to_pic_command is set and cleared. It's currently guessed but seems to work fine this way.


  --


  Chips of note

  Main board:

  TS68000CP8
  TMS34010FNL-40
  TMP82C55AP-2

  Bt478KPJ35  Palette / RAMDAC

  Actel A1010A-PL68C  (custom blitter maybe?)

  2x 8 DSW, bottom corner, away from everything..

 Sub / Sound board:

  ST Z8430AB1

  custom INDER badged chip 40 pin?  (probably just a z80 - it's in the sound section)
    MODELO: MEGA PHOENIX
    KIT NO. 1.034
    FECHA FABRICACION 08.10.91
    LA MANIPULCION DE LA ETIQUETA O DE LA PLACA ANULA SU SARANTIA
    (this sticker is also present on the other PCB)


*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "machine/74166.h"
#include "machine/i8255.h"
#include "machine/inder_sb.h"
#include "machine/inder_vid.h"



class megaphx_state : public driver_device
{
public:
	megaphx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mainram(*this, "mainram"),
		m_indersb(*this, "inder_sb"),
		m_indervid(*this, "inder_vid"),
		m_ppi(*this, "ppi8255_0"),
		m_dsw_shifter{ {*this, "ttl166_1"}, {*this, "ttl166_2"} },
		m_dsw_data(0),
		m_ppi_to_pic_command(0), m_ppi_to_pic_clock(0), m_ppi_to_pic_data(0),
		m_pic_to_ppi_clock(0), m_pic_to_ppi_data(0)
	{ }

	DECLARE_READ8_MEMBER(pic_porta_r);
	DECLARE_WRITE8_MEMBER(pic_porta_w);
	DECLARE_READ8_MEMBER(pic_portb_r);
	DECLARE_WRITE8_MEMBER(pic_portb_w);
	DECLARE_READ8_MEMBER(ppi_portc_r);
	DECLARE_WRITE8_MEMBER(ppi_portc_w);
	DECLARE_WRITE_LINE_MEMBER(dsw_w);

	DECLARE_DRIVER_INIT(megaphx);

private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint16_t> m_mainram;
	required_device<inder_sb_device> m_indersb;
	required_device<inder_vid_device> m_indervid;
	required_device<i8255_device> m_ppi;
	required_device<ttl166_device> m_dsw_shifter[2];

	int m_dsw_data;
	int m_ppi_to_pic_command;
	int m_ppi_to_pic_clock;
	int m_ppi_to_pic_data;
	int m_pic_to_ppi_clock;
	int m_pic_to_ppi_data;
};


static ADDRESS_MAP_START( megaphx_68k_map, AS_PROGRAM, 16, megaphx_state )
	AM_RANGE(0x000000, 0x0013ff) AM_RAM AM_SHARE("mainram") // maps over part of the rom??
	AM_RANGE(0x000000, 0x03ffff) AM_ROM AM_REGION("roms67", 0x00000) // or the rom doesn't map here? it contains the service mode grid amongst other things..
	AM_RANGE(0x040000, 0x040007) AM_DEVREADWRITE("inder_vid:tms", tms34010_device, host_r, host_w)
	AM_RANGE(0x050000, 0x050001) AM_DEVWRITE("inder_sb", inder_sb_device, megaphx_0x050000_w)
	AM_RANGE(0x050002, 0x050003) AM_DEVREAD("inder_sb", inder_sb_device, megaphx_0x050002_r)
	AM_RANGE(0x060000, 0x060007) AM_DEVREADWRITE8("ppi8255_0", i8255_device, read, write, 0x00ff)
	AM_RANGE(0x800000, 0x83ffff) AM_ROM  AM_REGION("roms01", 0x00000) // code + bg gfx are in here
	AM_RANGE(0x840000, 0x87ffff) AM_ROM  AM_REGION("roms23", 0x00000) // bg gfx are in here
	AM_RANGE(0x880000, 0x8bffff) AM_ROM  AM_REGION("roms45", 0x00000) // bg gfx + title screen in here
ADDRESS_MAP_END




static INPUT_PORTS_START( megaphx )
	PORT_START("P1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON2)        PORT_PLAYER(1) // shield
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON3)        PORT_PLAYER(1) // unused? (in test mode)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_PLAYER(1) // high score entry
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_PLAYER(1) // high score entry
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_PLAYER(1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON1)        PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON2)        PORT_PLAYER(2) // shield
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_COIN2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON3)        PORT_PLAYER(2) // unused? (in test mode)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_PLAYER(2) // high score entry
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_PLAYER(2) // high score entry
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_PLAYER(2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON1)        PORT_PLAYER(2)

	PORT_START("START")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_START2)

	PORT_START("DSW1")
	PORT_DIPNAME(0x07, 0x03, DEF_STR( Coin_A )) PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(   0x07, DEF_STR( 5C_1C ))
	PORT_DIPSETTING(   0x06, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(   0x05, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(   0x04, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(   0x03, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(   0x02, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(   0x01, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(   0x00, DEF_STR( 1C_4C ))
	PORT_DIPNAME(0x38, 0x18, DEF_STR( Coin_B )) PORT_DIPLOCATION("SW1:!4,!5,!6")
	PORT_DIPSETTING(   0x38, DEF_STR( 5C_1C ))
	PORT_DIPSETTING(   0x30, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(   0x28, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(   0x20, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(   0x18, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(   0x10, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(   0x08, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(   0x00, DEF_STR( 1C_4C ))
	PORT_DIPNAME(0xc0, 0x80, DEF_STR( Lives ))  PORT_DIPLOCATION("SW1:!7,!8")
	PORT_DIPSETTING(   0x00, "2")
	PORT_DIPSETTING(   0x40, "3")
	PORT_DIPSETTING(   0x80, "4")
	PORT_DIPSETTING(   0xc0, "5")

	PORT_START("DSW2")
	PORT_DIPNAME(0x01, 0x00, DEF_STR( Demo_Sounds )) PORT_DIPLOCATION("SW2:!1")
	PORT_DIPSETTING(   0x01, DEF_STR( Off ))
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_SERVICE_DIPLOC(0x02, IP_ACTIVE_HIGH, "SW2:!2")
	PORT_DIPNAME(0x1c, 0x10, DEF_STR( Difficulty))   PORT_DIPLOCATION("SW2:!3,!4,!5") // sets the enemy speed
	PORT_DIPSETTING(   0x00, "0")
	PORT_DIPSETTING(   0x04, "1")
	PORT_DIPSETTING(   0x08, "2")
	PORT_DIPSETTING(   0x0c, "3")
	PORT_DIPSETTING(   0x10, "4")
	PORT_DIPSETTING(   0x14, "5")
	PORT_DIPSETTING(   0x18, "6")
	PORT_DIPSETTING(   0x1c, "7")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:!6") // something to do with time in hammer boy??
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:!7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:!8")
INPUT_PORTS_END


READ8_MEMBER( megaphx_state::pic_porta_r )
{
	uint8_t data = 0;

	// 3-  serial clock
	// -0  serial data

	// clear when the pic starts reading the command
	m_ppi_to_pic_command = 0;

	data |= m_ppi_to_pic_data << 0;
	data |= m_ppi_to_pic_clock << 3;

	return data;
}

WRITE8_MEMBER( megaphx_state::pic_porta_w )
{
	// 2--  serial clock
	// -1-  not used
	// --0  serial data

	m_pic_to_ppi_data = BIT(data, 0);
	m_pic_to_ppi_clock = BIT(data, 2);
}

READ8_MEMBER( megaphx_state::pic_portb_r )
{
	uint8_t data = 0;

	// 5---  start2
	// -4--  start1
	// --1-  dip switch data
	// ---0  ppi to pic command incoming

	data |= m_ppi_to_pic_command << 0;
	data |= m_dsw_data << 1;
	data |= ioport("START")->read() << 4;

	return data;
}

WRITE8_MEMBER( megaphx_state::pic_portb_w )
{
	// 7---  not used
	// -6--  watchdog (not enabled by megaphx)
	// --3-  dsw clock
	// ---2  dsw shift/load

	m_dsw_shifter[0]->shift_load_w(BIT(data, 2));
	m_dsw_shifter[1]->shift_load_w(BIT(data, 2));
	m_dsw_shifter[0]->clock_w(BIT(data, 3));
	m_dsw_shifter[1]->clock_w(BIT(data, 3));
}

READ8_MEMBER( megaphx_state::ppi_portc_r )
{
	uint8_t data = 0;

	// 3---  pic to ppi clock
	// -2--  unknown
	// --1-  pic to ppi data
	// ---0  unknown

	data |= m_pic_to_ppi_data << 1;
	data |= m_pic_to_ppi_clock << 3;

	return data;
}

WRITE8_MEMBER( megaphx_state::ppi_portc_w )
{
	// 7---  ppi to pic clock
	// -6--  ppi to pic data
	// --5-  unknown
	// ---4  ppi to pic command

	// only set, don't clear here. otherwise the pic has no chance to pick it up
	if (BIT(data, 4))
		m_ppi_to_pic_command = BIT(data, 4);

	m_ppi_to_pic_data = BIT(data, 6);
	m_ppi_to_pic_clock = BIT(data, 7);
}

WRITE_LINE_MEMBER( megaphx_state::dsw_w )
{
	m_dsw_data = state;
}

static MACHINE_CONFIG_START( megaphx )
	MCFG_CPU_ADD("maincpu", M68000, 8000000) // ??  can't read xtal due to reflections, CPU is an 8Mhz part
	MCFG_CPU_PROGRAM_MAP(megaphx_68k_map)

	MCFG_CPU_ADD("pic", PIC16C54, 12000000)    /* 3MHz */
	MCFG_PIC16C5x_READ_A_CB(READ8(megaphx_state, pic_porta_r))
	MCFG_PIC16C5x_WRITE_A_CB(WRITE8(megaphx_state, pic_porta_w))
	MCFG_PIC16C5x_READ_B_CB(READ8(megaphx_state, pic_portb_r))
	MCFG_PIC16C5x_WRITE_B_CB(WRITE8(megaphx_state, pic_portb_w))

	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	MCFG_TTL166_ADD("ttl166_1")
	MCFG_TTL166_DATA_CB(IOPORT("DSW1"))
	MCFG_TTL166_QH_CB(DEVWRITELINE("ttl166_2", ttl166_device, serial_w))

	MCFG_TTL166_ADD("ttl166_2")
	MCFG_TTL166_DATA_CB(IOPORT("DSW2"))
	MCFG_TTL166_QH_CB(WRITELINE(megaphx_state, dsw_w))

	MCFG_DEVICE_ADD("ppi8255_0", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("P1"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("P2"))
	MCFG_I8255_IN_PORTC_CB(READ8(megaphx_state, ppi_portc_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(megaphx_state, ppi_portc_w))

	MCFG_INDER_VIDEO_ADD("inder_vid")

	MCFG_INDER_AUDIO_ADD("inder_sb")
MACHINE_CONFIG_END

DRIVER_INIT_MEMBER(megaphx_state, megaphx)
{
	uint16_t *src = (uint16_t*)memregion( "roms67" )->base();
	// copy vector table? - it must be writable because the game write the irq vector..
	memcpy(m_mainram, src, 0x80);
}


ROM_START( megaphx )
	ROM_REGION16_BE( 0x40000, "roms67", 0 )  // the majority of the data in these does not get used?! (only the vector table) is it just garbage??
	ROM_LOAD16_BYTE( "mph6.u32", 0x000001, 0x20000, CRC(b99703d4) SHA1(393b6869e71d4c61060e66e0e9e36a1e6ca345d1) )
	ROM_LOAD16_BYTE( "mph7.u21", 0x000000, 0x20000, CRC(f11e7449) SHA1(1017142d10011d68e49d3ccdb1ac4e815c03b17a) )

	ROM_REGION16_BE( 0x40000, "roms01", 0 )
	ROM_LOAD16_BYTE( "mph0.u38", 0x000001, 0x20000, CRC(b63dd20f) SHA1(c8ce5985a6ba49428d66a49d9d623ccdfce422c2) )
	ROM_LOAD16_BYTE( "mph1.u27", 0x000000, 0x20000, CRC(4dcbf44b) SHA1(a8fa49ecd033f1aeb323e0032ddcf5f8f9463ac0) )

	ROM_REGION16_BE( 0x40000, "roms23", 0 )
	ROM_LOAD16_BYTE( "mph2.u37", 0x000001, 0x20000, CRC(a0f69c27) SHA1(d0c5c241d94a1f03f51e7e517e2f9dec6abcf75a) )
	ROM_LOAD16_BYTE( "mph3.u26", 0x000000, 0x20000, CRC(4db84cc5) SHA1(dd74acd4b32c7e7553554ac0f9ba13503358e869) )

	ROM_REGION16_BE( 0x40000, "roms45", 0 )
	ROM_LOAD16_BYTE( "mph4.u36", 0x000001, 0x20000, CRC(c8e0725e) SHA1(b3af315b9a94a692e81e0dbfd4035036c2af4f50) )
	ROM_LOAD16_BYTE( "mph5.u25", 0x000000, 0x20000, CRC(c95ccb69) SHA1(9d14cbfafd943f6ff461a7f373170a35e36eb695) )

	ROM_REGION( 0x200000, "inder_sb:user2", 0 )
	ROM_LOAD( "sonido_mph1.u39", 0x00000, 0x20000, CRC(f5e65557) SHA1(5ae759c2bcef96fbda42f088c02b6dec208030f3) )
	ROM_LOAD( "sonido_mph2.u38", 0x20000, 0x20000, CRC(7444d0f9) SHA1(9739b48993bccea5530533b67808d13d6155ffe3) )

	ROM_REGION( 0x100000, "inder_sb:audiocpu", 0 )
	ROM_LOAD( "sonido_mph0.u35", 0x000000, 0x2000,  CRC(abc1b140) SHA1(8384a162d85cf9ea870d22f44b1ca64001c6a083) )

	ROM_REGION( 0x100000, "pic", 0 )
	ROM_LOAD( "pic16c54-xt.bin", 0x000000, 0x430,  CRC(21f396fb) SHA1(c8badb9b3681e684bced0ced1de4c3a15641de8b) )

	ROM_REGION( 0x100000, "pals", 0 ) // jedutil won't convert these? are they bad?
	ROM_LOAD( "p31_u31_palce16v8h-25.jed", 0x000, 0xbd4, CRC(05ef04b7) SHA1(330dd81a832b6675fb0473868c26fe9bec2da854) )
	ROM_LOAD( "p40_u29_palce16v8h-25.jed", 0x000, 0xbd4, CRC(44b7e51c) SHA1(b8b34f3b319d664ec3ad72ed87d9f65701f183a5) )
ROM_END

GAME( 1991, megaphx,  0,        megaphx, megaphx, megaphx_state, megaphx, ROT0, "Dinamic / Inder", "Mega Phoenix", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
