/***************************************************************************

        Vegas 6809

        Skeleton driver

Devices:

MC6809 cpu
MC6840 timer
MM58174 RTC
MB8876 (or FD1791) FDC
SY6545-1 CRTC
2x MC6821 PIA
2x MC6850 ACIA

Memory ranges:

0000-EFFF RAM
F000-F7FF Devices
F800-FFFF ROM

Monitor commands:

D boot from floppy (launch Flex OS)
F relaunch Flex
G go
M modify memory (. to exit)

ToDo:

- Colours (a photo shows colours but the schematic doesn't
           have any ram for colours).

- Connect the devices (RTC, FDC, PIAs, ACIAs, timer).

- Find the missing character generator rom.

- Add mirrors to the address map.

- Add sound. Pressing BEL character issues commands to the timer chip.

- Replace terminal keyboard with proper plug-in unit (no info).

****************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "video/mc6845.h"
//#include "machine/6821pia.h"
//#include "machine/6850acia.h"
#include "machine/keyboard.h"



class v6809_state : public driver_device
{
public:
	v6809_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_crtc(*this, "crtc"),
	m_video_address(0)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<mc6845_device> m_crtc;
	DECLARE_READ8_MEMBER(key_r);
	DECLARE_READ8_MEMBER(status_r);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(v6809_address_w);
	DECLARE_WRITE8_MEMBER(v6809_register_w);
	DECLARE_WRITE8_MEMBER(kbd_put);
	UINT8 *m_p_videoram;
	const UINT8 *m_p_chargen;
	UINT16 m_video_address;
	UINT8 m_video_index;
	UINT8 m_term_data;
	UINT8 m_vidbyte;
	virtual void machine_reset();
	virtual void video_start();
};


static ADDRESS_MAP_START(v6809_mem, AS_PROGRAM, 8, v6809_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xf000) AM_MIRROR(0xfe) AM_DEVREAD("crtc", mc6845_device, status_r) AM_WRITE(v6809_address_w)
	AM_RANGE(0xf001, 0xf001) AM_MIRROR(0xfe) AM_DEVREAD("crtc", mc6845_device, register_r) AM_WRITE(v6809_register_w)
	AM_RANGE(0xf200, 0xf200) AM_MIRROR(0xff) AM_WRITE(videoram_w)
	//AM_RANGE(0xf505, 0xf506) acia-1 (modem)
	//AM_RANGE(0xf50c, 0xf50d) acia-2 (printer)
	//AM_RANGE(0xf600, 0xf603) AM_MIRROR(0x3c) disk controller
	//AM_RANGE(0xf640, 0xf64f) AM_MIRROR(0x30) real time clock
	//AM_RANGE(0xf680, 0xf683) AM_MIRROR(0x3c) pia-1 (disk & keyboard)
	//AM_RANGE(0xf6c8, 0xf6cf) timer
	//AM_RANGE(0xf6d0, 0xf6d3) pia-2
	AM_RANGE(0xf800, 0xffff) AM_ROM

	//temp
	AM_RANGE(0xf682, 0xf682) AM_READ(key_r)
	AM_RANGE(0xf683, 0xf683) AM_READ(status_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START(v6809_io, AS_PROGRAM, 8, v6809_state)
	AM_RANGE(0x0064, 0x0064) AM_WRITENOP
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( v6809 )
INPUT_PORTS_END

// **** Video ****

/* F4 Character Displayer */
static const gfx_layout v6809_charlayout =
{
	8, 10,                  /* 8 x 10 characters */
	256,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( v6809 )
	GFXDECODE_ENTRY( "chargen", 0x0000, v6809_charlayout, 0, 1 )
GFXDECODE_END

MC6845_UPDATE_ROW( v6809_update_row )
{
	v6809_state *state = device->machine().driver_data<v6809_state>();
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());
	UINT8 chr,gfx;
	UINT16 mem,x;
	UINT32 *p = &bitmap.pix32(y);

	for (x = 0; x < x_count; x++)
	{
		mem = (ma + x) & 0x7ff;
		chr = state->m_p_videoram[mem];
		if (x == cursor_x)
			gfx = 0xff;
		else
			gfx = state->m_p_chargen[(chr<<4) | ra];

		/* Display a scanline of a character (8 pixels) */
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

MC6845_ON_UPDATE_ADDR_CHANGED( v6809_update_addr )
{
/* not sure what goes in here - parameters passed are device, address, strobe */
	v6809_state *state = device->machine().driver_data<v6809_state>();
	state->m_video_address = address & 0x7ff;
}

void v6809_state::video_start()
{
	m_p_chargen = machine().root_device().memregion("chargen")->base();
	m_p_videoram = memregion("videoram")->base();
}

static const mc6845_interface v6809_crtc = {
	"screen",           /* name of screen */
	8,              /* number of dots per character */
	NULL,
	v6809_update_row,       /* handler to display a scanline */
	NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	v6809_update_addr       /* handler to process transparent mode */
};

WRITE8_MEMBER( v6809_state::videoram_w )
{
	m_vidbyte = data;
}

WRITE8_MEMBER( v6809_state::v6809_address_w )
{
	m_crtc->address_w( space, 0, data );

	m_video_index = data & 0x1f;

	if (m_video_index == 31)
		m_p_videoram[m_video_address] = m_vidbyte;
}

WRITE8_MEMBER( v6809_state::v6809_register_w )
{
	UINT16 temp = m_video_address;

	m_crtc->register_w( space, 0, data );

	// Get transparent address
	if (m_video_index == 18)
		m_video_address = ((data & 7) << 8 ) | (temp & 0xff);
	else
	if (m_video_index == 19)
		m_video_address = data | (temp & 0xff00);
}

// **** Keyboard ****

READ8_MEMBER( v6809_state::key_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER( v6809_state::status_r )
{
	return (m_term_data) ? 0x80 : 0;
}

WRITE8_MEMBER( v6809_state::kbd_put )
{
	m_term_data = data;
}

static ASCII_KEYBOARD_INTERFACE( keyboard_intf )
{
	DEVCB_DRIVER_MEMBER(v6809_state, kbd_put)
};

// *** Machine ****

void v6809_state::machine_reset()
{
}

static MACHINE_CONFIG_START( v6809, v6809_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, XTAL_16MHz / 4)
	MCFG_CPU_PROGRAM_MAP(v6809_mem)
	MCFG_CPU_IO_MAP(v6809_io)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", sy6545_1_device, screen_update)
	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(black_and_white)
	MCFG_GFXDECODE(v6809)

	/* Devices */
	MCFG_MC6845_ADD("crtc", SY6545_1, XTAL_16MHz / 8, v6809_crtc)
	MCFG_ASCII_KEYBOARD_ADD(KEYBOARD_TAG, keyboard_intf)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( v6809 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "v6809.rom", 0xf800, 0x0800, CRC(54bf5f32) SHA1(10d1d70f0b51e2b90e5c29249d3eab4c6b0033a1) )

	ROM_REGION( 0x800, "videoram", ROMREGION_ERASE00 )

	/* character generator not dumped, using the one from 'h19' for now */
	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "2716_444-29_h19font.bin", 0x0000, 0x0800, BAD_DUMP CRC(d595ac1d) SHA1(130fb4ea8754106340c318592eec2d8a0deaf3d0))
	ROM_RELOAD(0x0800, 0x0800)
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY      FULLNAME       FLAGS */
COMP( 1982, v6809,  0,      0,       v6809,     v6809, driver_device,   0,      "Microkit", "Vegas 6809", GAME_NOT_WORKING | GAME_NO_SOUND)
