// license:BSD-3-Clause
// copyright-holders:Farfetch'd,David Haywood,Tomasz Slanina
/* Tecmo System
 Driver by Farfetch'd, David Haywood & Tomasz Slanina
 Protection simulation by nuapete

 TODO:
  - Dump / Decap MCUs to allow for proper protection emulation.
  - tkdenshoa pcb shows some garbage sprites at the top during y-scroll
  - Flip screen is wrong
  - Line scroll is unimplemented(but it seems like unused)
  - Unknowns of sprite/tilemap registers
  - Verify priority/mixing from real PCB


T.Slanina 20040530 :
 - preliminary gfx decode,
 - Angel Eyes - patched interrupt level1 vector
 - EEPROM r/w
 - txt layer
 - added hacks to see more gfx (press Z or X)
 - palette (press X in angel eyes to see 'color bar chack'(!))
 - watchdog (?) simulation

 20080528
 - Removed ROM patches and debug keypresses
 - Added protection simulation in machine/tecmosys.cpp
 - Fixed inputs
 - Added watchdog

   To enter test mode, you have to press the test switch before you insert any coins.

*/


/*

Deroon Dero Dero
Tecmo, 1996

This game is a Puyo Puyo rip-off.

PCB Layout
----------

TECMO SYSTEM BOARD A
|-------------------------------------------------------------------------|
|  LM324  UPC452C      16.9MHz          |--------|    |--------|    6264  |
| TA8205 LM324  YAC513 YMF262 YMZ280B   |TECMO   |    |TECMO   |    6264  |
|        LM324  M6295  UPC452C          |AA03-8431    |AA02-1927          |
|                      YAC512           |        |    |        |          |
|                                       |--------|    |--------|          |
|        Z80  6264 28MHz 14.31818MHz                  |--------|          |
|                    16MHz             62256          |TECMO   |          |
|            TA8030                    62256          |AA02-1927    6264  |
|                                                     |        |    6264  |
|J  93C46                               |--------|    |--------|          |
|A                                      |TECMO   |    |--------|          |
|M                                      |AA03-8431    |TECMO   |          |
|M          68000                       |        |    |AA02-1927          |
|A                                      |--------|    |        |    6264  |
|                  PAL              6116              |--------|    6264  |
|                                    6116             |--------|          |
|  |--------|                                         |TECMO   |          |
|  |TECMO   |                     PAL                 |AA02-1927          |
|  |AA03-8431  62256                                  |        |    6264  |
|  |        |  62256                                  |--------|    6264  |
|  |--------|                                  |---------|                |
|                                              |TECMO    |                |
|                                              |AA03-8431|                |
|                                              |         |                |
|                                              |---------|          424260|
|                                              62256 62256          424260|
|-------------------------------------------------------------------------|
Notes:
68000 @ 16MHz
Z80 @ 8MHz [16/2]
YMZ280B @ 16.9MHz
YMF262 @ 14.31818MHz
OKI M6295 @ 2MHz [16/8]. Pin 7 HIGH

Game Board
----------

TECMO SYSTEM BOARD B2
|-------------------------------------------------------------------------|
|    T201_DIP42_MASK.UBB1                                                 |
| |----|                                              T202_DIP42_MASK.UBC1|
| |*   |                                                                  |
| |----|                                                                  |
|                                                                         |
|  T003_2M_EPROM.UZ1                        T101_SOP44.UAH1               |
|                                                                         |
|                                            T301_DIP42_MASK.UBD1         |
|                                                                         |
|                                                                         |
|                                                                         |
|  T401_DIP42_MASK.UYA1      T104_SOP44.UCL1    T001_4M_EPROM.UPAU1       |
|                              T103_SOP44.UBL1                            |
|  T501_DIP32_MASK.UAD1      T102_SOP44.UAL1                              |
|                                                  T002_4M_EPROM.UPAL1    |
|-------------------------------------------------------------------------|
Notes:
      * - QFP64 microcontroller marked 'TECMO SC432146FU E23D 185 SSAB9540B'
          this is a 68HC11A8 with 8k ROM, 512 bytes EEPROM and 256 bytes on-chip RAM.
          Clocks: pin 33 - 8MHz, pin 31: 8MHz, pin 29 - 2MHz
          GND on pins 49, 23, 24, 27
          Power on pins 55, 25
          Note - Pins 25 and 27 are tied to some jumpers, so these
          appear to be some kind of configuration setting.

CPU  : TMP68HC000P-16
Sound: TMPZ84C00AP-8 YMF262 YMZ280B M6295
OSC  : 14.3181MHz (X1) 28.0000MHz (X2) 16.0000MHz (X3) 16.9MHz (X4)

Custom chips:
TECMO AA02-1927 (160pin PQFP) (x4)
TECMO AA03-8431 (208pin PQFP) (x4)

Others:
93C46 EEPROM (settings are stored to this)

ROMs:

name            type
t001.upau1      27c040 dip32 eprom
t002.upal1      27c040 dip32 eprom
t003.uz1        27c2001 dip32 eprom

t101.uah1       23c16000 sop44 maskrom
t102.ual1       23c16000 sop44 maskrom
t103.ubl1       23c32000 sop44 maskrom
t104.ucl1       23c16000 sop44 maskrom
t201.ubb1       23c8000 dip42 maskrom
t202.ubc1       23c8000 dip42 maskrom
t301.ubd1       23c8000 dip42 maskrom
t401.uya1       23c16000 dip42 maskrom
t501.uad1       23c4001 dip32 maskrom

*/

