/***************************************************************************

        EC-65

        16/07/2009 Initial driver.

****************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "cpu/g65816/g65816.h"
#include "video/mc6845.h"
#include "machine/6821pia.h"
#include "machine/6522via.h"
#include "machine/6551acia.h"
#include "machine/6850acia.h"
#include "machine/keyboard.h"

#define PIA6821_TAG "pia6821"
#define ACIA6850_TAG "acia6850"
#define ACIA6551_TAG "acia6551"
#define VIA6522_0_TAG "via6522_0"
#define VIA6522_1_TAG "via6522_1"
#define MC6845_TAG "mc6845"

class ec65_state : public driver_device
{
public:
	ec65_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_via_0(*this, VIA6522_0_TAG),
	m_p_videoram(*this, "videoram"){ }

	DECLARE_READ8_MEMBER(ec65_via_read_a);
	DECLARE_READ8_MEMBER(ec65_read_ca1 );
	DECLARE_READ8_MEMBER(ec65_via_read_b);
	DECLARE_WRITE8_MEMBER(ec65_via_write_a);
	DECLARE_WRITE8_MEMBER(ec65_via_write_b);
	DECLARE_WRITE8_MEMBER(kbd_put);
	UINT8 *m_p_chargen;
	UINT8 m_keyboard_input;
	optional_device<via6522_device> m_via_0;
	required_shared_ptr<UINT8> m_p_videoram;
	virtual void machine_reset();
	virtual void video_start();
};

static ADDRESS_MAP_START(ec65_mem, AS_PROGRAM, 8, ec65_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe003) AM_DEVREADWRITE(PIA6821_TAG, pia6821_device, read, write)
	AM_RANGE(0xe010, 0xe010) AM_DEVREADWRITE(ACIA6850_TAG, acia6850_device, status_read, control_write)
	AM_RANGE(0xe011, 0xe011) AM_DEVREADWRITE(ACIA6850_TAG, acia6850_device, data_read, data_write)
	AM_RANGE(0xe100, 0xe10f) AM_DEVREADWRITE(VIA6522_0_TAG, via6522_device, read, write)
	AM_RANGE(0xe110, 0xe11f) AM_DEVREADWRITE(VIA6522_1_TAG, via6522_device, read, write)
	AM_RANGE(0xe130, 0xe133) AM_DEVREADWRITE(ACIA6551_TAG,  acia6551_device, read, write)
	AM_RANGE(0xe140, 0xe140) AM_DEVWRITE(MC6845_TAG, mc6845_device, address_w)
	AM_RANGE(0xe141, 0xe141) AM_DEVREADWRITE(MC6845_TAG, mc6845_device, register_r , register_w)
	AM_RANGE(0xe400, 0xe7ff) AM_RAM // 1KB on-board RAM
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(ec65k_mem, AS_PROGRAM, 8, ec65_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xe7ff) AM_RAM
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END

static const pia6821_interface ec65_pia_interface=
{
	DEVCB_NULL,     /* port A input */
	DEVCB_NULL,     /* port B input */
	DEVCB_NULL,     /* CA1 input */
	DEVCB_NULL,     /* CB1 input */
	DEVCB_NULL,     /* CA2 input */
	DEVCB_NULL,     /* CB2 input */
	DEVCB_NULL,     /* port A output */
	DEVCB_NULL,     /* port B output */
	DEVCB_NULL,     /* CA2 output */
	DEVCB_NULL,     /* CB2 output */
	DEVCB_NULL,     /* IRQA output */
	DEVCB_NULL      /* IRQB output */
};

