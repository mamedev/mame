// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        Micro Craft Dimension 68000

        28/12/2011 Skeleton driver.

        This computer had the facility for plug-in cards to emulate popular
        computers of the time such as apple, trs80, kaypro, etc. The floppy
        disk parameters could be set to be able to read the disks of the
        various systems, or you create any other format you wished.

        There is no schematic available, so not fully able to emulate at
        this time.

        ToDo:

        - Floppy controller
        - Floppy formats (not sure if MESS supports user-defined formats)
        - Banking
        - Graphics display (including colour and video_control options)
        - DUART
        - RTC
        - RS232 interface
        - Keyboard
        - Centronics printer
        - Video-high
        - Video-reset
        - Game switches, paddles and timers
        - The plug-in boards
        - Emulator trap function

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/speaker.h"
#include "video/mc6845.h"
#include "machine/keyboard.h"
#include "machine/upd765.h"
#include "softlist.h"

class dim68k_state : public driver_device
{
public:
	dim68k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_crtc(*this, "crtc"),
		m_speaker(*this, "speaker"),
		m_ram(*this, "ram"),
		m_palette(*this, "palette") { }

	DECLARE_READ16_MEMBER( dim68k_duart_r );
	DECLARE_READ16_MEMBER( dim68k_fdc_r );
	DECLARE_READ16_MEMBER( dim68k_game_switches_r );
	DECLARE_READ16_MEMBER( dim68k_speaker_r );
	DECLARE_WRITE16_MEMBER( dim68k_banksw_w );
	DECLARE_WRITE16_MEMBER( dim68k_duart_w );
	DECLARE_WRITE16_MEMBER( dim68k_fdc_w );
	DECLARE_WRITE16_MEMBER( dim68k_printer_strobe_w );
	DECLARE_WRITE16_MEMBER( dim68k_reset_timers_w );
	DECLARE_WRITE16_MEMBER( dim68k_speaker_w );
	DECLARE_WRITE16_MEMBER( dim68k_video_control_w );
	DECLARE_WRITE16_MEMBER( dim68k_video_high_w );
	DECLARE_WRITE16_MEMBER( dim68k_video_reset_w );
	DECLARE_WRITE8_MEMBER(kbd_put);
	MC6845_UPDATE_ROW(crtc_update_row);
	const UINT8 *m_p_chargen;
	bool m_speaker_bit;
	UINT8 m_video_control;
	UINT8 m_term_data;
	virtual void machine_reset() override;
	virtual void video_start() override;
	required_device<cpu_device> m_maincpu;
	required_device<mc6845_device> m_crtc;
	required_device<speaker_sound_device> m_speaker;
	required_shared_ptr<UINT16> m_ram;
	required_device<palette_device> m_palette;
};

READ16_MEMBER( dim68k_state::dim68k_duart_r )
// Port A is for the keyboard : 300 baud, no parity, 8 bits, 1 stop bit. Port B is for RS232.
// The device also controls the parallel printer (except the strobe) and the RTC.
// Device = SCN2681, not emulated. The keyboard is standard ASCII, so we can use the terminal
// keyboard for now.
{
	if (offset==3)
		return m_term_data;
	else
	return 0;
}

READ16_MEMBER( dim68k_state::dim68k_fdc_r )
{
	return 0;
}

READ16_MEMBER( dim68k_state::dim68k_game_switches_r )
// Reading the game port switches
// FFCC11 = switch 0; FFCC13 = switch 1, etc to switch 3
// FFCC19 = paddle 0; FFCC1B = paddle 1, etc to paddle 3
{
	return 0xffff;
}

READ16_MEMBER( dim68k_state::dim68k_speaker_r )
// Any read or write of this address will toggle the position of the speaker cone
{
	m_speaker_bit ^= 1;
	m_speaker->level_w(m_speaker_bit);
	return 0;
}

WRITE16_MEMBER( dim68k_state::dim68k_speaker_w )
{
	m_speaker_bit ^= 1;
	m_speaker->level_w(m_speaker_bit);
}

WRITE16_MEMBER( dim68k_state::dim68k_fdc_w )
{
}

