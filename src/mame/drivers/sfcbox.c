/***************************************************************************

  sfcbox.c
  Preliminary driver file to handle emulation of the
  Nintendo Super Famicom Box.

The Super Famicom Box was used in hotels in Japan, with its installed Coin
Box, makes it a pay-for-play system.  It accepted 100 yen coins and gave 5
minutes of playtime.  The interesting part of this system is that it used
special multicarts that contained two or three games as part of a larger Cart,
for which the system holds two at once.

The "To Do" list:
-----------------
-Consider moving the 3 cartridges of the slot 2 in a sotwware list since they are interchangable
-Hook the z180 clone, the DSP 1A/1B and the Super FX
-Add the missing GROM4-1
-Add the possibly alternate revision of the attract ROM, with Kirby holding a coin
 (unless it is unlocked with some DIP switch)

Specific Model Number information
---------------------------------

Hardware:
                                    _
                                   | |--------------------------------------| GS 0871-102
                                   | |--------------------------------------| GS 0871-102
                       GD 0871-103 ||
                                   ||   |-----------------------------------| PU 0871-101
                                   ||     |_|
 |---------------------------------||-----|_|-------------------------------| MAIN 0871-100A
    __________________________________________
   |------------------------------------------|    GS 0871-102
   |------------------------------------------|    GS 0871-102
  ___|__________________________            |
 |------------------------------|           |      PU 0871-101
 _|______________________|__________________|____
|------------------------------------------------| MAIN 0871-100A


PSS-001 - SUPER FAMICOM BOX - Main Unit
This unit contains the three boards:
the main board, the BIOS board and a passive board.

1. The main board (MAIN 0871-100A) contains the Super Famicom core hardware as follow:
CPU: S-CPU B (5A22-02 3MB 83)
Video Controller: S-PPU1 (5C77-01 4AU 9L)
Video Controller: S-PPU2 C (5C78-03 3MB 9V)
Working CPU RAM: S-WRAM A (9442 T94 F)
Sound CPU: S-SMP (SONY Nintendo'89 JAPAN 3WK4V)
Sound DSP: S-DSP A (SONY'89 347AB7VZ)
D/A converter: NEC D6376 - PDIF output model can be modified
Slave (?) CPU: Hitachi 3M3R1 HD64180RF6X (Z180 clone)
S-ENC A (9504 BA)
MB90082 001 (9351 M02)

2. The BIOS board (PU 0871-101) must be inserted into the main board.
BIOS ROM: KROM 1, 512Kibit
SRAM: SRM20257LM12 F27K 256 (S-MOS Systems) - SRAM accounting and control circuits are
      self-diagnostic features that set time and operational status of the
      game(s) that are installed.
Battery: C2032
Decoders: 74HC139A, 74HC237D, 74LS641, RTC S-3520CF (Seiko S3520CF2 C4446 J18)
          and two other unreadable chips
MB3790 (9413 M32)

3. The passive board (GD 0871-103) to be inserted into the main board, has two cartridge slots.

PSS-002 - SUPER FAMICOM BOX - Cartridge

PSS-003 - SUPER FAMICOM BOX - Coin Box

Software/Cartridge:
The PSS-61 cartridge is required on the slot 1 for the machine to operate.
The slot 2 may be free or contain PSS-62, PSS-63 or PSS-64 interchangably.

PSS-61  - SUPER FAMICOM BOX Commercial Regular Cart
WARNING: This cartridge is required for the machine to operate.
This game board (GS 0871-102) contains various chips in addition to the game ROMs:
Attraction ROM: ATROM-4S-0, 4Mibit (is called Slave ProgramROM in BIOS menu ?)
                Menu to select one of the 3 games (or 5, if a cartridge is inserted in slot 2)
GameData ROM: GROM1-1, 256Kibit, most likely contains the graphics used by the attract menu
              May also contain the extra text layers added to the game graphics
Upper ROM: Super Mario Kart ROM (Nintendo), SHVC-MK-0, 4Mibit
Upper ROM: Super Mario Collection ROM (Nintendo), SHVC-4M-1, 16Mibit
Upper ROM: Star Fox ROM (Nintendo), SHVC-FO-1, 8Mibit
DSP 1 B coprocessor: (DSP 1 A in earlier SFBOX units) it is needed to operate Super Mario Kart
MARIO CHIP 1 coprocessor: Also known as "Super FX", it is needed to operate Star Fox
Static RAM: 1Mibit
Static RAM: 256 Kibit

PSS-62  - SUPER FAMICOM BOX Commercial Optional Cart
GameData ROM: GROM2-1, 256Kibit
Lower ROM: New Super 3D Golf Simulation - Waialae No Kiseki (Waialae Golf) (T&E SOFT), SHVC-GC-0, 4Mibit
Lower ROM: Super Mahjong 2 (I'MAX), SHVC-2A-1, 8Mibit

PSS-63  - SUPER FAMICOM BOX Commercial Optional Cart
GameData ROM: GROM3-1, 256Kibit
Lower ROM: Super Donkey Kong (Nintendo), SHVC-8X-1, 32Mibit
Lower ROM: Super Tetris 2 + Bombliss (BPS), SHVC-T2-1, 8Mibit

PSS-64  - SUPER FAMICOM BOX Commercial Optional Cart
GameData ROM: GROM4-1, undumped
Lower ROM: Super Donkey Kong (Nintendo), SHVC-8X-1, 32Mibit
Lower ROM: Super Bomberman 2 (Hudson Soft), SHVC-M4-0, 8Mibit

How does the Super Famicom Box operates
---------------------------------------
-Operate with a key, goes into BIOS, does automated checks, PSS-61 must be inserted for system to operate.
-Goes into the attraction ROM, there is a graphical menu to select a game.
 Apparently the menu graphics are imported from the GROMs.
-Goes into selected game, can go back to attraction ROM menu.

***************************************************************************/

