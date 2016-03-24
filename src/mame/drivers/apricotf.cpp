// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Robbbert
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

#include "emu.h"
#include "cpu/i86/i86.h"
#include "cpu/z80/z80daisy.h"
#include "imagedev/flopdrv.h"
#include "machine/apricotkb.h"
#include "machine/buffer.h"
#include "bus/centronics/ctronics.h"
#include "machine/wd_fdc.h"
#include "machine/z80ctc.h"
#include "machine/z80dart.h"
#include "formats/apridisk.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define SCREEN_TAG      "screen"
#define I8086_TAG       "10d"
#define Z80CTC_TAG      "13d"
#define Z80SIO2_TAG     "15d"
#define WD2797_TAG      "5f"
#define CENTRONICS_TAG  "centronics"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> f1_state

class f1_state : public driver_device
{
public:
	f1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, I8086_TAG),
			m_ctc(*this, Z80CTC_TAG),
			m_sio(*this, Z80SIO2_TAG),
			m_fdc(*this, WD2797_TAG),
			m_floppy0(*this, WD2797_TAG ":0"),
			m_floppy1(*this, WD2797_TAG ":1"),
			m_centronics(*this, CENTRONICS_TAG),
			m_cent_data_out(*this, "cent_data_out"),
			m_ctc_int(CLEAR_LINE),
			m_sio_int(CLEAR_LINE),
			m_p_scrollram(*this, "p_scrollram"),
			m_p_paletteram(*this, "p_paletteram"),
			m_palette(*this, "palette")
	{ }

	DECLARE_FLOPPY_FORMATS(floppy_formats);

	virtual void machine_start() override;

	required_device<cpu_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_device<z80sio2_device> m_sio;
	required_device<wd2797_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	int m_ctc_int;
	int m_sio_int;
	required_shared_ptr<UINT16> m_p_scrollram;
	required_shared_ptr<UINT16> m_p_paletteram;
	required_device<palette_device> m_palette;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ16_MEMBER( palette_r );
	DECLARE_WRITE16_MEMBER( palette_w );
	DECLARE_WRITE8_MEMBER( system_w );
	DECLARE_WRITE_LINE_MEMBER( sio_int_w );
	DECLARE_WRITE_LINE_MEMBER( ctc_int_w );
	DECLARE_WRITE_LINE_MEMBER( ctc_z1_w );
	DECLARE_WRITE_LINE_MEMBER( ctc_z2_w );

	int m_40_80;
	int m_200_256;
};


//**************************************************************************
//  VIDEO
//**************************************************************************

UINT32 f1_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	int lines = m_200_256 ? 200 : 256;

	for (int y = 0; y < lines; y++)
	{
		offs_t addr = m_p_scrollram[y] << 1;

		for (int sx = 0; sx < 80; sx++)
		{
			UINT16 data = program.read_word(addr);

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

		m_palette->set_pen_color(offset, pal2bit(r), pal2bit(g), pal2bit(b));
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
		m_cent_data_out->write(space, 0, data);
		break;

	case 1: // drive select
		m_fdc->set_floppy(BIT(data, 0) ? m_floppy0->get_device() : m_floppy1->get_device());
		break;

	case 3: // drive head load
		break;

	case 5: // drive motor on
		m_floppy0->get_device()->mon_w(!BIT(data, 0));
		m_floppy1->get_device()->mon_w(!BIT(data, 0));
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
		m_centronics->write_strobe(!BIT(data, 0));
		break;
	}
}


void f1_state::machine_start()
{
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
	AM_RANGE(0x0020, 0x0027) AM_DEVREADWRITE8(Z80SIO2_TAG, z80sio2_device, ba_cd_r, ba_cd_w, 0x00ff)
//  AM_RANGE(0x0030, 0x0031) AM_WRITE8(ctc_ack_w, 0x00ff)
	AM_RANGE(0x0040, 0x0047) AM_DEVREADWRITE8(WD2797_TAG, wd2797_t, read, write, 0x00ff)
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
//  Z80SIO
//-------------------------------------------------

WRITE_LINE_MEMBER( f1_state::sio_int_w )
{
	m_sio_int = state;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, m_ctc_int || m_sio_int);
}

//-------------------------------------------------
//  Z80CTC
//-------------------------------------------------

WRITE_LINE_MEMBER( f1_state::ctc_int_w )
{
	m_ctc_int = state;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, m_ctc_int | m_sio_int);
}