WRITE16_MEMBER( dim68k_state::dim68k_video_high_w )
// "write high byte of address in memory of start of display buffer"
{
}

WRITE16_MEMBER( dim68k_state::dim68k_video_control_w )
{
/* D7 0 = Hires/Graphics; 1= Lores/Text [not emulated yet]
   D6 0 = 8 dots per character; 1 = 7 dots [emulated]
   D5 0 = CRTC and CPU run Asynchronously; 1 = Synchronously [won't be emulated]
   D4,D3  Dot clock: 00=14MHz; 01=3.58MHz; 10=7MHz; 11=1.79MHz [emulated]
   D2 0 = Screen On; 1 = Off [emulated]
   D1 0 = Standard Chars & LoRes; 1 = Alternate Chars & HiRes [not emulated yet]
   D0 0 = Non-Mixed (all text or all Graphics); 1 = Mixed (Colour Graphics and Monochrome Text) [not emulated yet]
 */
	m_crtc->set_hpixels_per_column((data & 0x40) ? 7 : 8);
	m_video_control = data;

	switch (data & 0x18)
	{
		case 0x00: m_crtc->set_clock(XTAL_14MHz);
					break;
		case 0x08: m_crtc->set_clock(XTAL_3_579545MHz);
					break;
		case 0x10: m_crtc->set_clock(XTAL_14MHz / 2);
					break;
		case 0x18: m_crtc->set_clock(XTAL_3_579545MHz / 2);
					break;
	}
}

WRITE16_MEMBER( dim68k_state::dim68k_video_reset_w )
{
}

WRITE16_MEMBER( dim68k_state::dim68k_duart_w )
{
}

WRITE16_MEMBER( dim68k_state::dim68k_reset_timers_w )
// reset game port timer before reading paddles
{
}

WRITE16_MEMBER( dim68k_state::dim68k_printer_strobe_w )
// anything sent here will trigger a one-shot for a strobe pulse
{
}

WRITE16_MEMBER( dim68k_state::dim68k_banksw_w )
// At boot time, the rom and IO occupy 0-FFFF, this moves it to the proper place
{
}

static ADDRESS_MAP_START(dim68k_mem, AS_PROGRAM, 16, dim68k_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x00feffff) AM_RAM AM_SHARE("ram") // 16MB RAM / ROM at boot
	AM_RANGE(0x00ff0000, 0x00ff1fff) AM_ROM AM_REGION("bootrom", 0)
	AM_RANGE(0x00ff2000, 0x00ff7fff) AM_RAM // Graphics Video RAM
	AM_RANGE(0x00ff8000, 0x00ff8001) AM_DEVREADWRITE8("crtc", mc6845_device, status_r, address_w, 0xff)
	AM_RANGE(0x00ff8002, 0x00ff8003) AM_DEVREADWRITE8("crtc", mc6845_device, register_r, register_w, 0xff)
	AM_RANGE(0x00ff8004, 0x00ff8005) AM_WRITE(dim68k_video_high_w)
	AM_RANGE(0x00ff8008, 0x00ff8009) AM_WRITE(dim68k_video_control_w)
	AM_RANGE(0x00ff800a, 0x00ff800b) AM_WRITE(dim68k_video_reset_w)
	AM_RANGE(0x00ff8800, 0x00ff8fff) AM_ROM AM_REGION("cop6512",0) // slot 1 controller rom
	AM_RANGE(0x00ff9000, 0x00ff97ff) AM_ROM AM_REGION("copz80",0) // slot 2 controller rom
	AM_RANGE(0x00ff9800, 0x00ff9fff) AM_ROM AM_REGION("cop8086",0) // slot 3 controller rom
#if 0
	AM_RANGE(0x00ffa000, 0x00ffa7ff) AM_ROM // slot 4 controller rom
	AM_RANGE(0x00ffa800, 0x00ffafff) AM_ROM // slot 5 controller rom
	AM_RANGE(0x00ffb000, 0x00ffb7ff) AM_ROM // slot 6 controller rom
