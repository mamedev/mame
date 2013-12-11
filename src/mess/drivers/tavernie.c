// license:MAME
// copyright-holders: Robbbert
/***************************************************************************

    Tavernier CPU09 and IVG09 (Realisez votre ordinateur individuel)

    2013-12-08 Skeleton driver.

    This system was described in a French computer magazine.

    CPU09 includes 6809, 6821, 6840, 6850, cassette, rs232
    IVG09 includes 6845, WD1795, another 6821, beeper

ToDo:
    - Almost everything
    - Scrolling can cause the screen to go blank (use Z command to fix)
    - There's supposed to be a device at 2000-2003
    - Character rom is not dumped
    - Fix cassette
    - Need software

List of commands (must be in UPPERCASE):
A -
B -
C -
D - Dump memory (^X to break)
G -
I -
L - Load cassette
M -
N -
O -
P - Save cassette
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
#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"
#include "video/mc6845.h"
#include "machine/keyboard.h"
#include "machine/serial.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"
#include "tavernie.lh"


class tavernie_state : public driver_device
{
public:
	tavernie_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_p_videoram(*this, "videoram")
		, m_cass(*this, "cassette")
		, m_maincpu(*this, "maincpu")
	{ }

	DECLARE_READ_LINE_MEMBER(ca1_r);
	DECLARE_READ8_MEMBER(pa_r);
	DECLARE_WRITE8_MEMBER(pa_w);
	DECLARE_WRITE8_MEMBER(pb_w);
	DECLARE_READ8_MEMBER(keyin_r);
	DECLARE_WRITE8_MEMBER(kbd_put);
	const UINT8 *m_p_chargen;
	optional_shared_ptr<UINT8> m_p_videoram;
private:
	UINT8 m_term_data;
	UINT8 m_pa;
	virtual void machine_reset();
	required_device<cassette_image_device> m_cass;
	required_device<cpu_device> m_maincpu;
};


static ADDRESS_MAP_START(cpu09_mem, AS_PROGRAM, 8, tavernie_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x1000, 0x1fff) AM_NOP
	AM_RANGE(0xeb00, 0xeb03) AM_DEVREADWRITE("pia", pia6821_device, read, write)
	AM_RANGE(0xeb04, 0xeb04) AM_DEVREADWRITE("acia", acia6850_device, status_read, control_write)
	AM_RANGE(0xeb05, 0xeb05) AM_DEVREADWRITE("acia", acia6850_device, data_read, data_write)
	AM_RANGE(0xeb08, 0xeb0f) AM_DEVREADWRITE("ptm", ptm6840_device, read, write)
	AM_RANGE(0xec00, 0xefff) AM_RAM // 1Kx8 RAM MK4118
	AM_RANGE(0xf000, 0xffff) AM_ROM AM_REGION("roms", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(ivg09_mem, AS_PROGRAM, 8, tavernie_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x1000, 0x1fff) AM_RAM AM_SHARE("videoram")
	//AM_RANGE(0x2000, 0x2003) 6821 on ivg09
	AM_RANGE(0x2002, 0x2003) AM_READ(keyin_r)
	AM_RANGE(0x2080, 0x2080) AM_DEVREADWRITE("crtc", mc6845_device, status_r, address_w)
	AM_RANGE(0x2081, 0x2081) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0xeb00, 0xeb03) AM_DEVREADWRITE("pia", pia6821_device, read, write)
	AM_RANGE(0xeb04, 0xeb04) AM_DEVREADWRITE("acia", acia6850_device, status_read, control_write)
	AM_RANGE(0xeb05, 0xeb05) AM_DEVREADWRITE("acia", acia6850_device, data_read, data_write)
	AM_RANGE(0xeb08, 0xeb0f) AM_DEVREADWRITE("ptm", ptm6840_device, read, write)
	AM_RANGE(0xec00, 0xefff) AM_RAM // 1Kx8 RAM MK4118
	AM_RANGE(0xf000, 0xffff) AM_ROM AM_REGION("roms", 0)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( cpu09 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, "IRQ PTM") PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x02, 0x00, "IRQ ACIA") PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x60, 0x40, "Terminal") PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, "110 baud" )
	PORT_DIPSETTING(    0x20, "300 baud" )
	PORT_DIPSETTING(    0x40, "1200 baud" )
	PORT_DIPSETTING(    0x60, "IVG09 (mc6845)" )
INPUT_PORTS_END

static INPUT_PORTS_START( ivg09 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, "IRQ PTM") PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x02, 0x00, "IRQ ACIA") PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x60, 0x60, "Terminal") PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, "110 baud" )
	PORT_DIPSETTING(    0x20, "300 baud" )
	PORT_DIPSETTING(    0x40, "1200 baud" )
	PORT_DIPSETTING(    0x60, "IVG09 (mc6845)" )
INPUT_PORTS_END

void tavernie_state::machine_reset()
{
	m_p_chargen = memregion("chargen")->base();
	m_term_data = 0;
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


READ8_MEMBER( tavernie_state::pa_r )
{
	return ioport("DSW")->read() | m_pa;
}


/*
d0: A16
d1: A17 (for 256kb expansion)
d2, d3, d4: to 40-pin port
d5: S3
d6: S4
d7: cassout
*/
WRITE8_MEMBER( tavernie_state::pa_w )
{
	m_pa = data & 0x9f;
	m_cass->output(BIT(data, 7) ? -1.0 : +1.0);
}