/*

Toukidenshou -Angel Eyes-
(c)1996 Tecmo
Tecmo System Board

CPU  : TMP68HC000P-16
Sound: TMPZ84C00AP-8 YMF262 YMZ280B M6295
OSC  : 14.3181MHz (X1) 28.0000MHz (X2) 16.0000MHz (X3) 16.9MHz (X4)

Custom chips:
TECMO AA02-1927 (160pin PQFP) (x4)
TECMO AA03-8431 (208pin PQFP) (x4)

Others:
93C46 EEPROM (settings are stored to this)

EPROMs:
aeprge-2.pal - Main program (even) (27c4001)
aeprgo-2.pau - Main program (odd)  (27c4001)

aesprg-2.z1 - Sound program (27c1001)

Mask ROMs:
ae100h.ah1 - Graphics (23c32000/16000 SOP)
ae100.al1  |
ae101h.bh1 |
ae101.bl1  |
ae102h.ch1 |
ae102.cl1  |
ae104.el1  |
ae105.fl1  |
ae106.gl1  /

ae200w74.ba1 - Graphics (23c16000)
ae201w75.bb1 |
ae202w76.bc1 /

ae300w36.bd1 - Graphics (23c4000)

ae400t23.ya1 - YMZ280B Samples (23c16000)
ae401t24.yb1 /

ae500w07.ad1 - M6295 Samples (23c4001)

*/

#include "emu.h"
#include "tecmosys.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"
#include "sound/ymz280b.h"
#include "speaker.h"


u8 tecmosys_state::sound_command_pending_r()
{
	return m_soundlatch->pending_r();
}

void tecmosys_state::sound_nmi_disable_w(u8 data)
{
	// 00 and FF are the only values written here; the latter value is set during initialization and NMI processing
	m_soundnmi->in_w<1>(data == 0);
}

/*
    880000 and 880002 might be video related,
    see sub @ 68k:002e5e where they are written if the screen is set to inverted.
    Also, irq code at 22c4 :
    - 880000 & 00, execute irq code
    - 880000 & 01, scroll?
    - 880000 & 03, crash
*/

void tecmosys_state::unk880000_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_880000regs[offset]);

	switch( offset )
	{
		case 0x00/2:
			break; // global x scroll for sprites?

		case 0x02/2:
			break; // global y scroll for sprites

		case 0x08/2:
			m_spritelist = data & 0x3; // which of the 4 spritelists to use (buffering)
			break;

		case 0x22/2:
			m_watchdog->watchdog_reset();
			//logerror( "watchdog_w( %06x, %04x ) @ %06x\n", (offset * 2)+0x880000, data, m_maincpu->_pc() );
			break;

		default:
			logerror( "unk880000_w( %06x, %04x ) @ %06x\n", (offset * 2)+0x880000, data, m_maincpu->pc() );
			break;
	}
}

u16 tecmosys_state::unk880000_r(offs_t offset)
{
	//u16 ret = m_880000regs[offset];

	logerror( "unk880000_r( %06x ) @ %06x = %04x\n", (offset * 2 ) +0x880000, m_maincpu->pc(), m_880000regs[offset] );

	/* this code allows scroll regs to be updated, but tkdensho at least resets perodically */

	switch( offset )
	{
	case 0:
		return (m_screen->vpos() >= 240) ? 0 : 1;

	default:
		return 0;
	}
}

u16 tecmosys_state::eeprom_r()
{
	return ((m_eeprom->do_read() & 0x01) << 11);
}

void tecmosys_state::eeprom_w(offs_t offset, u16 data, u16 mem_mask)
{
	if ( ACCESSING_BITS_8_15 )
	{
		m_eeprom->di_write((data & 0x0800) >> 11);
		m_eeprom->cs_write((data & 0x0200) ? ASSERT_LINE : CLEAR_LINE );
		m_eeprom->clk_write((data & 0x0400) ? CLEAR_LINE: ASSERT_LINE );
	}
}

template<int Layer>
void tecmosys_state::vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_vram[Layer][offset]);
	m_tilemap[Layer]->mark_tile_dirty(offset/2);
}

template<int Layer>
void tecmosys_state::lineram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_lineram[Layer][offset]);
	if (data!=0x0000) popmessage("non 0 write to bg%01x lineram %04x %04x",Layer,offset,data);
}