static ACIA6850_INTERFACE( ec65_acia_intf )
{
	0,
	0,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

READ8_MEMBER( ec65_state::ec65_via_read_a)
{
	return m_keyboard_input;
}

READ8_MEMBER( ec65_state::ec65_read_ca1 )
{
	return 0;
}

READ8_MEMBER( ec65_state::ec65_via_read_b)
{
	return 0xff;
}

WRITE8_MEMBER( ec65_state::ec65_via_write_a )
{
}

WRITE8_MEMBER( ec65_state::ec65_via_write_b )
{
}

static const via6522_interface ec65_via_1_intf=
{
	DEVCB_NULL,   /* in_a_func */
	DEVCB_DRIVER_MEMBER(ec65_state, ec65_via_read_b),   /* in_b_func */
	DEVCB_NULL,                       /* in_ca1_func */
	DEVCB_NULL,                       /* in_cb1_func */
	DEVCB_NULL,                       /* in_ca2_func */
	DEVCB_NULL,                       /* in_cb2_func */
	DEVCB_DRIVER_MEMBER(ec65_state, ec65_via_write_a),  /* out_a_func */
	DEVCB_DRIVER_MEMBER(ec65_state, ec65_via_write_b),  /* out_b_func */
	DEVCB_NULL,                       /* out_ca1_func */
	DEVCB_NULL,                       /* out_cb1_func */
	DEVCB_NULL,                       /* out_ca2_func */
	DEVCB_NULL,                       /* out_cb2_func */
	DEVCB_NULL,                       /* irq_func */
};

static const via6522_interface ec65_via_0_intf=
{
	DEVCB_DRIVER_MEMBER(ec65_state, ec65_via_read_a),      /* in_a_func */
	DEVCB_NULL,      /* in_b_func */
	DEVCB_DRIVER_MEMBER(ec65_state, ec65_read_ca1),      /* in_ca1_func */
	DEVCB_NULL,      /* in_cb1_func */
	DEVCB_NULL,      /* in_ca2_func */
	DEVCB_NULL,      /* in_cb2_func */
	DEVCB_NULL,      /* out_a_func */
	DEVCB_NULL,      /* out_b_func */
	DEVCB_NULL,      /* out_ca1_func */
	DEVCB_NULL,      /* out_cb1_func */
	DEVCB_NULL,      /* out_ca2_func */
	DEVCB_NULL,      /* out_cb2_func */
	DEVCB_NULL,      /* irq_func */
};

/* Input ports */
static INPUT_PORTS_START( ec65 )
INPUT_PORTS_END

WRITE8_MEMBER( ec65_state::kbd_put )
{
	if (data)
	{
		m_keyboard_input = data;
		m_via_0->write_ca1(1);
		m_via_0->write_ca1(0);
	}
}


static ASCII_KEYBOARD_INTERFACE( keyboard_intf )
{
	DEVCB_DRIVER_MEMBER(ec65_state, kbd_put)
};

void ec65_state::machine_reset()
{
}

void ec65_state::video_start()
{
	m_p_chargen = memregion("chargen")->base();
}

static MC6845_UPDATE_ROW( ec65_update_row )
{
	ec65_state *state = device->machine().driver_data<ec65_state>();
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());
	UINT8 chr,gfx,inv;
	UINT16 mem,x;
	UINT32 *p = &bitmap.pix32(y);

	for (x = 0; x < x_count; x++)
	{
		inv = (x == cursor_x) ? 0xff : 0;
		mem = (ma + x) & 0x7ff;
		chr = state->m_p_videoram[mem];

		/* get pattern of pixels for that character scanline */
		gfx = state->m_p_chargen[(chr<<4) | (ra & 0x0f)] ^ inv;

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


static MC6845_INTERFACE( ec65_crtc6845_interface )
{
	"screen",
	false,
	8 /*?*/,
	NULL,
	ec65_update_row,
	NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	NULL
};


/* F4 Character Displayer */
static const gfx_layout ec65_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( ec65 )
	GFXDECODE_ENTRY( "chargen", 0x0000, ec65_charlayout, 0, 1 )
GFXDECODE_END

static MACHINE_CONFIG_START( ec65, ec65_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6502, XTAL_4MHz / 4)
	MCFG_CPU_PROGRAM_MAP(ec65_mem)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 640 - 1, 0, 200 - 1)
	MCFG_SCREEN_UPDATE_DEVICE(MC6845_TAG, mc6845_device, screen_update)

	MCFG_GFXDECODE(ec65)
	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(black_and_white)

	MCFG_MC6845_ADD(MC6845_TAG, MC6845, XTAL_16MHz / 8, ec65_crtc6845_interface)


	/* devices */
	MCFG_PIA6821_ADD( PIA6821_TAG, ec65_pia_interface )
	MCFG_ACIA6850_ADD(ACIA6850_TAG, ec65_acia_intf)
	MCFG_VIA6522_ADD(VIA6522_0_TAG, XTAL_4MHz / 4, ec65_via_0_intf)
	MCFG_VIA6522_ADD(VIA6522_1_TAG, XTAL_4MHz / 4, ec65_via_1_intf)
	MCFG_ACIA6551_ADD(ACIA6551_TAG)     // have XTAL of 1.8432MHz connected
	MCFG_ASCII_KEYBOARD_ADD(KEYBOARD_TAG, keyboard_intf)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( ec65k, ec65_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",G65816, XTAL_4MHz) // can use 4,2 or 1 MHz
	MCFG_CPU_PROGRAM_MAP(ec65k_mem)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 640 - 1, 0, 200 - 1)
	MCFG_SCREEN_UPDATE_DEVICE(MC6845_TAG, mc6845_device, screen_update)

	MCFG_GFXDECODE(ec65)
	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(black_and_white)

	MCFG_MC6845_ADD(MC6845_TAG, MC6845, XTAL_16MHz / 8, ec65_crtc6845_interface)

MACHINE_CONFIG_END

/* ROM definition */
ROM_START( ec65 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ec65.ic6", 0xf000, 0x1000, CRC(acd928ed) SHA1(e02a688a057ff77294717cf7b887425fed0b1153))

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "chargen.ic19", 0x0000, 0x1000, CRC(9b56a28d) SHA1(41c04fd9fb542c50287bc0e366358a61fc4b0cd4)) // Located on VDU card
ROM_END

ROM_START( ec65k )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ec65k.ic19",  0xf000, 0x1000, CRC(5e5a890a) SHA1(daa006f2179fd156833e11c73b37881cafe5dede))

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "chargen.ic19", 0x0000, 0x1000, CRC(9b56a28d) SHA1(41c04fd9fb542c50287bc0e366358a61fc4b0cd4)) // Located on VDU card
ROM_END
/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1985, ec65,   0,       0,      ec65,      ec65, driver_device,    0,    "Elektor Electronics", "EC-65", GAME_NOT_WORKING | GAME_NO_SOUND)
COMP( 1985, ec65k,  ec65,    0,      ec65k,     ec65, driver_device,    0,    "Elektor Electronics", "EC-65K", GAME_NOT_WORKING | GAME_NO_SOUND)