// centronics
WRITE8_MEMBER( tavernie_state::pb_w )
{
}

// cass in
READ_LINE_MEMBER( tavernie_state::ca1_r )
{
	return (m_cass->input() > +0.01);
}

static const pia6821_interface mc6821_intf =
{
	DEVCB_DRIVER_MEMBER(tavernie_state, pa_r),     /* port A input */
	DEVCB_NULL,     /* port B input */
	DEVCB_DRIVER_LINE_MEMBER(tavernie_state, ca1_r),     /* CA1 input - cassin */
	DEVCB_NULL,     /* CB1 input */
	DEVCB_NULL,     /* CA2 input */
	DEVCB_NULL,     /* CB2 input */
	DEVCB_DRIVER_MEMBER(tavernie_state, pa_w),     /* port A output */
	DEVCB_DRIVER_MEMBER(tavernie_state, pb_w),     /* port B output */
	DEVCB_NULL,     /* CA2 output */
	DEVCB_NULL,     /* CB2 output */
	DEVCB_CPU_INPUT_LINE("maincpu", M6809_IRQ_LINE),    /* IRQA output */
	DEVCB_CPU_INPUT_LINE("maincpu", M6809_IRQ_LINE)     /* IRQB output */
};

// all i/o lines connect to the 40-pin expansion connector
static const ptm6840_interface mc6840_intf =
{
	XTAL_4MHz / 4,
	{ 0, 0, 0 },
	{ DEVCB_NULL,
		DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_NMI),
		DEVCB_NULL },
	DEVCB_CPU_INPUT_LINE("maincpu", M6809_IRQ_LINE)
};

static ACIA6850_INTERFACE( mc6850_intf )
{
	153600,
	153600,
	DEVCB_DEVICE_LINE_MEMBER("rs232", serial_port_device, rx),
	DEVCB_DEVICE_LINE_MEMBER("rs232", serial_port_device, tx),
	DEVCB_DEVICE_LINE_MEMBER("rs232", rs232_port_device, cts_r),
	DEVCB_DEVICE_LINE_MEMBER("rs232", rs232_port_device, rts_w),
	DEVCB_NULL,
	DEVCB_NULL
};

static const rs232_port_interface rs232_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

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

static MACHINE_CONFIG_START( cpu09, tavernie_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6809E, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(cpu09_mem)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* Devices */
	MCFG_CASSETTE_ADD( "cassette", default_cassette_interface )
	MCFG_RS232_PORT_ADD("rs232", rs232_intf, default_rs232_devices, "serial_terminal")
	MCFG_PIA6821_ADD("pia", mc6821_intf)
	MCFG_PTM6840_ADD("ptm", mc6840_intf)
	MCFG_ACIA6850_ADD("acia", mc6850_intf)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ivg09, cpu09 )
	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(ivg09_mem)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(80*8, 25*10)
	MCFG_SCREEN_VISIBLE_AREA(0, 80*8-1, 0, 25*10-1)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)
	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT_OVERRIDE(driver_device, black_and_white)
	MCFG_DEFAULT_LAYOUT(layout_tavernie)

	/* Devices */
	MCFG_ASCII_KEYBOARD_ADD(KEYBOARD_TAG, keyboard_intf)
	MCFG_MC6845_ADD("crtc", MC6845, "screen", 1008000, mc6845_intf) // unknown clock
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( cpu09 )
	ROM_REGION( 0x1000, "roms", 0 )
	ROM_LOAD( "tavbug.bin",   0x0000, 0x1000, CRC(77945cae) SHA1(d89b577bc0b4e15e9a49a849998681bdc6cf5fbe) )
ROM_END

ROM_START( ivg09 )
	ROM_REGION( 0x1000, "roms", 0 )
	ROM_LOAD( "tavbug.bin",   0x0000, 0x1000, CRC(77945cae) SHA1(d89b577bc0b4e15e9a49a849998681bdc6cf5fbe) )

	// charrom is missing, using one from 'c10' for now
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "c10_char.bin", 0x0000, 0x2000, CRC(cb530b6f) SHA1(95590bbb433db9c4317f535723b29516b9b9fcbf))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE  INPUT    CLASS          INIT    COMPANY        FULLNAME   FLAGS */
COMP( 1982, cpu09,  0,      0,       cpu09,   cpu09,   driver_device,   0,   "C. Tavernier",  "CPU09", GAME_NOT_WORKING )
COMP( 1983, ivg09,  cpu09,  0,       ivg09,   ivg09,   driver_device,   0,   "C. Tavernier",  "CPU09 with IVG09", GAME_NOT_WORKING )