void tecmosys_state::main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x200000, 0x20ffff).ram(); // work ram
	map(0x210000, 0x210001).nopr(); // single byte overflow on stack defined as 0x210000
	map(0x300000, 0x300fff).ram().w(FUNC(tecmosys_state::vram_w<1>)).share("vram_1"); // bg0 ram
	map(0x301000, 0x3013ff).ram().w(FUNC(tecmosys_state::lineram_w<0>)).share("bg0_lineram");// bg0 linescroll? (guess)

	map(0x400000, 0x400fff).ram().w(FUNC(tecmosys_state::vram_w<2>)).share("vram_2"); // bg1 ram
	map(0x401000, 0x4013ff).ram().w(FUNC(tecmosys_state::lineram_w<1>)).share("bg1_lineram");// bg1 linescroll? (guess)

	map(0x500000, 0x500fff).ram().w(FUNC(tecmosys_state::vram_w<3>)).share("vram_3"); // bg2 ram
	map(0x501000, 0x5013ff).ram().w(FUNC(tecmosys_state::lineram_w<2>)).share("bg2_lineram"); // bg2 linescroll? (guess)

	map(0x700000, 0x703fff).ram().w(FUNC(tecmosys_state::vram_w<0>)).share("vram_0"); // fix ram
	map(0x800000, 0x80ffff).ram().share("spriteram"); // obj ram
	map(0x880000, 0x88000b).r(FUNC(tecmosys_state::unk880000_r));
	map(0x880000, 0x88002f).w(FUNC(tecmosys_state::unk880000_w)).share("880000regs");  // 10 byte dta@88000c, 880022=watchdog?
	map(0x900000, 0x907fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette"); // .writeonly(); // obj pal

	//map(0x980000, 0x9807ff).writeonly(); // bg pal
	//map(0x980800, 0x980fff).w(FUNC(tecmosys_state::paletteram_xGGGGGRRRRRBBBBB_word_w)).share("paletteram"); // fix pal
	// the two above are as tested by the game code, I've only rolled them into one below to get colours to show right.
	map(0x980000, 0x980fff).ram().w(FUNC(tecmosys_state::tilemap_paletteram16_xGGGGGRRRRRBBBBB_word_w)).share("tmap_palette");

	map(0xa00000, 0xa00001).w(FUNC(tecmosys_state::eeprom_w));
	map(0xa80000, 0xa80005).writeonly().share("scroll_2");    // a80000-3 scroll? a80004 inverted ? 3 : 0
	map(0xb00000, 0xb00005).writeonly().share("scroll_3");    // b00000-3 scrool?, b00004 inverted ? 3 : 0
	map(0xb80000, 0xb80001).rw(FUNC(tecmosys_state::prot_status_r), FUNC(tecmosys_state::prot_status_w));
	map(0xc00000, 0xc00005).writeonly().share("scroll_0");    // c00000-3 scroll? c00004 inverted ? 13 : 10
	map(0xc80000, 0xc80005).writeonly().share("scroll_1");    // c80000-3 scrool? c80004 inverted ? 3 : 0
	map(0xd00000, 0xd00001).portr("P1");
	map(0xd00002, 0xd00003).portr("P2");
	map(0xd80000, 0xd80001).r(FUNC(tecmosys_state::eeprom_r));
	map(0xe00001, 0xe00001).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xe80000, 0xe80001).w(FUNC(tecmosys_state::prot_data_w));
	map(0xf00001, 0xf00001).r(FUNC(tecmosys_state::sound_command_pending_r));
	map(0xf80000, 0xf80001).r(FUNC(tecmosys_state::prot_data_r));
}


void tecmosys_state::z80_bank_w(u8 data)
{
	m_audiobank->set_entry(data);
}

void tecmosys_state::oki_bank_w(u8 data)
{
	m_okibank[0]->set_entry((data & 0x03) >> 0);
	m_okibank[1]->set_entry((data & 0x30) >> 4);
}


void tecmosys_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("audiobank");
	map(0xe000, 0xf7ff).ram();
}

void tecmosys_state::oki_map(address_map &map)
{
	map(0x00000, 0x1ffff).bankr("okibank1");
	map(0x20000, 0x3ffff).bankr("okibank2");
}

void tecmosys_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ymf", FUNC(ymf262_device::read), FUNC(ymf262_device::write));
	map(0x10, 0x10).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x20, 0x20).w(FUNC(tecmosys_state::oki_bank_w));
	map(0x30, 0x30).w(FUNC(tecmosys_state::z80_bank_w));
	map(0x40, 0x40).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x50, 0x50).w(FUNC(tecmosys_state::sound_nmi_disable_w));
	map(0x60, 0x61).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write));
}


static INPUT_PORTS_START( tecmosys )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )      PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )    PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )    PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )   PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )          PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )          PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )          PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 )          PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )      PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )   PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )          PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )          PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )          PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 )          PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


const gfx_layout gfx_sprite =
{
	16,1,
	RGN_FRAC(1,1),
	8,
	{ STEP4(0,1), STEP4(16,1) },
	{ STEP4(0*16*2,4), STEP4(1*16*2,4), STEP4(2*16*2,4), STEP4(3*16*2,4) },
	{ 0 },
	8*16*1
};

static GFXDECODE_START( gfx_tecmosys )
	GFXDECODE_ENTRY( "layer0",  0, gfx_8x8x4_packed_msb,               0x4400, 0x40 )
	GFXDECODE_ENTRY( "layer1",  0, gfx_8x8x4_row_2x2_group_packed_msb, 0x4000, 0x40 )
	GFXDECODE_ENTRY( "layer2",  0, gfx_8x8x4_row_2x2_group_packed_msb, 0x4000, 0x40 )
	GFXDECODE_ENTRY( "layer3",  0, gfx_8x8x4_row_2x2_group_packed_msb, 0x4000, 0x40 )
	GFXDECODE_ENTRY( "sprites", 0, gfx_sprite,                         0x0000, 0x40 )
GFXDECODE_END


void tecmosys_state::machine_start()
{
	m_audiobank->configure_entries(0, 16, memregion("audiocpu")->base(), 0x4000);
	for (int bank = 0; bank < 2; bank++)
		m_okibank[bank]->configure_entries(0, 4, memregion("oki")->base(), 0x20000);

	save_item(NAME(m_device_read_ptr));
	save_item(NAME(m_device_status));
	save_item(NAME(m_device_value));
}