#include "emu.h"
#include "cpu/spc700/spc700.h"
#include "cpu/g65816/g65816.h"
#include "cpu/z180/z180.h"
#include "video/mb90092.h"
#include "includes/snes.h"
#include "audio/snes_snd.h"
#include "rendlay.h"

class sfcbox_state : public snes_state
{
public:
	sfcbox_state(const machine_config &mconfig, device_type type, const char *tag)
		: snes_state(mconfig, type, tag),
		m_bios(*this, "bios"),
		m_mb90092(*this,"mb90092")
		{ }

	required_device <cpu_device> m_bios;
	required_device <mb90092_device> m_mb90092;

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE8_MEMBER( port_81_w );
};

UINT32 sfcbox_state::screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	m_mb90092->screen_update(screen,bitmap,cliprect);
	return 0;
}

static ADDRESS_MAP_START( snes_map, AS_PROGRAM, 8, sfcbox_state )
	AM_RANGE(0x000000, 0x2fffff) AM_READWRITE_LEGACY(snes_r_bank1, snes_w_bank1)	/* I/O and ROM (repeats for each bank) */
	AM_RANGE(0x300000, 0x3fffff) AM_READWRITE_LEGACY(snes_r_bank2, snes_w_bank2)	/* I/O and ROM (repeats for each bank) */
	AM_RANGE(0x400000, 0x5fffff) AM_READ_LEGACY(snes_r_bank3)						/* ROM (and reserved in Mode 20) */
	AM_RANGE(0x600000, 0x6fffff) AM_READWRITE_LEGACY(snes_r_bank4, snes_w_bank4)	/* used by Mode 20 DSP-1 */
	AM_RANGE(0x700000, 0x7dffff) AM_READWRITE_LEGACY(snes_r_bank5, snes_w_bank5)
	AM_RANGE(0x7e0000, 0x7fffff) AM_RAM					/* 8KB Low RAM, 24KB High RAM, 96KB Expanded RAM */
	AM_RANGE(0x800000, 0xbfffff) AM_READWRITE_LEGACY(snes_r_bank6, snes_w_bank6)	/* Mirror and ROM */
	AM_RANGE(0xc00000, 0xffffff) AM_READWRITE_LEGACY(snes_r_bank7, snes_w_bank7)	/* Mirror and ROM */