#endif
	AM_RANGE(0x00ffc400, 0x00ffc41f) AM_READWRITE(dim68k_duart_r,dim68k_duart_w) // Signetics SCN2681AC1N40 Dual UART
	AM_RANGE(0x00ffc800, 0x00ffc801) AM_READWRITE(dim68k_speaker_r,dim68k_speaker_w)
	AM_RANGE(0x00ffcc00, 0x00ffcc1f) AM_READWRITE(dim68k_game_switches_r,dim68k_reset_timers_w)
	AM_RANGE(0x00ffd000, 0x00ffd003) AM_DEVICE8("fdc",upd765a_device,map,0x00ff) // NEC uPD765A
	AM_RANGE(0x00ffd004, 0x00ffd005) AM_READWRITE(dim68k_fdc_r,dim68k_fdc_w)
	//AM_RANGE(0x00ffd400, 0x00ffd403) emulation trap control
	AM_RANGE(0x00ffd800, 0x00ffd801) AM_WRITE(dim68k_printer_strobe_w)
	AM_RANGE(0x00ffdc00, 0x00ffdc01) AM_WRITE(dim68k_banksw_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( dim68k )
INPUT_PORTS_END


void dim68k_state::machine_reset()
{
	UINT8* ROM = memregion("bootrom")->base();

	memcpy((UINT8*)m_ram.target(), ROM, 0x2000);

	m_maincpu->reset();
}

void dim68k_state::video_start()
{
	m_p_chargen = memregion("chargen")->base();
}

// Text-only; graphics isn't emulated yet. Need to find out if hardware cursor is used.
MC6845_UPDATE_ROW( dim68k_state::crtc_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT8 chr,gfx,x,xx,inv;
	UINT16 chr16=0x2020; // set to spaces if screen is off
	UINT32 *p = &bitmap.pix32(y);
	UINT8 screen_on = ~m_video_control & 4;
	UINT8 dot8 = ~m_video_control & 40;

	// need to divide everything in half to cater for 16-bit reads
	x_count /= 2;
	ma /= 2;
	xx = 0;

	for (x = 0; x < x_count; x++)
	{
		if (screen_on)
			chr16 = m_ram[ma+x]; // reads 2 characters

		inv=0;
		if (xx == cursor_x && screen_on) inv=0xff;
		xx++;

		chr = chr16>>8;
		gfx = m_p_chargen[(chr<<4) | ra] ^ inv ^ ((chr & 0x80) ? 0xff : 0);
		*p++ = palette[BIT(gfx, 7)];
		*p++ = palette[BIT(gfx, 6)];
		*p++ = palette[BIT(gfx, 5)];
		*p++ = palette[BIT(gfx, 4)];
		*p++ = palette[BIT(gfx, 3)];
		*p++ = palette[BIT(gfx, 2)];
		*p++ = palette[BIT(gfx, 1)];
		if (dot8) *p++ = palette[BIT(gfx, 1)];

		inv = 0;
		if (xx == cursor_x) inv=0xff;
		xx++;

		chr = chr16;
		gfx = m_p_chargen[(chr<<4) | ra] ^ inv ^ ((chr & 0x80) ? 0xff : 0);
		*p++ = palette[BIT(gfx, 7)];
		*p++ = palette[BIT(gfx, 6)];
		*p++ = palette[BIT(gfx, 5)];
		*p++ = palette[BIT(gfx, 4)];
		*p++ = palette[BIT(gfx, 3)];
		*p++ = palette[BIT(gfx, 2)];
		*p++ = palette[BIT(gfx, 1)];
		if (dot8) *p++ = palette[BIT(gfx, 1)];
	}
}

/* F4 Character Displayer */
static const gfx_layout dim68k_charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( dim68k )
	GFXDECODE_ENTRY( "chargen", 0x0000, dim68k_charlayout, 0, 1 )
GFXDECODE_END

static SLOT_INTERFACE_START( dim68k_floppies )
	SLOT_INTERFACE( "525hd", FLOPPY_525_HD )
SLOT_INTERFACE_END

WRITE8_MEMBER( dim68k_state::kbd_put )
{
	m_term_data = data;
}

static MACHINE_CONFIG_START( dim68k, dim68k_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_10MHz)
	MCFG_CPU_PROGRAM_MAP(dim68k_mem)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 250-1)
	MCFG_PALETTE_ADD_MONOCHROME("palette")
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", dim68k)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* Devices */
	MCFG_UPD765A_ADD("fdc", true, true) // these options unknown
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", dim68k_floppies, "525hd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", dim68k_floppies, "525hd", floppy_image_device::default_floppy_formats)

	MCFG_MC6845_ADD("crtc", MC6845, "screen", 1790000)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(dim68k_state, crtc_update_row)

	MCFG_DEVICE_ADD("keyboard", GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(WRITE8(dim68k_state, kbd_put))

	// software lists
	MCFG_SOFTWARE_LIST_ADD("flop_list", "dim68k")
MACHINE_CONFIG_END

/*
68000

MC101A  82S100
MC102B  82S100
MC103E  2732A
MC104   2732A   label "4050" underneath
MC105   2732A   char gen

6512

MC106   82LS135 U24
MC107   82LS135 U20
MC108   82S137  U23
MC109   82S131  U16
MC110   82LS135 U35

Z80

MC111   82S123  U11

8086

MC112   82LS135 U18
MC113   82S153  U16
*/
/* ROM definition */
ROM_START( dim68k )
	ROM_REGION( 0x2000, "bootrom", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "mc103e.bin", 0x0000, 0x1000, CRC(4730c902) SHA1(5c4bb79ad22def721a22eb63dd05e0391c8082be))
	ROM_LOAD16_BYTE( "mc104.bin",  0x0001, 0x1000, CRC(14b04575) SHA1(43e15d9ebe1c9c1bf1bcfc1be3899a49e6748200))

	ROM_REGION( 0x1000, "chargen", ROMREGION_ERASEFF )
	ROM_LOAD( "mc105e.bin", 0x0000, 0x1000, CRC(7a09daa8) SHA1(844bfa579293d7c3442fcbfa21bda75fff930394))

	// The remaining roms may not be in the correct positions or being loaded correctly
	ROM_REGION( 0x1000, "cop6512", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "mc106.bin", 0x0000, 0x0100, CRC(11530d8a) SHA1(e3eae266535383bcaee2d84d7bed6052d40e4e4a))
	ROM_LOAD16_WORD_SWAP( "mc107.bin", 0x0100, 0x0100, CRC(966db11b) SHA1(3c3105ac842602d8e01b0f924152fd672a85f00c))
	ROM_LOAD16_WORD_SWAP( "mc108.bin", 0x0200, 0x0400, CRC(687f9b0a) SHA1(ed9f1265b25f89f6d3cf8cd0a7b0fb73cb129f9f))
	ROM_LOAD16_WORD_SWAP( "mc109.bin", 0x0600, 0x0200, CRC(4a857f98) SHA1(9f2bbc2171fc49f65aa798c9cd7799a26afd2ddf))
	ROM_LOAD16_WORD_SWAP( "mc110.bin", 0x0800, 0x0100, CRC(e207b457) SHA1(a8987ba3d1bbdb3d8b3b11cec90c532ff09e762e))

	ROM_REGION( 0x1000, "copz80", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "mc111.bin", 0x0000, 0x0020, CRC(6a380057) SHA1(6522a7b3e0af9db14a6ed04d4eec3ee6e44c2dab))

	ROM_REGION( 0x1000, "cop8086", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "mc112.bin", 0x0000, 0x0100, CRC(dfd4cdbb) SHA1(a7831d415943fa86c417066807038bccbabb2573))
	ROM_LOAD( "mc113.bin", 0x0100, 0x00ef, CRC(594bdf05) SHA1(36db911a27d930e023fa12683e86e9eecfffdba6))

	ROM_REGION( 0x1000, "mb", ROMREGION_ERASEFF )   // mainboard unknown
	ROM_LOAD( "mc102.bin", 0x0000, 0x00fa, CRC(38e2abac) SHA1(0d7e730b46fc162764c69c51dea3bbe8337b1a7d))
	ROM_LOAD( "mc101.bin", 0x0100, 0x00fa, CRC(caffb3a0) SHA1(36f5140b306565794c4d856e0c20589b8f2a37f4))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1984, dim68k,  0,       0,     dim68k,   dim68k, driver_device,   0,     "Micro Craft", "Dimension 68000", MACHINE_NOT_WORKING)