void tecmosys_state::tecmosys(machine_config &config)
{
	M68000(config, m_maincpu, XTAL(16'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &tecmosys_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(tecmosys_state::irq1_line_hold));

	WATCHDOG_TIMER(config, m_watchdog).set_vblank_count(m_screen, 400); // guess

	Z80(config, m_audiocpu, XTAL(16'000'000)/2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &tecmosys_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &tecmosys_state::io_map);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tecmosys);

	EEPROM_93C46_16BIT(config, m_eeprom, eeprom_serial_streaming::ENABLE);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_AFTER_VBLANK);
	m_screen->set_refresh_hz(57.4458);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(3000));
	m_screen->set_size(64*8, 64*8);
	m_screen->set_visarea(0*8, 40*8-1, 0*8, 30*8-1);
	m_screen->set_screen_update(FUNC(tecmosys_state::screen_update));

	PALETTE(config, m_palette).set_format(palette_device::xGRB_555, 0x4000+0x800);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set("soundnmi", FUNC(input_merger_device::in_w<0>));

	INPUT_MERGER_ALL_HIGH(config, m_soundnmi);
	m_soundnmi->output_handler().set_inputline("audiocpu", INPUT_LINE_NMI);

	ymf262_device &ymf(YMF262(config, "ymf", XTAL(14'318'181)));
	ymf.irq_handler().set_inputline("audiocpu", 0);
	ymf.add_route(0, "speaker", 0.50, 0);
	ymf.add_route(1, "speaker", 0.50, 1);
	ymf.add_route(2, "speaker", 0.50, 0);
	ymf.add_route(3, "speaker", 0.50, 1);

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(16'000'000)/8, okim6295_device::PIN7_HIGH));
	oki.add_route(ALL_OUTPUTS, "speaker", 0.25, 0);
	oki.add_route(ALL_OUTPUTS, "speaker", 0.25, 1);
	oki.set_addrmap(0, &tecmosys_state::oki_map);

	ymz280b_device &ymz(YMZ280B(config, "ymz", XTAL(16'934'400)));
	ymz.add_route(0, "speaker", 0.30, 0);
	ymz.add_route(1, "speaker", 0.30, 1);
}

ROM_START( deroon )
	ROM_REGION( 0x100000, "maincpu", 0 ) // Main Program
	ROM_LOAD16_BYTE( "t001.upau1", 0x00000, 0x80000, CRC(14b92c18) SHA1(b47b8c828222a3f7c0fe9271899bd38171d972fb) )
	ROM_LOAD16_BYTE( "t002.upal1", 0x00001, 0x80000, CRC(0fb05c68) SHA1(5140592e15414770fb46d5ac9ba8f76e3d4ab323) )

	ROM_REGION( 0x040000, "audiocpu", 0 ) // Sound Program
	ROM_LOAD( "t003.uz1", 0x000000, 0x040000, CRC(8bdfafa0) SHA1(c0cf3eb7a65d967958fe2aace171859b0faf7753) )

	ROM_REGION( 0x2200, "cpu2", 0 ) // MCU is a 68HC11A8 with 8k ROM, 512 bytes EEPROM
	ROM_LOAD( "deroon_68hc11a8.rom",    0x0000, 0x2000, NO_DUMP )
	ROM_LOAD( "deroon_68hc11a8.eeprom", 0x2000, 0x0200, NO_DUMP )

	ROM_REGION( 0x2000000, "sprites", ROMREGION_ERASE00 ) // Sprites (non-tile based)
	/* all these roms need verifying, they could be half size */

	ROM_LOAD32_WORD_SWAP( "t101.uah1", 0x0000000, 0x200000, CRC(74baf845) SHA1(935d2954ba227a894542be492654a2750198e1bc) )
	ROM_LOAD32_WORD_SWAP( "t102.ual1", 0x0000002, 0x200000, CRC(1a02c4a3) SHA1(5155eeaef009fc9a9f258e3e54ca2a7f78242df5) )
	/*                                 0x8000000, 0x400000 - no rom loaded here, these gfx are 4bpp */
	ROM_LOAD32_WORD_SWAP( "t103.ubl1", 0x0800002, 0x400000, CRC(84e7da88) SHA1(b5c3234f33bb945cc9762b91db087153a0589cfb) )
	/*                                 0x1000000, 0x400000 - no rom loaded here, these gfx are 4bpp */
	ROM_LOAD32_WORD_SWAP( "t104.ucl1", 0x1000002, 0x200000, CRC(66eb611a) SHA1(64435d35677fea3c06fdb03c670f3f63ee481c02) )

	ROM_REGION( 0x100000, "layer0", 0 ) // 8x8 4bpp tiles
	ROM_LOAD( "t301.ubd1", 0x000000, 0x100000, CRC(8b026177) SHA1(3887856bdaec4d9d3669fe3bc958ef186fbe9adb) )

	ROM_REGION( 0x100000, "layer1", ROMREGION_ERASE00) // 16x16 4bpp tiles
	/* not used? */

	ROM_REGION( 0x100000, "layer2", ROMREGION_ERASE00 ) // 16x16 4bpp tiles
	ROM_LOAD( "t201.ubb1", 0x000000, 0x100000, CRC(d5a087ac) SHA1(5098160ce7719d93e3edae05f6edd317d4c61f0d) )

	ROM_REGION( 0x100000, "layer3", ROMREGION_ERASE00 ) // 16x16 4bpp tiles
	ROM_LOAD( "t202.ubc1", 0x000000, 0x100000, CRC(f051dae1) SHA1(f5677c07fe644b3838657370f0309fb09244c619) )

	ROM_REGION( 0x200000, "ymz", 0 ) // YMZ280B Samples
	ROM_LOAD( "t401.uya1", 0x000000, 0x200000, CRC(92111992) SHA1(ae27e11ae76dec0b9892ad32e1a8bf6ab11f2e6c) )

	ROM_REGION( 0x80000, "oki", 0 ) // M6295 Samples
	ROM_LOAD( "t501.uad1", 0x000000, 0x080000, CRC(2fbcfe27) SHA1(f25c830322423f0959a36955edb563a6150f2142) )