ADDRESS_MAP_END

static READ8_DEVICE_HANDLER( spc_ram_100_r )
{
	return spc_ram_r(device, offset + 0x100);
}

static WRITE8_DEVICE_HANDLER( spc_ram_100_w )
{
	spc_ram_w(device, offset + 0x100, data);
}

static ADDRESS_MAP_START( spc_mem, AS_PROGRAM, 8, sfcbox_state )
	AM_RANGE(0x0000, 0x00ef) AM_DEVREADWRITE_LEGACY("spc700", spc_ram_r, spc_ram_w)	/* lower 32k ram */
	AM_RANGE(0x00f0, 0x00ff) AM_DEVREADWRITE_LEGACY("spc700", spc_io_r, spc_io_w)	/* spc io */
	AM_RANGE(0x0100, 0xffff) AM_DEVWRITE_LEGACY("spc700", spc_ram_100_w)
	AM_RANGE(0x0100, 0xffbf) AM_DEVREAD_LEGACY("spc700", spc_ram_100_r)
	AM_RANGE(0xffc0, 0xffff) AM_DEVREAD_LEGACY("spc700", spc_ipl_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sfcbox_map, AS_PROGRAM, 8, sfcbox_state )
	AM_RANGE(0x00000, 0x0ffff) AM_ROM AM_REGION("krom", 0)
	AM_RANGE(0x20000, 0x27fff) AM_RAM
	AM_RANGE(0x40000, 0x47fff) AM_ROM AM_REGION("grom", 0)
	AM_RANGE(0x60000, 0x67fff) AM_NOP // grom slot 1
ADDRESS_MAP_END

WRITE8_MEMBER( sfcbox_state::port_81_w )
{
	device_set_input_line(m_maincpu, INPUT_LINE_RESET, (data & 1) ? CLEAR_LINE : ASSERT_LINE);
	device_set_input_line(m_soundcpu, INPUT_LINE_RESET, (data & 1) ? CLEAR_LINE : ASSERT_LINE);

	ioport("OSD_CS")->write(data, 0xff);
}

static ADDRESS_MAP_START( sfcbox_io, AS_IO, 8, sfcbox_state )
	AM_RANGE(0x0b, 0x0b) AM_DEVWRITE("mb90092",mb90092_device,write)
	AM_RANGE(0x00, 0x3f) AM_RAM // internal i/o
//	AM_RANGE(0x80, 0x80) // Keyswitch and Button Inputs / SNES Transfer and Misc Output
	AM_RANGE(0x81, 0x81) AM_WRITE(port_81_w) // SNES Transfer and Misc Input / Misc Output
//	AM_RANGE(0x82, 0x82) // Unknown/unused
//	AM_RANGE(0x83, 0x83) // Joypad Input/Status / Joypad Output/Control
//	AM_RANGE(0x84, 0x84) // Joypad 1, MSB (1st 8 bits) (eg. Bit7=ButtonB, 0=Low=Pressed)
// 	AM_RANGE(0x85, 0x85) // Joypad 1, LSB (2nd 8 bits) (eg. Bit0=LSB of ID, 0=Low=One)
//	AM_RANGE(0x86, 0x86) // Joypad 2, MSB (1st 8 bits) (eg. Bit7=ButtonB, 0=Low=Pressed)
//  AM_RANGE(0x87, 0x87) // Joypad 2, LSB (2nd 8 bits) (eg. Bit0=LSB of ID, 0=Low=One)
//	AM_RANGE(0xa0, 0xa0) //  Real Time Clock
//	AM_RANGE(0xc0, 0xc0) // SNES Mapping Register 0
//  AM_RANGE(0xc1, 0xc1) // SNES Mapping Register 1
ADDRESS_MAP_END


