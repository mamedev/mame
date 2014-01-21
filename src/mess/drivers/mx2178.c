/***************************************************************************

    Skeleton driver for Memorex 2178

    Chips: Z80A, N8X305N, 2x B8452A, 4x D4016C-3, 2x HD468A50P, HD46505SP-1
    Crystal: 18.8696MHz
    There is a large piezo-beeper.

    TODO:
    - Everything - this is just a skeleton


****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/mc6845.h"
#include "machine/keyboard.h"


class mx2178_state : public driver_device
{
public:
	mx2178_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_p_videoram(*this, "videoram")
		, m_maincpu(*this, "maincpu")
	{ }

	DECLARE_READ8_MEMBER(keyin_r);
	DECLARE_WRITE8_MEMBER(kbd_put);
	const UINT8 *m_p_chargen;
	required_shared_ptr<UINT8> m_p_videoram;
private:
	UINT8 m_term_data;
	virtual void machine_reset();
	required_device<z80_device> m_maincpu;
};

static ADDRESS_MAP_START(mx2178_mem, AS_PROGRAM, 8, mx2178_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0x2000, 0x27ff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x6000, 0x6fff) AM_RAM
	AM_RANGE(0xe000, 0xe7ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(mx2178_io, AS_IO, 8, mx2178_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0x01, 0x01) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0xa0, 0xa1) AM_READ(keyin_r)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( mx2178 )
INPUT_PORTS_END

READ8_MEMBER( mx2178_state::keyin_r )
{
	if (offset)
	{
		UINT8 ret = m_term_data;
		m_term_data = 0;
		return ret;
	}
	else
		return (m_term_data) ? 1 : 0;
}

WRITE8_MEMBER( mx2178_state::kbd_put )
{
	m_term_data = data;
}

static ASCII_KEYBOARD_INTERFACE( keyboard_intf )
{
	DEVCB_DRIVER_MEMBER(mx2178_state, kbd_put)
};

static MC6845_UPDATE_ROW( update_row )
{
	mx2178_state *state = device->machine().driver_data<mx2178_state>();
	const rgb_t *pens = palette_entry_list_raw(bitmap.palette());
	UINT8 chr,gfx,fg=1,inv;
	UINT16 mem,x;
	UINT32 *p = &bitmap.pix32(y);

	for (x = 0; x < x_count; x++)
	{
		inv = (x == cursor_x) ? 0xff : 0;
		mem = (ma + x) & 0x7ff;
		chr = state->m_p_videoram[mem];

		/* get pattern of pixels for that character scanline */
		gfx = state->m_p_chargen[(chr<<4) | ra] ^ inv;

		/* Display a scanline of a character (8 pixels) */
		*p++ = pens[BIT(gfx, 7) ? fg : 0];
		*p++ = pens[BIT(gfx, 6) ? fg : 0];
		*p++ = pens[BIT(gfx, 5) ? fg : 0];
		*p++ = pens[BIT(gfx, 4) ? fg : 0];
		*p++ = pens[BIT(gfx, 3) ? fg : 0];
		*p++ = pens[BIT(gfx, 2) ? fg : 0];
		*p++ = pens[BIT(gfx, 1) ? fg : 0];
		*p++ = pens[BIT(gfx, 0) ? fg : 0];
	}
}

static MC6845_INTERFACE( crtc_interface )
{
	false,
	0,0,0,0,
	8,
	NULL,
	update_row,
	NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	NULL
};

/* F4 Character Displayer */
static const gfx_layout mx2178_charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8 },
	8*16                    /* every char takes 8 bytes */
};

static GFXDECODE_START( mx2178 )
	GFXDECODE_ENTRY( "chargen", 0x0000, mx2178_charlayout, 0, 1 )
GFXDECODE_END

void mx2178_state::machine_reset()
{
	m_p_chargen = memregion("chargen")->base();
}

static MACHINE_CONFIG_START( mx2178, mx2178_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 18869600/5) // guess
	MCFG_CPU_PROGRAM_MAP(mx2178_mem)
	MCFG_CPU_IO_MAP(mx2178_io)
	//MCFG_CPU_VBLANK_INT_DRIVER("screen", mx2178_state,  irq0_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // not correct
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_GFXDECODE(mx2178)
	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT_OVERRIDE(driver_device, black_and_white)

	/* Devices */
	MCFG_MC6845_ADD("crtc", MC6845, "screen", 18869600 / 8, crtc_interface) // clk unknown
	MCFG_ASCII_KEYBOARD_ADD(KEYBOARD_TAG, keyboard_intf)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( mx2178 )
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD( "96274139.bin", 0x000000, 0x002000, CRC(eb471a27) SHA1(433abefd1a72653d0bf35bcaaeccf9943b96260b) )

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "96273883.bin", 0x000000, 0x001000, CRC(8311fadd) SHA1(573bbad23e893ad9374edc929642dc1cba3452d2) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT   STATE         INIT    COMPANY    FULLNAME       FLAGS */
COMP( 19??, mx2178, 0,      0,       mx2178,    mx2178, driver_device,  0,  "Memorex", "Memorex 2178", GAME_IS_SKELETON )