ROM_END

ROM_START( deroona ) // only main CPU program ROMs differ slightly from the deroon set (a routine is NOPed out at 0x711e-0x7152). Labels are original Tecmo. Low serial number.
	ROM_REGION( 0x100000, "maincpu", 0 ) // Main Program
	ROM_LOAD16_BYTE( "t01.upau1", 0x00000, 0x80000, CRC(7ad6c740) SHA1(51be70edf30364919c0d1da4a9bdd16bfbace213) )
	ROM_LOAD16_BYTE( "t02.upal1", 0x00001, 0x80000, CRC(e44f4430) SHA1(8a4ff12ef3a238694c30516fd7f39ccb4625dae9) )

	ROM_REGION( 0x040000, "audiocpu", 0 ) // Sound Program
	ROM_LOAD( "t003.uz1", 0x000000, 0x040000, CRC(8bdfafa0) SHA1(c0cf3eb7a65d967958fe2aace171859b0faf7753) )

	ROM_REGION( 0x2200, "cpu2", 0 ) // MCU is a 68HC11A8 with 8k ROM, 512 bytes EEPROM
	ROM_LOAD( "deroon_68hc11a8.rom",    0x0000, 0x2000, NO_DUMP )
	ROM_LOAD( "deroon_68hc11a8.eeprom", 0x2000, 0x0200, NO_DUMP )

	ROM_REGION( 0x2000000, "sprites", ROMREGION_ERASE00 ) // Sprites (non-tile based)
	/* all these roms need verifying, they could be half size */

	ROM_LOAD32_WORD_SWAP( "t101.uah1", 0x0000000, 0x200000, CRC(74baf845) SHA1(935d2954ba227a894542be492654a2750198e1bc) )
	ROM_LOAD32_WORD_SWAP( "t102.ual1", 0x0000002, 0x200000, CRC(1a02c4a3) SHA1(5155eeaef009fc9a9f258e3e54ca2a7f78242df5) )
	/*                                 0x8000000, 0x400000 - no rom loaded here, these gfx are 4bpp */
	ROM_LOAD32_WORD_SWAP( "t103.ubl1", 0x0800002, 0x400000, CRC(84e7da88) SHA1(b5c3234f33bb945cc9762b91db087153a0589cfb) )
	/*                                 0x1000000, 0x400000 - no rom loaded here, these gfx are 4bpp */
	ROM_LOAD32_WORD_SWAP( "t104.ucl1", 0x1000002, 0x200000, CRC(66eb611a) SHA1(64435d35677fea3c06fdb03c670f3f63ee481c02) )

	ROM_REGION( 0x100000, "layer0", 0 ) // 8x8 4bpp tiles
	ROM_LOAD( "t301.ubd1", 0x000000, 0x100000, CRC(8b026177) SHA1(3887856bdaec4d9d3669fe3bc958ef186fbe9adb) )

	ROM_REGION( 0x100000, "layer1", ROMREGION_ERASE00) // 16x16 4bpp tiles
	/* not used? */

	ROM_REGION( 0x100000, "layer2", ROMREGION_ERASE00 ) // 16x16 4bpp tiles
	ROM_LOAD( "t201.ubb1", 0x000000, 0x100000, CRC(d5a087ac) SHA1(5098160ce7719d93e3edae05f6edd317d4c61f0d) )

	ROM_REGION( 0x100000, "layer3", ROMREGION_ERASE00 ) // 16x16 4bpp tiles
	ROM_LOAD( "t202.ubc1", 0x000000, 0x100000, CRC(f051dae1) SHA1(f5677c07fe644b3838657370f0309fb09244c619) )

	ROM_REGION( 0x200000, "ymz", 0 ) // YMZ280B Samples
	ROM_LOAD( "t401.uya1", 0x000000, 0x200000, CRC(92111992) SHA1(ae27e11ae76dec0b9892ad32e1a8bf6ab11f2e6c) )

	ROM_REGION( 0x80000, "oki", 0 ) // M6295 Samples
	ROM_LOAD( "t501.uad1", 0x000000, 0x080000, CRC(2fbcfe27) SHA1(f25c830322423f0959a36955edb563a6150f2142) )
ROM_END