static INPUT_PORTS_START( snes )
	PORT_START("OSD_CS")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("mb90092", mb90092_device, set_cs_line)

	PORT_START("SERIAL1_DATA1_L")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Button A") PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P1 Button X") PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P1 Button L") PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P1 Button R") PORT_PLAYER(1)
	PORT_START("SERIAL1_DATA1_H")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Button B") PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Button Y") PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("P1 Select")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("P1 Start")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)

	PORT_START("SERIAL2_DATA1_L")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P2 Button A") PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P2 Button X") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P2 Button L") PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P2 Button R") PORT_PLAYER(2)
	PORT_START("SERIAL2_DATA1_H")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Button B") PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Button Y") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_NAME("P2 Select")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("P2 Start")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)

	PORT_START("SERIAL1_DATA2_L")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("SERIAL1_DATA2_H")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SERIAL2_DATA2_L")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("SERIAL2_DATA2_H")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hard )  )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x00, "Time limit per level?" ) // taken from the scan of nss_adam
	PORT_DIPSETTING(    0x10, "104 sec." )
	PORT_DIPSETTING(    0x20, "112 sec." )
	PORT_DIPSETTING(    0x00, "120 sec." )
	PORT_DIPSETTING(    0x30, "? sec." )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

#if SNES_LAYER_DEBUG
	PORT_START("DEBUG1")
	PORT_CONFNAME( 0x03, 0x00, "Select BG1 priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x01, "BG1B (lower) only" )
	PORT_CONFSETTING(    0x02, "BG1A (higher) only" )
	PORT_CONFNAME( 0x0c, 0x00, "Select BG2 priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x04, "BG2B (lower) only" )
	PORT_CONFSETTING(    0x08, "BG2A (higher) only" )
	PORT_CONFNAME( 0x30, 0x00, "Select BG3 priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x10, "BG3B (lower) only" )
	PORT_CONFSETTING(    0x20, "BG3A (higher) only" )
	PORT_CONFNAME( 0xc0, 0x00, "Select BG4 priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x40, "BG4B (lower) only" )
	PORT_CONFSETTING(    0x80, "BG4A (higher) only" )

	PORT_START("DEBUG2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle BG 1") PORT_CODE(KEYCODE_1_PAD) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle BG 2") PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle BG 3") PORT_CODE(KEYCODE_3_PAD) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle BG 4") PORT_CODE(KEYCODE_4_PAD) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Objects") PORT_CODE(KEYCODE_5_PAD) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Main/Sub") PORT_CODE(KEYCODE_6_PAD) PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Color Math") PORT_CODE(KEYCODE_7_PAD) PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Windows") PORT_CODE(KEYCODE_8_PAD) PORT_TOGGLE

	PORT_START("DEBUG3")
	PORT_BIT( 0x4, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mosaic") PORT_CODE(KEYCODE_9_PAD) PORT_TOGGLE
	PORT_CONFNAME( 0x70, 0x00, "Select OAM priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x10, "OAM0 only" )
	PORT_CONFSETTING(    0x20, "OAM1 only" )
	PORT_CONFSETTING(    0x30, "OAM2 only" )
	PORT_CONFSETTING(    0x40, "OAM3 only" )
	PORT_CONFNAME( 0x80, 0x00, "Draw sprite in reverse order" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DEBUG4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 0 draw") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 1 draw") PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 2 draw") PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 3 draw") PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 4 draw") PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 5 draw") PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 6 draw") PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 7 draw") PORT_TOGGLE
#endif
INPUT_PORTS_END

static MACHINE_CONFIG_START( snes, sfcbox_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", _5A22, 3580000*6)	/* 2.68Mhz, also 3.58Mhz */
	MCFG_CPU_PROGRAM_MAP(snes_map)

	MCFG_CPU_ADD("soundcpu", SPC700, 2048000/2)	/* 2.048 Mhz, but internal divider */
	MCFG_CPU_PROGRAM_MAP(spc_mem)

	MCFG_QUANTUM_TIME(attotime::from_hz(24000))

	MCFG_MACHINE_START( snes )
	MCFG_MACHINE_RESET( snes )

	/* video hardware */
	MCFG_VIDEO_START( snes )

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(DOTCLK_NTSC, SNES_HTOTAL, 0, SNES_SCR_WIDTH, SNES_VTOTAL_NTSC, 0, SNES_SCR_HEIGHT_NTSC)
	MCFG_SCREEN_UPDATE_DRIVER( snes_state, snes_screen_update )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("spc700", SNES, 0)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.00)
