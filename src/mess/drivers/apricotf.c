/***************************************************************************

    ACT Apricot F1 series

    preliminary driver by Angelo Salese


11/09/2011 - modernised. The portable doesn't seem to have
             scroll registers, and it sets the palette to black.
             I've added a temporary video output so that you can get
             an idea of what the screen should look like. [Robbbert]

****************************************************************************/

/*

    TODO:

    - CTC/SIO interrupt acknowledge
    - CTC clocks
    - sound

*/

#include "includes/apricotf.h"



//**************************************************************************
//  VIDEO
//**************************************************************************

UINT32 f1_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	address_space *program = m_maincpu->memory().space(AS_PROGRAM);
	int lines = m_200_256 ? 200 : 256;

	for (int y = 0; y < lines; y++)
	{
		offs_t addr = m_p_scrollram[y] << 1;

		for (int sx = 0; sx < 80; sx++)
		{
			UINT16 data = program->read_word(addr);

			if (m_40_80)
			{
				for (int x = 0; x < 8; x++)
				{
					int color = (BIT(data, 15) << 1) | BIT(data, 7);

					bitmap.pix16(y, (sx * 8) + x) = color;

					data <<= 1;
				}
			}
			else
			{
				for (int x = 0; x < 4; x++)
				{
					int color = (BIT(data, 15) << 3) | (BIT(data, 14) << 2) | (BIT(data, 7) << 1) | BIT(data, 6);

					bitmap.pix16(y, (sx * 8) + (x * 2)) = color;
					bitmap.pix16(y, (sx * 8) + (x * 2) + 1) = color;

					data <<= 2;
				}
			}

			addr += 2;
		}
	}

	return 0;
}

READ16_MEMBER( f1_state::palette_r )
{
	return m_p_paletteram[offset];
}

WRITE16_MEMBER( f1_state::palette_w )
{
	UINT8 i,r,g,b;
	COMBINE_DATA(&m_p_paletteram[offset]);

	if(ACCESSING_BITS_0_7 && offset) //TODO: offset 0 looks bogus
	{
		i = m_p_paletteram[offset] & 1;
		r = ((m_p_paletteram[offset] & 2)>>0) | i;
		g = ((m_p_paletteram[offset] & 4)>>1) | i;
		b = ((m_p_paletteram[offset] & 8)>>2) | i;

		palette_set_color_rgb(machine(), offset, pal2bit(r), pal2bit(g), pal2bit(b));
	}
}