/*
About the Deroon DeroDero listed below:

 This set contains less Japanese text and English translations for some game aspects such as game menus.
 Coining up displays "TECMO STACKERS" but this set doesn't seem to include a "How to Play" demo like the parent set

 PCB contained genuine Tecmo development labels, it's unknown which specific region this set was intended for.
 Still missing the full English version titled Temco Stackers if it exists.

*/
ROM_START( deroon2 )
	ROM_REGION( 0x100000, "maincpu", 0 ) // Main Program
	ROM_LOAD16_BYTE( "stk_t01.upau1", 0x00000, 0x80000, CRC(90c794df) SHA1(b6edd62bedf609551f4e1c19ada20bd1373deca2) )
	ROM_LOAD16_BYTE( "stk_t02.upal1", 0x00001, 0x80000, CRC(cca9f87c) SHA1(0637b0b979f4c6c6b16cf2a21dd193b7d7ec311f) )

	ROM_REGION( 0x040000, "audiocpu", 0 ) // Sound Program
	ROM_LOAD( "t003.uz1", 0x000000, 0x040000, CRC(8bdfafa0) SHA1(c0cf3eb7a65d967958fe2aace171859b0faf7753) )

	ROM_REGION( 0x2200, "cpu2", 0 ) // MCU is a 68HC11A8 with 8k ROM, 512 bytes EEPROM
	ROM_LOAD( "deroon_68hc11a8.rom",    0x0000, 0x2000, NO_DUMP )
	ROM_LOAD( "deroon_68hc11a8.eeprom", 0x2000, 0x0200, NO_DUMP )

	ROM_REGION( 0x2000000, "sprites", ROMREGION_ERASE00 ) // Sprites (non-tile based)
	ROM_LOAD32_WORD_SWAP( "t101.uah1", 0x0000000, 0x200000, CRC(74baf845) SHA1(935d2954ba227a894542be492654a2750198e1bc) )
	ROM_LOAD32_WORD_SWAP( "t102.ual1", 0x0000002, 0x200000, CRC(1a02c4a3) SHA1(5155eeaef009fc9a9f258e3e54ca2a7f78242df5) )
	/*                                 0x8000000, 0x400000 - no rom loaded here, these gfx are 4bpp */
	ROM_LOAD32_WORD_SWAP( "t103.ubl1", 0x0800002, 0x400000, CRC(84e7da88) SHA1(b5c3234f33bb945cc9762b91db087153a0589cfb) )
	/*                                 0x1000000, 0x400000 - no rom loaded here, these gfx are 4bpp */
	ROM_LOAD32_WORD_SWAP( "t104.ucl1", 0x1000002, 0x200000, CRC(66eb611a) SHA1(64435d35677fea3c06fdb03c670f3f63ee481c02) )

	ROM_REGION( 0x100000, "layer0", 0 ) // 8x8 4bpp tiles
	ROM_LOAD( "t301.ubd1", 0x000000, 0x100000, CRC(8b026177) SHA1(3887856bdaec4d9d3669fe3bc958ef186fbe9adb) )

	ROM_REGION( 0x100000, "layer1", ROMREGION_ERASE00) // 16x16 4bpp tiles
	/* not used? */

	ROM_REGION( 0x100000, "layer2", ROMREGION_ERASE00 ) // 16x16 4bpp tiles
	ROM_LOAD( "t201.ubb1", 0x000000, 0x100000, CRC(d5a087ac) SHA1(5098160ce7719d93e3edae05f6edd317d4c61f0d) )

	ROM_REGION( 0x100000, "layer3", ROMREGION_ERASE00 ) // 16x16 4bpp tiles
	ROM_LOAD( "t202.ubc1", 0x000000, 0x100000, CRC(f051dae1) SHA1(f5677c07fe644b3838657370f0309fb09244c619) )

	ROM_REGION( 0x200000, "ymz", 0 ) // YMZ280B Samples
	ROM_LOAD( "t401.uya1", 0x000000, 0x200000, CRC(92111992) SHA1(ae27e11ae76dec0b9892ad32e1a8bf6ab11f2e6c) )

	ROM_REGION( 0x80000, "oki", 0 ) // M6295 Samples
	ROM_LOAD( "t501.uad1", 0x000000, 0x080000, CRC(2fbcfe27) SHA1(f25c830322423f0959a36955edb563a6150f2142) )
ROM_END