WRITE_LINE_MEMBER( f1_state::ctc_z1_w )
{
	m_sio->rxcb_w(state);
	m_sio->txcb_w(state);
}

WRITE_LINE_MEMBER( f1_state::ctc_z2_w )
{
	m_sio->txca_w(state);
}

//-------------------------------------------------
//  floppy
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( f1_state::floppy_formats )
	FLOPPY_APRIDISK_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( apricotf_floppies )
	SLOT_INTERFACE( "d31v", SONY_OA_D31V )
	SLOT_INTERFACE( "d32w", SONY_OA_D32W )
SLOT_INTERFACE_END



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
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 16)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", act_f1)

	/* Devices */
	MCFG_DEVICE_ADD(APRICOT_KEYBOARD_TAG, APRICOT_KEYBOARD, 0)

	MCFG_Z80SIO2_ADD(Z80SIO2_TAG, 2500000, 0, 0, 0, 0)
	MCFG_Z80DART_OUT_INT_CB(WRITELINE(f1_state, sio_int_w))

	MCFG_DEVICE_ADD(Z80CTC_TAG, Z80CTC, 2500000)
	MCFG_Z80CTC_INTR_CB(WRITELINE(f1_state, ctc_int_w))
	MCFG_Z80CTC_ZC1_CB(WRITELINE(f1_state, ctc_z1_w))
	MCFG_Z80CTC_ZC2_CB(WRITELINE(f1_state, ctc_z2_w))

	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(DEVWRITELINE(Z80SIO2_TAG, z80dart_device, ctsa_w))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", CENTRONICS_TAG)

	// floppy
	MCFG_WD2797_ADD(WD2797_TAG, XTAL_4MHz / 2 /* ? */)
	MCFG_WD_FDC_INTRQ_CALLBACK(INPUTLINE(I8086_TAG, INPUT_LINE_NMI))
	MCFG_WD_FDC_DRQ_CALLBACK(INPUTLINE(I8086_TAG, INPUT_LINE_TEST))

	MCFG_FLOPPY_DRIVE_ADD(WD2797_TAG ":0", apricotf_floppies, "d32w", f1_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(WD2797_TAG ":1", apricotf_floppies, "d32w", f1_state::floppy_formats)
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( f1 )
//-------------------------------------------------

ROM_START( f1 )
	ROM_REGION( 0x8000, I8086_TAG, 0 )
	ROM_LOAD16_BYTE( "lo_f1_1.6.8f",  0x0000, 0x4000, CRC(be018be2) SHA1(80b97f5b2111daf112c69b3f58d1541a4ba69da0) )    // Labelled F1 - LO Vr. 1.6
	ROM_LOAD16_BYTE( "hi_f1_1.6.10f", 0x0001, 0x4000, CRC(bbba77e2) SHA1(e62bed409eb3198f4848f85fccd171cd0745c7c0) )    // Labelled F1 - HI Vr. 1.6
ROM_END

#define rom_f1e rom_f1
#define rom_f2 rom_f1


//-------------------------------------------------
//  ROM( f10 )
//-------------------------------------------------

ROM_START( f10 )
	ROM_REGION( 0x8000, I8086_TAG, 0 )
	ROM_LOAD16_BYTE( "lo_f10_3.1.1.8f",  0x0000, 0x4000, CRC(bfd46ada) SHA1(0a36ef379fa9af7af9744b40c167ce6e12093485) ) // Labelled LO-FRange Vr3.1.1
	ROM_LOAD16_BYTE( "hi_f10_3.1.1.10f", 0x0001, 0x4000, CRC(67ad5b3a) SHA1(a5ececb87476a30167cf2a4eb35c03aeb6766601) ) // Labelled HI-FRange Vr3.1.1
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT   INIT     COMPANY             FULLNAME        FLAGS
COMP( 1984, f1,    0,      0,      act_f1,    act, driver_device,    0,     "ACT",   "Apricot F1",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1984, f1e,   f1,     0,      act_f1,    act, driver_device,    0,     "ACT",   "Apricot F1e",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1984, f2,    f1,     0,      act_f1,    act, driver_device,    0,     "ACT",   "Apricot F2",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1985, f10,   f1,     0,      act_f1,    act, driver_device,    0,     "ACT",   "Apricot F10",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