MACHINE_CONFIG_END


static MACHINE_START( sfcbox )
{
	sfcbox_state *state = machine.driver_data<sfcbox_state>();

	MACHINE_START_CALL(snes);

	state->m_is_sfcbox = 1;
}

static MACHINE_RESET( sfcbox )
{
	sfcbox_state *state = machine.driver_data<sfcbox_state>();

	MACHINE_RESET_CALL( snes );

	/* start with both CPUs disabled */
	device_set_input_line(state->m_maincpu, INPUT_LINE_RESET, ASSERT_LINE);
	device_set_input_line(state->m_soundcpu, INPUT_LINE_RESET, ASSERT_LINE);
}

static MACHINE_CONFIG_DERIVED( sfcbox, snes )

	MCFG_CPU_ADD("bios", Z180, XTAL_12MHz / 2)	/* HD64180RF6X */
	MCFG_CPU_PROGRAM_MAP(sfcbox_map)
	MCFG_CPU_IO_MAP(sfcbox_io)

	MCFG_MB90092_ADD("mb90092",XTAL_12MHz / 2) /* TODO: correct clock */

	MCFG_MACHINE_START( sfcbox )
	MCFG_MACHINE_RESET( sfcbox )

	/* TODO: the screen should actually superimpose, but for the time being let's just separate outputs */
	MCFG_DEFAULT_LAYOUT(layout_dualhsxs)

	MCFG_SCREEN_ADD("osd", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
//	MCFG_SCREEN_SIZE(24*12+22, 12*18+22)
//	MCFG_SCREEN_VISIBLE_AREA(0*8, 24*12-1, 0*8, 12*18-1)
	MCFG_SCREEN_SIZE(24*16+22, 12*16+22)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 24*16-1, 0*8, 12*16-1)
	MCFG_SCREEN_UPDATE_DRIVER(sfcbox_state,screen_update)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

#define SFCBOX_BIOS \
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 ) \
	ROM_REGION( 0x100, "user5", 0 )	\
	ROM_LOAD( "spc700.rom", 0x00, 0x40, CRC(44bb3a40) SHA1(97e352553e94242ae823547cd853eecda55c20f0) ) \
	ROM_REGION( 0x10000, "krom", 0 ) \
	ROM_LOAD( "krom1.ic1", 0x00000, 0x10000, CRC(c9010002) SHA1(f4c74086a83b728b1c1af3a021a60efa80eff5a4) ) \
	ROM_REGION( 0x100000, "user3", 0 ) \
	ROM_LOAD( "atrom-4s-0.rom5", 0x00000, 0x80000, CRC(ad3ec05c) SHA1(a3d336db585fe02a37c323422d9db6a33fd489a6) ) \


ROM_START( sfcbox )
	SFCBOX_BIOS

	ROM_REGION( 0x8000, "grom", ROMREGION_ERASEFF )
ROM_END

ROM_START( pss61 )
	SFCBOX_BIOS

	ROM_REGION( 0x8000, "grom", 0 )
	ROM_LOAD( "grom1-1.ic1", 0x0000, 0x8000, CRC(333bf9a7) SHA1(5d0cd9ca29e5580c3eebe9f136839987c879f979) )

	ROM_REGION( 0x380000, "game", 0 )
	ROM_LOAD( "shvc-mk-0.rom6", 0x000000, 0x080000, CRC(c8002453) SHA1(cbb853bf911255c1d8eb27cd34fc7855a0dda218) )
	ROM_LOAD( "shvc-4m-1.rom3", 0x080000, 0x200000, CRC(91b28d56) SHA1(b83dd73d3d6049450bb8092d73c3af879804f58c) )
	ROM_LOAD( "shvc-fo-1.ic20", 0x280000, 0x100000, CRC(ad668a41) SHA1(39ff7354a7fa02295c899b7a7ec3556998ac2636) ) /* Super FX hook needed for Star Fox */