ROM_START( tkdensho )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "aeprge-2.pal", 0x00000, 0x80000, CRC(25e453d6) SHA1(9c84e2af42eff5cc9b14c1759d5bab42fa7bb663) )
	ROM_LOAD16_BYTE( "aeprgo-2.pau", 0x00001, 0x80000, CRC(22d59510) SHA1(5ade482d6ab9a22df2ee8337458c22cfa9045c73) )

	ROM_REGION( 0x040000, "audiocpu", 0 ) // Sound Program
	ROM_LOAD( "aesprg-2.z1", 0x000000, 0x020000, CRC(43550ab6) SHA1(2580129ef8ebd9295249175de4ba985c752e06fe) )
	ROM_RELOAD(              0x020000, 0x020000) // for banks

	ROM_REGION( 0x2200, "cpu2", 0 ) // MCU is a 68HC11A8 with 8k ROM, 512 bytes EEPROM
	ROM_LOAD( "tkdensho_68hc11a8.rom",    0x0000, 0x2000, NO_DUMP )
	ROM_LOAD( "tkdensho_68hc11a8.eeprom", 0x2000, 0x0200, NO_DUMP )

	ROM_REGION( 0x4000000, "sprites", ROMREGION_ERASE00 ) // Graphics - not tile based
	ROM_LOAD32_WORD_SWAP( "ae100h.ah1",    0x0000000, 0x0400000, CRC(06be252b) SHA1(08d1bb569fd2e66e2c2f47da7780b31945232e62) )
	ROM_LOAD32_WORD_SWAP( "ae100.al1",     0x0000002, 0x0400000, CRC(009cdff4) SHA1(fd88f07313d14fd4429b09a1e8d6b595df3b98e5) )
	ROM_LOAD32_WORD_SWAP( "ae101h.bh1",    0x0800000, 0x0400000, CRC(f2469eff) SHA1(ba49d15cc7949437ba9f56d9b425a5f0e62137df) )
	ROM_LOAD32_WORD_SWAP( "ae101.bl1",     0x0800002, 0x0400000, CRC(db7791bb) SHA1(1fe40b747b7cee7a9200683192b1d60a735a0446) )
	ROM_LOAD32_WORD_SWAP( "ae102h.ch1",    0x1000000, 0x0200000, CRC(f9d2a343) SHA1(d141ac0b20be587e77a576ef78f15d269d9c84e5) )
	ROM_LOAD32_WORD_SWAP( "ae102.cl1",     0x1000002, 0x0200000, CRC(681be889) SHA1(8044ca7cbb325e6dcadb409f91e0c01b88a1bca7) )
	ROM_LOAD32_WORD_SWAP( "ae104.el1",     0x2000002, 0x0400000, CRC(e431b798) SHA1(c2c24d4f395bba8c78a45ecf44009a830551e856) )
	ROM_LOAD32_WORD_SWAP( "ae105.fl1",     0x2800002, 0x0400000, CRC(b7f9ebc1) SHA1(987f664072b43a578b39fa6132aaaccc5fe5bfc2) )
	ROM_LOAD32_WORD_SWAP( "ae106.gl1",     0x3000002, 0x0200000, CRC(7c50374b) SHA1(40865913125230122072bb13f46fb5fb60c088ea) )

	ROM_REGION( 0x080000, "layer0", 0 ) // 8x8 4bpp tiles
	ROM_LOAD( "ae300w36.bd1",  0x000000, 0x0080000, CRC(e829f29e) SHA1(e56bfe2669ed1d1ae394c644def426db129d97e3) )

	ROM_REGION( 0x100000, "layer1", 0 ) // 16x16 4bpp tiles
	ROM_LOAD( "ae200w74.ba1",  0x000000, 0x0100000, CRC(c1645041) SHA1(323670a6aa2a4524eb968cc0b4d688098ffeeb12) )

	ROM_REGION( 0x100000, "layer2", 0 ) // 16x16 4bpp tiles
	ROM_LOAD( "ae201w75.bb1",  0x000000, 0x0100000, CRC(3f63bdff) SHA1(0d3d57fdc0ec4bceef27c11403b3631d23abadbf) )

	ROM_REGION( 0x100000, "layer3", 0 ) // 16x16 4bpp tiles
	ROM_LOAD( "ae202w76.bc1",  0x000000, 0x0100000, CRC(5cc857ca) SHA1(2553fb5220433acc15dfb726dc064fe333e51d88) )

	ROM_REGION( 0x400000, "ymz", 0 ) // YMZ280B Samples
	ROM_LOAD( "ae400t23.ya1", 0x000000, 0x200000, CRC(c6ffb043) SHA1(e0c6c5f6b840f63c9a685a2c3be66efa4935cbeb) )
	ROM_LOAD( "ae401t24.yb1", 0x200000, 0x200000, CRC(d83f1a73) SHA1(412b7ac9ff09a984c28b7d195330d78c4aac3dc5) )

	ROM_REGION( 0x80000, "oki", 0 ) // M6295 Samples
	ROM_LOAD( "ae500w07.ad1", 0x000000, 0x080000, CRC(3734f92c) SHA1(048555b5aa89eaf983305c439ba08d32b4a1bb80) )
ROM_END

