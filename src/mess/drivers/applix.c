/***************************************************************************

    Skeleton driver for Applix 1616 computer

    See for docs: http;//www.microbee-mspp.org.au
    You need to sign up and make an introductory thread.
    Then you will be granted permission to visit the repository.

    First revealed to the world in December 1986 issue of Electronics Today
    International (ETI) an Australian electronics magazine which is now defunct.

    The main articles appeared in ETI February/March/April 1987, followed by
    other articles in various issues after that.

    TODO: everything!
    - Required device Z8530 Z80SCC (not emulated, but it is just a dual-channel
      serial controller, possibly just like the other ones)
    - Keyboard is a standard pc keyboard
    - Sound is stereo dac-sound, plus an analog output. Details unknown.

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "video/mc6845.h"


class applix_state : public driver_device
{
public:
	applix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_crtc(*this, "crtc")
	,
		m_base(*this, "base"){ }

	required_device<cpu_device> m_maincpu;
	required_device<mc6845_device> m_crtc;
	DECLARE_READ16_MEMBER(applix_inputs_r);
	DECLARE_WRITE16_MEMBER(applix_index_w);
	DECLARE_WRITE16_MEMBER(applix_register_w);
	required_shared_ptr<UINT16> m_base;
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
};

WRITE16_MEMBER( applix_state::applix_index_w )
{
	m_crtc->address_w( space, offset, data >> 8 );
}

WRITE16_MEMBER( applix_state::applix_register_w )
{
	m_crtc->register_w( space, offset, data >> 8 );
}

READ16_MEMBER( applix_state::applix_inputs_r )
{// as far as i can tell:
// bits 4,5,6,7 are SW0,1,2,3
// bits 2,3 joystick in
// bit 1 cassette in
// bit 0 something to do with audio
	return 0;
}

// dac-latch seems to be:
// bit 7 cassette relay, low=on
// bit 6,5,4 sound outputs
// bit 3 cassette LED, low=on
// bit 2,1,0 joystick

static ADDRESS_MAP_START(applix_mem, AS_PROGRAM, 16, applix_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xffffff)
	AM_RANGE(0x000000, 0x07ffff) AM_RAM AM_SHARE("base")
	AM_RANGE(0x080000, 0x47ffff) AM_NOP //AM_RAM // expansion
	AM_RANGE(0x500000, 0x5fffff) AM_ROM
	//AM_RANGE(0x600000, 0x600001) centronics latch (odd), video palette entry 0 (even)
	//AM_RANGE(0x600020, 0x600021) video palette entry 1 (even)
	//AM_RANGE(0x600040, 0x600041) video palette entry 2 (even)
	//AM_RANGE(0x600060, 0x600061) video palette entry 3 (even)
	//AM_RANGE(0x600080, 0x600081) dac latch (odd)
	//AM_RANGE(0x600100, 0x600101) video latch (=border colour, high nybble; video base, low nybble) (odd)
	//AM_RANGE(0x600180, 0x600181) analog multiplexer latch (odd)
	//AM_RANGE(0x700000, 0x700007) z80-scc (ch b control, ch b data, ch a control, ch a data) on even addresses
	AM_RANGE(0x700080, 0x700081) AM_READ(applix_inputs_r) //input port (odd)
	//AM_RANGE(0x700100, 0x70011f) VIA base address (even)
	AM_RANGE(0x700180, 0x700181) AM_WRITE(applix_index_w)
	AM_RANGE(0x700182, 0x700183) AM_WRITE(applix_register_w)
	//600000, 6FFFFF  io ports and latches
	//700000, 7FFFFF  peripheral chips and devices
	//800000, FFC000  optional roms
	//FFFFC0, FFFFFF  disk controller board
ADDRESS_MAP_END

// io priorities:
// 4 cassette
// 3 scc
// 2 via

/* Input ports */
static INPUT_PORTS_START( applix )
INPUT_PORTS_END


void applix_state::machine_reset()
{
	UINT8* RAM = memregion("maincpu")->base();
	memcpy(m_base, RAM+0x500000, 16);
	machine().device("maincpu")->reset();
}


