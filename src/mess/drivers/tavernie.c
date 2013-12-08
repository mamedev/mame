// license:MAME
// copyright-holders: I'm not claiming this!
/***************************************************************************

    Tavernier

    2013-12-08 Skeleton driver.

    This is a French computer. Nothing else is known.

ToDo:
    - Almost everything
    - Scrolling can cause the screen to go blank (use Z command to fix)
    - There's supposed to be a device at 2000-2003
    - Get rid of the bodgy timer. It won't boot up without it.
    - There's a bootstrap rom for this, but it's a bad dump
    - Character rom is not dumped

List of commands (must be in UPPERCASE):
A -
B -
C -
D -
G -
I -
L -
M -
N -
O -
P -
Q -
R - Display/Alter Registers
S -
T -
U -
V -
W -
X - 'erreur de chargement dos'
Y -
Z - more scan lines per row (cursor is bigger)


****************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "video/mc6845.h"
#include "machine/keyboard.h"


class tavernie_state : public driver_device
{
public:
	tavernie_state(const machine_config &mconfig, device_type type, const char *tag)
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
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	required_device<cpu_device> m_maincpu;
};


static ADDRESS_MAP_START(tavernie_mem, AS_PROGRAM, 8, tavernie_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_RAM
	AM_RANGE(0x1000, 0x1fff) AM_RAM AM_SHARE("videoram")
	//AM_RANGE(0x2000, 0x2003) some device
	AM_RANGE(0x2002, 0x2003) AM_READ(keyin_r)
	AM_RANGE(0x2080, 0x2080) AM_DEVREADWRITE("crtc", mc6845_device, status_r, address_w)
	AM_RANGE(0x2081, 0x2081) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0xe000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_ROM AM_REGION("roms", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tavernie_io, AS_IO, 8, tavernie_state)
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( tavernie )
INPUT_PORTS_END

void tavernie_state::machine_reset()
{
	m_p_chargen = memregion("chargen")->base();
	m_term_data = 0;
	timer_set(attotime::from_msec(400), 0); //bodge
}

static MC6845_UPDATE_ROW( update_row )
{
	tavernie_state *state = device->machine().driver_data<tavernie_state>();
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());
	UINT8 chr,gfx=0;
	UINT16 mem,x;
	UINT32 *p = &bitmap.pix32(y);

	for (x = 0; x < x_count; x++)
	{
		UINT8 inv=0;
		if (x == cursor_x) inv=0xff;
		mem = (ma + x) & 0x7ff;
		if (ra > 7)
			gfx = inv;  // some blank spacing lines
		else
		{
			chr = state->m_p_videoram[mem];
			gfx = state->m_p_chargen[(chr<<4) | ra] ^ inv;
		}

		/* Display a scanline of a character */
		*p++ = palette[BIT(gfx, 7)];
		*p++ = palette[BIT(gfx, 6)];
		*p++ = palette[BIT(gfx, 5)];
		*p++ = palette[BIT(gfx, 4)];
		*p++ = palette[BIT(gfx, 3)];
		*p++ = palette[BIT(gfx, 2)];
		*p++ = palette[BIT(gfx, 1)];
		*p++ = palette[BIT(gfx, 0)];
	}
}

static MC6845_INTERFACE( mc6845_intf )
{
	false,              /* show border area */
	8,                  /* number of pixels per video memory address */
	NULL,               /* before pixel update callback */
	update_row,         /* row update callback */
	NULL,               /* after pixel update callback */
	DEVCB_NULL,         /* callback for display state changes */
	DEVCB_NULL,         /* callback for cursor state changes */
	DEVCB_NULL,         /* HSYNC callback */
	DEVCB_NULL,         /* VSYNC callback */
	NULL                /* update address callback */
};

void tavernie_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	space.write_byte(0xef7a, 0xff);
}

READ8_MEMBER( tavernie_state::keyin_r )
{
	if (offset)
		return (m_term_data) ? 0x80 : 0;

	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

WRITE8_MEMBER( tavernie_state::kbd_put )
{
	m_term_data = data;
}

static ASCII_KEYBOARD_INTERFACE( keyboard_intf )
{
	DEVCB_DRIVER_MEMBER(tavernie_state, kbd_put)
};

static MACHINE_CONFIG_START( tavernie, tavernie_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6809, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(tavernie_mem)
	MCFG_CPU_IO_MAP(tavernie_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(80*8, 25*10)
	MCFG_SCREEN_VISIBLE_AREA(0, 80*8-1, 0, 25*10-1)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)
	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT_OVERRIDE(driver_device, black_and_white)

	/* Devices */
	MCFG_ASCII_KEYBOARD_ADD(KEYBOARD_TAG, keyboard_intf)
	MCFG_MC6845_ADD("crtc", MC6845, "screen", 1008000, mc6845_intf)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( tavernie )
	ROM_REGION( 0x1000, "roms", 0 )
	ROM_LOAD( "tavbug.bin",   0x0000, 0x1000, CRC(77945cae) SHA1(d89b577bc0b4e15e9a49a849998681bdc6cf5fbe) )

	// charrom is missing, using one from 'c10' for now
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "c10_char.bin", 0x0000, 0x2000, CRC(cb530b6f) SHA1(95590bbb433db9c4317f535723b29516b9b9fcbf))
ROM_END

/* Driver */

/*    YEAR  NAME       PARENT  COMPAT   MACHINE     INPUT     CLASS          INIT    COMPANY        FULLNAME   FLAGS */
COMP( 19??, tavernie,  0,      0,       tavernie,   tavernie, driver_device,   0,   "<unknown>",  "Tavernier", GAME_IS_SKELETON )