ROM_END

ROM_START( pss62 )
	SFCBOX_BIOS

	ROM_REGION( 0x8000, "grom", 0 )
	ROM_LOAD( "grom2-1.ic1", 0x0000, 0x8000, CRC(bcfc5642) SHA1(a96e52685bd3dcdf09d1b7acd6e1c1ab7726a640) )

	ROM_REGION( 0x180000, "game", 0 )
	ROM_LOAD( "shvc-gc-0.rom1", 0x000000, 0x100000, CRC(b4fd7aff) SHA1(eb553b77418dedba25fc4d5dddcb04f424b0f6a9) )
	ROM_LOAD( "shvc-2a-1.rom3", 0x100000, 0x080000, CRC(6b23e2e4) SHA1(684123a12ca1e31115bd6221d96f82461066877f) )
ROM_END

ROM_START( pss63 )
	SFCBOX_BIOS

	ROM_REGION( 0x8000, "grom", 0 )
	ROM_LOAD( "grom3-1.ic1", 0x0000, 0x8000, CRC(ebec4c1c) SHA1(d638ef1486b4c0b3d4d5b666929ca7947e16efad) )

	ROM_REGION( 0x500000, "game", 0 )
	ROM_LOAD( "shvc-t2-1.rom3", 0x000000, 0x100000, CRC(4ae93c10) SHA1(5fa25d027940907b769578d7bf85a9d5ba94911a) )
	ROM_LOAD( "shvc-8x-1.rom1", 0x100000, 0x400000, CRC(3adef543) SHA1(df02860e691fbee453e345dd343c08b6da08d4ea) )
ROM_END

ROM_START( pss64 )
	SFCBOX_BIOS

	ROM_REGION( 0x8000, "grom", 0 )
	ROM_LOAD( "grom4-1.ic1", 0x0000, 0x8000, NO_DUMP )

	ROM_REGION( 0x500000, "game", 0 )
	ROM_LOAD( "shvc-m4-0.rom3", 0x000000, 0x100000, CRC(fb259f4f) SHA1(8faeb56f80e82dd042bdc84d19c526a979c6de8f) )
	ROM_LOAD( "shvc-8x-1.rom1", 0x100000, 0x400000, CRC(3adef543) SHA1(df02860e691fbee453e345dd343c08b6da08d4ea) )
//  Possibly reverse order :
//  ROM_LOAD( "shvc-8x-1.rom1", 0x000000, 0x400000, CRC(3adef543) SHA1(df02860e691fbee453e345dd343c08b6da08d4ea) )
//  ROM_LOAD( "shvc-m4-0.rom3", 0x400000, 0x100000, CRC(fb259f4f) SHA1(8faeb56f80e82dd042bdc84d19c526a979c6de8f) )
ROM_END


GAME( 1994, sfcbox,      0,     sfcbox,      snes, snes_state,    snes,    ROT0, "Nintendo",                   "Super Famicom Box BIOS", GAME_IS_BIOS_ROOT | GAME_NOT_WORKING )
GAME( 1994, pss61, sfcbox, sfcbox, snes, snes_state, snes, ROT0, "Nintendo", "Super Mario Kart / Super Mario Collection / Star Fox (Super Famicom Box)", GAME_NOT_WORKING )
GAME( 1994, pss62, sfcbox, sfcbox, snes, snes_state, snes, ROT0, "T&E Soft / I'Max", "New Super 3D Golf Simulation - Waialae No Kiseki / Super Mahjong 2 (Super Famicom Box)", GAME_NOT_WORKING )
GAME( 1994, pss63, sfcbox, sfcbox, snes, snes_state, snes, ROT0, "Nintendo / BPS", "Super Donkey Kong / Super Tetris 2 + Bombliss (Super Famicom Box)", GAME_NOT_WORKING )
GAME( 199?, pss64, sfcbox, sfcbox, snes, snes_state, snes, ROT0, "Nintendo / Hudson Soft", "Super Donkey Kong / Super Bomberman 2 (Super Famicom Box)", GAME_NOT_WORKING )