void applix_state::palette_init()
{ // shades need to be verified - the names on the right are from the manual
	const UINT8 colors[16*3] = {
	0x00, 0x00, 0x00,   //  0 Black
	0x40, 0x40, 0x40,   //  1 Dark Grey
	0x00, 0x00, 0x80,   //  2 Dark Blue
	0x00, 0x00, 0xff,   //  3 Mid Blue
	0x00, 0x80, 0x00,   //  4 Dark Green
	0x00, 0xff, 0x00,   //  5 Green
	0x00, 0xff, 0xff,   //  6 Blue Grey
	0x00, 0x7f, 0x7f,   //  7 Light Blue
	0x7f, 0x00, 0x00,   //  8 Dark Red
	0xff, 0x00, 0x00,   //  9 Red
	0x7f, 0x00, 0x7f,   // 10 Dark Violet
	0xff, 0x00, 0xff,   // 11 Violet
	0x7f, 0x7f, 0x00,   // 12 Brown
	0xff, 0xff, 0x00,   // 13 Yellow
	0xbf, 0xbf, 0xbf,   // 14 Light Grey
	0xff, 0xff, 0xff }; // 15 White

	UINT8 r, b, g, i, color_count = 0;

	for (i = 0; i < 48; color_count++)
	{
		r = colors[i++]; g = colors[i++]; b = colors[i++];
		palette_set_color(machine(), color_count, MAKE_RGB(r, g, b));
	}
}


void applix_state::video_start()
{
}

MC6845_UPDATE_ROW( applix_update_row )
{
#if 0
// This needs complete rewriting
// Need to display a border but the mame mc6845 code doesn't allow it.
// 640x200, display 4 of 16 colours, each 2 bits points at a palette entry.
// 320x200, display all 16 colours, each nybble has the colour code
// There is a monochrome mode, but no info found as yet.
// Display is bitmapped, no character generator.
// The video page can be moved around to almost anywhere in memory.
	applix_state *state = device->machine().driver_data<applix_state>();
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());
	UINT8 chr,gfx,fg,bg;
	UINT16 mem,x,col;
	UINT16 colourm = (state->m_08 & 0x0e) << 7;
	UINT32  *p = &bitmap.pix32(y);

	for (x = 0; x < x_count; x++)           // for each character
	{
		UINT8 inv=0;
		mem = (ma + x) & 0x7ff;
		chr = state->m_videoram[mem];
		col = state->m_colorram[mem] | colourm;                 // read a byte of colour

		/* get pattern of pixels for that character scanline */
		gfx = state->m_gfxram[(chr<<4) | ra] ^ inv;
		fg = (col & 0x001f) | 64;                   // map to foreground palette
		bg = (col & 0x07e0) >> 5;                   // and background palette

		/* Display a scanline of a character (8 pixels) */
		*p++ = palette[( gfx & 0x80 ) ? fg : bg];
		*p++ = palette[( gfx & 0x40 ) ? fg : bg];
		*p++ = palette[( gfx & 0x20 ) ? fg : bg];
		*p++ = palette[( gfx & 0x10 ) ? fg : bg];
		*p++ = palette[( gfx & 0x08 ) ? fg : bg];
		*p++ = palette[( gfx & 0x04 ) ? fg : bg];
		*p++ = palette[( gfx & 0x02 ) ? fg : bg];
		*p++ = palette[( gfx & 0x01 ) ? fg : bg];
	}
#endif
}

static const mc6845_interface applix_crtc = {
	"screen",           /* name of screen */
	8,          /* number of dots per character */
	NULL,
	applix_update_row,      /* handler to display a scanline */
	NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	NULL
};

static MACHINE_CONFIG_START( applix, applix_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 7500000)
	MCFG_CPU_PROGRAM_MAP(applix_mem)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)

	MCFG_PALETTE_LENGTH(16)

	MCFG_MC6845_ADD("crtc", MC6845, 1875000, applix_crtc) // 6545
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( applix )
	ROM_REGION(0x1000000, "maincpu", 0)
	ROM_LOAD16_BYTE( "1616oshv.044", 0x500000, 0x10000, CRC(4a1a90d3) SHA1(4df504bbf6fc5dad76c29e9657bfa556500420a6) )
	ROM_LOAD16_BYTE( "1616oslv.044", 0x500001, 0x10000, CRC(ef619994) SHA1(ff16fe9e2c99a1ffc855baf89278a97a2a2e881a) )
	//ROM_LOAD16_BYTE( "1616oshv.045", 0x520000, 0x10000, CRC(9dfb3224) SHA1(5223833a357f90b147f25826c01713269fc1945f) )
	//ROM_LOAD16_BYTE( "1616oslv.045", 0x520001, 0x10000, CRC(951bd441) SHA1(e0a38c8d0d38d84955c1de3f6a7d56ce06b063f6) )
	//ROM_LOAD( "1616osv.045",  0x540000, 0x20000, CRC(b9f75432) SHA1(278964e2a02b1fe26ff34f09dc040e03c1d81a6d) )
	//ROM_LOAD( "1616ssdv.022", 0x560000, 0x08000, CRC(6d8e413a) SHA1(fc27d92c34f231345a387b06670f36f8c1705856) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1986, applix, 0,       0,     applix, applix, driver_device,   0,       "Applix Pty Ltd",   "Applix 1616", GAME_NOT_WORKING | GAME_NO_SOUND)