static const gfx_layout charset_8x8 =
{
	8,8,
	256,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static GFXDECODE_START( act_f1 )
	GFXDECODE_ENTRY( I8086_TAG, 0x0800, charset_8x8, 0, 1 )
GFXDECODE_END


WRITE8_MEMBER( f1_state::system_w )
{
	switch(offset)
	{
	case 0: // centronics data port
		m_centronics->write(space, 0, data);
		break;

	case 1: // drive select
		wd17xx_set_drive(m_fdc, !BIT(data, 0));
		break;

	case 3: // drive head load
		break;

	case 5: // drive motor on
		floppy_mon_w(m_floppy0, !BIT(data, 0));
		break;

	case 7: // video lines (1=200, 0=256)
		m_200_256 = BIT(data, 0);
		break;

	case 9: // video columns (1=80, 0=40)
		m_40_80 = BIT(data, 0);
		break;

	case 0x0b: // LED 0 enable
		break;

	case 0x0d: // LED 1 enable
		break;

	case 0x0f: // centronics strobe output
		m_centronics->strobe_w(!BIT(data, 0));
		break;
	}
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( act_f1_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( act_f1_mem, AS_PROGRAM, 16, f1_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x01dff) AM_RAM
	AM_RANGE(0x01e00, 0x01fff) AM_RAM AM_SHARE("p_scrollram")
	AM_RANGE(0x02000, 0x3ffff) AM_RAM
	AM_RANGE(0xe0000, 0xe001f) AM_READWRITE(palette_r, palette_w) AM_SHARE("p_paletteram")
	AM_RANGE(0xf8000, 0xfffff) AM_ROM AM_REGION(I8086_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( act_f1_io )
//-------------------------------------------------

static ADDRESS_MAP_START( act_f1_io, AS_IO, 16, f1_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x000f) AM_WRITE8(system_w, 0xffff)
	AM_RANGE(0x0010, 0x0017) AM_DEVREADWRITE8(Z80CTC_TAG, z80ctc_device, read, write, 0x00ff)
	AM_RANGE(0x0020, 0x0027) AM_DEVREADWRITE8_LEGACY(Z80SIO2_TAG, z80dart_ba_cd_r, z80dart_ba_cd_w, 0x00ff)
//  AM_RANGE(0x0030, 0x0031) AM_WRITE8_LEGACY(ctc_ack_w, 0x00ff)
	AM_RANGE(0x0040, 0x0047) AM_DEVREADWRITE8_LEGACY(WD2797_TAG, wd17xx_r, wd17xx_w, 0x00ff)
//  AM_RANGE(0x01e0, 0x01ff) winchester
ADDRESS_MAP_END


//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( act )
//-------------------------------------------------

static INPUT_PORTS_START( act )
	// defined in machine/apricotkb.c
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  APRICOT_KEYBOARD_INTERFACE( kb_intf )
//-------------------------------------------------

static APRICOT_KEYBOARD_INTERFACE( kb_intf )
{
	DEVCB_NULL
};


//-------------------------------------------------
//  Z80DART_INTERFACE( sio_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( f1_state::sio_int_w )
{
	m_sio_int = state;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, m_ctc_int | m_sio_int);
}

static Z80DART_INTERFACE( sio_intf )
{
	0, 0, 0, 0,

	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_DRIVER_LINE_MEMBER(f1_state, sio_int_w)
};


//-------------------------------------------------
//  Z80CTC_INTERFACE( ctc_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( f1_state::ctc_int_w )
{
	m_ctc_int = state;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, m_ctc_int | m_sio_int);
}

WRITE_LINE_MEMBER( f1_state::ctc_z1_w )
{
	z80dart_rxcb_w(m_sio, state);
	z80dart_txcb_w(m_sio, state);
}

WRITE_LINE_MEMBER( f1_state::ctc_z2_w )
{
	z80dart_txca_w(m_sio, state);
}

static Z80CTC_INTERFACE( ctc_intf )
{
	DEVCB_DRIVER_LINE_MEMBER(f1_state, ctc_int_w),		// interrupt handler
	DEVCB_NULL,		// ZC/TO0 callback
	DEVCB_DRIVER_LINE_MEMBER(f1_state, ctc_z1_w),	// ZC/TO1 callback
	DEVCB_DRIVER_LINE_MEMBER(f1_state, ctc_z2_w),	// ZC/TO2 callback
};


//-------------------------------------------------
//  wd17xx_interface fdc_intf
//-------------------------------------------------

static LEGACY_FLOPPY_OPTIONS_START( act )
	LEGACY_FLOPPY_OPTION( img2hd, "dsk", "2HD disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([2])
		TRACKS([80])
		SECTORS([16])
		SECTOR_LENGTH([256])
		FIRST_SECTOR_ID([1]))
LEGACY_FLOPPY_OPTIONS_END

static const floppy_interface act_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_3_5_DSDD, // Sony OA-D32W
	LEGACY_FLOPPY_OPTIONS_NAME(act),
	"floppy_3_5",
	NULL
};

static const wd17xx_interface fdc_intf =
{
	DEVCB_NULL,
	DEVCB_CPU_INPUT_LINE(I8086_TAG, INPUT_LINE_NMI),
	DEVCB_CPU_INPUT_LINE(I8086_TAG, INPUT_LINE_TEST), // TODO inverted?
	{ FLOPPY_0, NULL, NULL, NULL }
};


//-------------------------------------------------
//  centronics_interface centronics_intf
//-------------------------------------------------

static const centronics_interface centronics_intf =
{
	DEVCB_NULL,
	DEVCB_DEVICE_LINE(Z80SIO2_TAG, z80dart_ctsa_w),
	DEVCB_NULL
};



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( act_f1 )
//-------------------------------------------------

static MACHINE_CONFIG_START( act_f1, f1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(I8086_TAG, I8086, XTAL_14MHz/4)
	MCFG_CPU_PROGRAM_MAP(act_f1_mem)
	MCFG_CPU_IO_MAP(act_f1_io)

	/* video hardware */
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(f1_state, screen_update)
	MCFG_SCREEN_SIZE(640, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 256-1)
	MCFG_PALETTE_LENGTH(16)
	MCFG_GFXDECODE(act_f1)

	/* Devices */
	MCFG_APRICOT_KEYBOARD_ADD(kb_intf)
	MCFG_Z80DART_ADD(Z80SIO2_TAG, 2500000, sio_intf)
	MCFG_Z80CTC_ADD(Z80CTC_TAG, 2500000, ctc_intf)
	MCFG_WD2797_ADD(WD2797_TAG, fdc_intf)
	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(act_floppy_interface)
	MCFG_CENTRONICS_PRINTER_ADD(CENTRONICS_TAG, centronics_intf)
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( f1 )
//-------------------------------------------------

ROM_START( f1 )
	ROM_REGION( 0x8000, I8086_TAG, 0 )
	ROM_LOAD16_BYTE( "lo_f1_1.6.8f",  0x0000, 0x4000, CRC(be018be2) SHA1(80b97f5b2111daf112c69b3f58d1541a4ba69da0) )	// Labelled F1 - LO Vr. 1.6
	ROM_LOAD16_BYTE( "hi_f1_1.6.10f", 0x0001, 0x4000, CRC(bbba77e2) SHA1(e62bed409eb3198f4848f85fccd171cd0745c7c0) )	// Labelled F1 - HI Vr. 1.6
ROM_END

#define rom_f1e rom_f1
#define rom_f2 rom_f1


//-------------------------------------------------
//  ROM( f10 )
//-------------------------------------------------

ROM_START( f10 )
	ROM_REGION( 0x8000, I8086_TAG, 0 )
	ROM_LOAD16_BYTE( "lo_f10_3.1.1.8f",  0x0000, 0x4000, CRC(bfd46ada) SHA1(0a36ef379fa9af7af9744b40c167ce6e12093485) )	// Labelled LO-FRange Vr3.1.1
	ROM_LOAD16_BYTE( "hi_f10_3.1.1.10f", 0x0001, 0x4000, CRC(67ad5b3a) SHA1(a5ececb87476a30167cf2a4eb35c03aeb6766601) )	// Labelled HI-FRange Vr3.1.1
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT   INIT     COMPANY             FULLNAME        FLAGS
COMP( 1984, f1,    0,      0,      act_f1,    act, driver_device,    0,     "ACT",   "Apricot F1",            GAME_NOT_WORKING | GAME_NO_SOUND )
COMP( 1984, f1e,   f1,     0,      act_f1,    act, driver_device,    0,     "ACT",   "Apricot F1e",           GAME_NOT_WORKING | GAME_NO_SOUND )
COMP( 1984, f2,    f1,     0,      act_f1,    act, driver_device,    0,     "ACT",   "Apricot F2",            GAME_NOT_WORKING | GAME_NO_SOUND )
COMP( 1985, f10,   f1,     0,      act_f1,    act, driver_device,    0,     "ACT",   "Apricot F10",           GAME_NOT_WORKING | GAME_NO_SOUND )