ROM_START( tkdenshoa )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "aeprge.pal", 0x00000, 0x80000, CRC(17a209ff) SHA1(b5dbea9868cbb89d4e27bf19fdb616ac256985b4) )
	ROM_LOAD16_BYTE( "aeprgo.pau", 0x00001, 0x80000, CRC(d265e6a1) SHA1(f39d8ce115f197a660f5210b2483108854eb12a9) )

	ROM_REGION( 0x040000, "audiocpu", 0 ) // Sound Program
	ROM_LOAD( "aesprg-2.z1", 0x000000, 0x020000, CRC(43550ab6) SHA1(2580129ef8ebd9295249175de4ba985c752e06fe) )
	ROM_RELOAD(              0x020000, 0x020000) // for banks

	ROM_REGION( 0x2200, "cpu2", 0 ) // MCU is a 68HC11A8 with 8k ROM, 512 bytes EEPROM
	ROM_LOAD( "tkdensho_68hc11a8.rom",    0x0000, 0x2000, NO_DUMP )
	ROM_LOAD( "tkdensho_68hc11a8.eeprom", 0x2000, 0x0200, NO_DUMP )

	ROM_REGION( 0x4000000, "sprites", ROMREGION_ERASE00 ) // Graphics - not tile based
	ROM_LOAD32_WORD_SWAP( "ae100h.ah1",    0x0000000, 0x0400000, CRC(06be252b) SHA1(08d1bb569fd2e66e2c2f47da7780b31945232e62) )
	ROM_LOAD32_WORD_SWAP( "ae100.al1",     0x0000002, 0x0400000, CRC(009cdff4) SHA1(fd88f07313d14fd4429b09a1e8d6b595df3b98e5) )
	ROM_LOAD32_WORD_SWAP( "ae101h.bh1",    0x0800000, 0x0400000, CRC(f2469eff) SHA1(ba49d15cc7949437ba9f56d9b425a5f0e62137df) )
	ROM_LOAD32_WORD_SWAP( "ae101.bl1",     0x0800002, 0x0400000, CRC(db7791bb) SHA1(1fe40b747b7cee7a9200683192b1d60a735a0446) )
	ROM_LOAD32_WORD_SWAP( "ae102h.ch1",    0x1000000, 0x0200000, CRC(f9d2a343) SHA1(d141ac0b20be587e77a576ef78f15d269d9c84e5) )
	ROM_LOAD32_WORD_SWAP( "ae102.cl1",     0x1000002, 0x0200000, CRC(681be889) SHA1(8044ca7cbb325e6dcadb409f91e0c01b88a1bca7) )
	ROM_LOAD32_WORD_SWAP( "ae104.el1",     0x2000002, 0x0400000, CRC(e431b798) SHA1(c2c24d4f395bba8c78a45ecf44009a830551e856) )
	ROM_LOAD32_WORD_SWAP( "ae105.fl1",     0x2800002, 0x0400000, CRC(b7f9ebc1) SHA1(987f664072b43a578b39fa6132aaaccc5fe5bfc2) )
	ROM_LOAD32_WORD_SWAP( "ae106.gl1",     0x3000002, 0x0200000, CRC(7c50374b) SHA1(40865913125230122072bb13f46fb5fb60c088ea) )

	ROM_REGION( 0x080000, "layer0", 0 ) // 8x8 4bpp tiles
	ROM_LOAD( "ae300w36.bd1",  0x000000, 0x0080000, CRC(e829f29e) SHA1(e56bfe2669ed1d1ae394c644def426db129d97e3) )

	ROM_REGION( 0x100000, "layer1", 0 ) // 16x16 4bpp tiles
	ROM_LOAD( "ae200w74.ba1",  0x000000, 0x0100000, CRC(c1645041) SHA1(323670a6aa2a4524eb968cc0b4d688098ffeeb12) )

	ROM_REGION( 0x100000, "layer2", 0 ) // 16x16 4bpp tiles
	ROM_LOAD( "ae201w75.bb1",  0x000000, 0x0100000, CRC(3f63bdff) SHA1(0d3d57fdc0ec4bceef27c11403b3631d23abadbf) )

	ROM_REGION( 0x100000, "layer3", 0 ) // 16x16 4bpp tiles
	ROM_LOAD( "ae202w76.bc1",  0x000000, 0x0100000, CRC(5cc857ca) SHA1(2553fb5220433acc15dfb726dc064fe333e51d88) )

	ROM_REGION( 0x400000, "ymz", 0 ) // YMZ280B Samples
	ROM_LOAD( "ae400t23.ya1", 0x000000, 0x200000, CRC(c6ffb043) SHA1(e0c6c5f6b840f63c9a685a2c3be66efa4935cbeb) )
	ROM_LOAD( "ae401t24.yb1", 0x200000, 0x200000, CRC(d83f1a73) SHA1(412b7ac9ff09a984c28b7d195330d78c4aac3dc5) )

	ROM_REGION( 0x80000, "oki", 0 ) // M6295 Samples
	ROM_LOAD( "ae500w07.ad1", 0x000000, 0x080000, CRC(3734f92c) SHA1(048555b5aa89eaf983305c439ba08d32b4a1bb80) )
ROM_END

void tecmosys_state::descramble()
{
	m_sprite_gfx_mask = 1;
	while (m_sprite_gfx_mask < m_sprite_region.length())
	{
		m_sprite_gfx_mask <<= 1;
	}
	m_sprite_gfx = make_unique_clear<u8[]>(m_sprite_gfx_mask);

	gfx_element *gfx = m_gfxdecode->gfx(4);
	u8 *dst = m_sprite_gfx.get();
	for (int e = 0; e < gfx->elements(); e++)
	{
		const u8 *data = gfx->get_data(e);
		for (int y = 0; y < gfx->height(); y++)
		{
			const u8 *datatmp = data;
			for (int x = 0; x < gfx->width(); x++)
			{
				*dst++ = *datatmp++;
			}
			data += gfx->rowbytes();
		}
	}
	m_sprite_gfx_mask--;
}

void tecmosys_state::init_deroon()
{
	descramble();
	prot_init(0); // tecmo/tecmosys_m.cpp
}

void tecmosys_state::init_tkdensho()
{
	descramble();
	prot_init(1);
}

void tecmosys_state::init_tkdensha()
{
	descramble();
	prot_init(2);
}

GAME( 1995, deroon,    0,        tecmosys, tecmosys, tecmosys_state, init_deroon,     ROT0, "Tecmo", "Deroon DeroDero (earlier, set 1)",        MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1996, deroona,   deroon,   tecmosys, tecmosys, tecmosys_state, init_deroon,     ROT0, "Tecmo", "Deroon DeroDero (earlier, set 2)",        MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1996, deroon2,   deroon,   tecmosys, tecmosys, tecmosys_state, init_deroon,     ROT0, "Tecmo", "Deroon DeroDero / Tecmo Stackers",        MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // some parts translated in English, attract without 'how to play'
GAME( 1996, tkdensho,  0,        tecmosys, tecmosys, tecmosys_state, init_tkdensho,   ROT0, "Tecmo", "Toukidenshou - Angel Eyes (VER. 960614)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1996, tkdenshoa, tkdensho, tecmosys, tecmosys, tecmosys_state, init_tkdensha,   ROT0, "Tecmo", "Toukidenshou - Angel Eyes (VER. 960427)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
