// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

    ACT Apricot FP

    preliminary driver by Angelo Salese


11/09/2011 - modernised. The portable doesn't seem to have
             scroll registers, and it sets the palette to black.
             I've added a temporary video output so that you can get
             an idea of what the screen should look like. [Robbbert]

****************************************************************************/

/*

    TODO:

    - devices
    - LCD
    - sound

*/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "cpu/m6800/m6800.h"
#include "machine/am9517a.h"
#include "machine/apricotkb.h"
#include "bus/centronics/ctronics.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "machine/z80dart.h"
#include "sound/sn76496.h"
#include "video/mc6845.h"
#include "formats/apridisk.h"
#include "apricotp.lh"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I8086_TAG       "ic7"
#define I8284_TAG       "ic30"
#define I8237_TAG       "ic17"
#define I8259A_TAG      "ic51"
#define I8253A5_TAG     "ic20"
#define TMS4500_TAG     "ic42"
#define MC6845_TAG      "ic69"
#define HD63B01V1_TAG   "ic29"
#define AD7574_TAG      "ic34"
#define AD1408_TAG      "ic37"
#define Z80SIO0_TAG     "ic6"
#define WD2797_TAG      "ic5"
#define SN76489AN_TAG   "ic13"
#define CENTRONICS_TAG  "centronics"
#define SCREEN_LCD_TAG  "screen0"
#define SCREEN_CRT_TAG  "screen1"

enum
{
	LED_STOP = 0,
	LED_POWER,
	LED_SHIFT_LOCK,
	LED_DISK,
	LED_VOICE,
	LED_COLOUR_SELECT,
	LED_CAPS_LOCK
};


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> fp_state

class fp_state : public driver_device
{
public:
	fp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, I8086_TAG),
			m_soundcpu(*this, HD63B01V1_TAG),
			m_dmac(*this, I8237_TAG),
			m_pic(*this, I8259A_TAG),
			m_pit(*this, I8253A5_TAG),
			m_sio(*this, Z80SIO0_TAG),
			m_fdc(*this, WD2797_TAG),
			m_crtc(*this, MC6845_TAG),
			m_ram(*this, RAM_TAG),
			m_floppy0(*this, WD2797_TAG":0"),
			m_floppy1(*this, WD2797_TAG":1"),
			m_floppy(NULL),
			m_centronics(*this, CENTRONICS_TAG),
			m_work_ram(*this, "work_ram"),
			m_video_ram(*this, "video_ram")
	{ }

	DECLARE_FLOPPY_FORMATS(floppy_formats);

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<am9517a_device> m_dmac;
	required_device<pic8259_device> m_pic;
	required_device<pit8253_device> m_pit;
	required_device<z80dart_device> m_sio;
	required_device<wd2797_t> m_fdc;
	required_device<mc6845_device> m_crtc;
	required_device<ram_device> m_ram;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	floppy_image_device *m_floppy;
	required_device<centronics_device> m_centronics;

	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	MC6845_UPDATE_ROW(update_row);
	DECLARE_READ16_MEMBER( mem_r );
	DECLARE_WRITE16_MEMBER( mem_w );
	DECLARE_READ8_MEMBER( prtr_snd_r );
	DECLARE_WRITE8_MEMBER( pint_clr_w );
	DECLARE_WRITE8_MEMBER( ls_w );
	DECLARE_WRITE8_MEMBER( contrast_w );
	DECLARE_WRITE8_MEMBER( palette_w );
	DECLARE_WRITE16_MEMBER( video_w );
	DECLARE_WRITE8_MEMBER( lat_w );

	void lat_ls259_w(offs_t offset, int state);

	optional_shared_ptr<UINT16> m_work_ram;

	// video state
	optional_shared_ptr<UINT16> m_video_ram;
	UINT8 m_video;

	int m_centronics_busy;
	int m_centronics_select;
	int m_centronics_fault;
	int m_centronics_perror;

	DECLARE_WRITE_LINE_MEMBER( write_centronics_busy );
	DECLARE_WRITE_LINE_MEMBER( write_centronics_select );
	DECLARE_WRITE_LINE_MEMBER( write_centronics_fault );
	DECLARE_WRITE_LINE_MEMBER( write_centronics_perror );
};


//**************************************************************************
//  VIDEO
//**************************************************************************

void fp_state::video_start()
{
	// allocate memory
	m_video_ram.allocate(0x20000);
}

MC6845_UPDATE_ROW( fp_state::update_row )
{
}

UINT32 fp_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	offs_t addr = (!BIT(m_video, 4) << 15) | (BIT(m_video, 1) << 14);

	for (int y = 0; y < 200; y++)
	{
		for (int sx = 0; sx < 40; sx++)
		{
			UINT16 data = m_video_ram[addr++];

			for (int x = 0; x < 16; x++)
			{
				int color = BIT(data, 15);

				bitmap.pix16(y, (sx * 16) + x) = color;

				data <<= 1;
			}
		}
	}

	return 0;
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


READ8_MEMBER( fp_state::prtr_snd_r )
{
	/*

	    bit     description

	    0       BUSY
	    1       SE
	    2       _FAULT
	    3       PE
	    4       LP23
	    5       DCNG-L
	    6       J9 1-2
	    7       J9 3-4

	*/

	UINT8 data = 0;

	// centronics
	data |= m_centronics_busy;
	data |= m_centronics_select << 1;
	data |= m_centronics_fault << 2;
	data |= m_centronics_perror << 3;

	// floppy
	data |= (m_floppy ? m_floppy->dskchg_r() : 1) << 5;

	return data;
}

WRITE8_MEMBER( fp_state::pint_clr_w )
{
	m_pic->ir6_w(CLEAR_LINE);
}


WRITE8_MEMBER( fp_state::ls_w )
{
	m_centronics->write_strobe(!BIT(data, 0));
}


WRITE8_MEMBER( fp_state::contrast_w )
{
}


WRITE8_MEMBER( fp_state::palette_w )
{
	/*

	    bit     description

	    0       B
	    1       G
	    2       R
	    3       I
	    4       index
	    5       index
	    6       index
	    7       index

	*/
}


WRITE16_MEMBER( fp_state::video_w )
{
	/*

	    bit     description

	    0       CRTRES-H
	    1       SEL1
	    2       DON-H
	    3       LCDON-H
	    4       SEL2
	    5       L3 even access
	    6       L2 odd access
	    7       L1 video RAM enable
	    8
	    9       STOP LED
	    10      POWER LED
	    11      SHIFT LOCK LED
	    12      DISK LED
	    13      VOICE LED
	    14      COLOUR SELECT LED
	    15      CAPS LOCK LED

	*/

	m_video = data & 0xff;
}

void fp_state::lat_ls259_w(offs_t offset, int state)
{
	switch (offset)
	{
	case 0:
		{
			m_floppy = NULL;

			if (state) m_floppy = m_floppy0->get_device();
			else m_floppy = m_floppy1->get_device();

			m_fdc->set_floppy(m_floppy);

			if (m_floppy)
				m_floppy->mon_w(0);
		}
		break;
	}
}

WRITE8_MEMBER( fp_state::lat_w )
{
	lat_ls259_w((offset >> 1) & 0x07, BIT(data, 0));
}


READ16_MEMBER( fp_state::mem_r )
{
	UINT16 data = 0xffff;

	if (offset >= 0xd0000/2 && offset < 0xf0000/2)
	{
		if (BIT(m_video, 7))
		{
			data = m_video_ram[offset - 0xd0000/2];
		}
		else
		{
			if (offset < m_ram->size()/2)
			{
				data = m_work_ram[offset];
			}
		}
	}
	else
	{
		if (offset < m_ram->size()/2)
		{
			data = m_work_ram[offset];
		}
	}

	return data;
}


WRITE16_MEMBER( fp_state::mem_w )
{
	if (offset >= 0xd0000/2 && offset < 0xe0000/2)
	{
		if (BIT(m_video, 7))
		{
			m_video_ram[offset - 0xd0000/2] = data;
		}
		else
		{
			if (offset < m_ram->size()/2)
			{
				m_work_ram[offset] = data;
			}
		}
	}
	else if (offset >= 0xe0000/2 && offset < 0xf0000/2)
	{
		if (BIT(m_video, 7))
		{
			if (BIT(m_video, 5))
			{
				m_video_ram[offset - 0xd0000/2] = (data & 0xff00) | (m_video_ram[offset - 0xd0000/2] & 0x00ff);
			}

			if (BIT(m_video, 6))
			{
				m_video_ram[offset - 0xd0000/2] = (data & 0x00ff) | (m_video_ram[offset - 0xd0000/2] & 0xff00);
			}
		}
		else
		{
			if (offset < m_ram->size()/2)
			{
				m_work_ram[offset] = data;
			}
		}
	}
	else
	{
		if (offset < m_ram->size()/2)
		{
			m_work_ram[offset] = data;
		}
	}
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( fp_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( fp_mem, AS_PROGRAM, 16, fp_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0xf7fff) AM_READWRITE(mem_r, mem_w)
	AM_RANGE(0xf8000, 0xfffff) AM_ROM AM_REGION(I8086_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( fp_io )
//-------------------------------------------------

static ADDRESS_MAP_START( fp_io, AS_IO, 16, fp_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000, 0x007) AM_DEVREADWRITE8(WD2797_TAG, wd2797_t, read, write, 0x00ff)
	AM_RANGE(0x008, 0x00f) AM_DEVREADWRITE8(I8253A5_TAG, pit8253_device, read, write, 0x00ff)
	AM_RANGE(0x018, 0x01f) AM_DEVREADWRITE8(Z80SIO0_TAG, z80sio0_device, ba_cd_r, ba_cd_w, 0x00ff)
	AM_RANGE(0x020, 0x021) AM_DEVWRITE8("cent_data_out", output_latch_device, write, 0x00ff)
	AM_RANGE(0x022, 0x023) AM_WRITE8(pint_clr_w, 0x00ff)
	AM_RANGE(0x024, 0x025) AM_READ8(prtr_snd_r, 0x00ff)
	AM_RANGE(0x026, 0x027) AM_DEVWRITE8(SN76489AN_TAG, sn76489a_device, write, 0x00ff)
	AM_RANGE(0x028, 0x029) AM_WRITE8(contrast_w, 0x00ff)
	AM_RANGE(0x02a, 0x02b) AM_WRITE8(palette_w, 0x00ff)
	AM_RANGE(0x02e, 0x02f) AM_WRITE(video_w)
	AM_RANGE(0x040, 0x05f) AM_DEVREADWRITE8(I8237_TAG, am9517a_device, read, write, 0x00ff)
	AM_RANGE(0x068, 0x06b) AM_DEVREADWRITE8(I8259A_TAG, pic8259_device, read, write, 0x00ff)
	AM_RANGE(0x06c, 0x06d) AM_DEVWRITE8(MC6845_TAG, mc6845_device, address_w, 0x00ff)
	AM_RANGE(0x06e, 0x06f) AM_DEVREADWRITE8(MC6845_TAG, mc6845_device, register_r, register_w, 0x00ff)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( sound_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( sound_mem, AS_PROGRAM, 8, fp_state )
	AM_RANGE(0xf000, 0xffff) AM_ROM AM_REGION(HD63B01V1_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( sound_io )
//-------------------------------------------------

static ADDRESS_MAP_START( sound_io, AS_IO, 8, fp_state )
	AM_RANGE(M6801_PORT1, M6801_PORT1)
	AM_RANGE(M6801_PORT2, M6801_PORT2)
	AM_RANGE(M6801_PORT3, M6801_PORT3)
	AM_RANGE(M6801_PORT4, M6801_PORT4)
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( act )
//-------------------------------------------------

static INPUT_PORTS_START( fp )
	// defined in machine/apricotkb.c
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  pic8259_interface pic_intf
//-------------------------------------------------

/*

    INT0    TIMER
    INT1    FDC
    INT2    6301
    INT3    COMS
    INT4    USART
    INT5    COMS
    INT6    PRINT
    INT7    EOP

*/

WRITE_LINE_MEMBER( fp_state::write_centronics_busy )
{
	m_centronics_busy = state;
	if (!state) m_pic->ir6_w(ASSERT_LINE);
}

WRITE_LINE_MEMBER( fp_state::write_centronics_select )
{
	m_centronics_select = state;
}

WRITE_LINE_MEMBER( fp_state::write_centronics_fault )
{
	m_centronics_fault = state;
}

WRITE_LINE_MEMBER( fp_state::write_centronics_perror )
{
	m_centronics_perror = state;
}

//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( fp )
//-------------------------------------------------

void fp_state::machine_start()
{
	// allocate memory
	m_work_ram.allocate(m_ram->size() / 2);
}


void fp_state::machine_reset()
{
	m_video = 0;

	m_fdc->dden_w(0);

	for (offs_t offset = 0; offset < 7; offset++)
	{
		lat_ls259_w(offset, 0);
	}
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

FLOPPY_FORMATS_MEMBER( fp_state::floppy_formats )
	FLOPPY_APRIDISK_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( fp_floppies )
	SLOT_INTERFACE("d32w", SONY_OA_D32W)
SLOT_INTERFACE_END


//-------------------------------------------------
//  MACHINE_CONFIG( fp )
//-------------------------------------------------

static MACHINE_CONFIG_START( fp, fp_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(I8086_TAG, I8086, XTAL_15MHz/3)
	MCFG_CPU_PROGRAM_MAP(fp_mem)
	MCFG_CPU_IO_MAP(fp_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE(I8259A_TAG, pic8259_device, inta_cb)

	MCFG_CPU_ADD(HD63B01V1_TAG, HD6301, 2000000)
	MCFG_CPU_PROGRAM_MAP(sound_mem)
	MCFG_CPU_IO_MAP(sound_io)
	MCFG_DEVICE_DISABLE()

	/* video hardware */
	MCFG_DEFAULT_LAYOUT( layout_apricotp )

	MCFG_SCREEN_ADD(SCREEN_LCD_TAG, LCD)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(fp_state, screen_update)
	MCFG_SCREEN_SIZE(640, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 200-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_SCREEN_ADD(SCREEN_CRT_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DEVICE(MC6845_TAG, mc6845_device, screen_update)
	MCFG_SCREEN_SIZE(640, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 256-1)

	MCFG_PALETTE_ADD("palette", 16)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", act_f1)

	MCFG_MC6845_ADD(MC6845_TAG, MC6845, SCREEN_CRT_TAG, 4000000)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(fp_state, update_row)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SN76489AN_TAG, SN76489A, 2000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* Devices */
	MCFG_DEVICE_ADD(APRICOT_KEYBOARD_TAG, APRICOT_KEYBOARD, 0)
	MCFG_DEVICE_ADD(I8237_TAG, AM9517A, 250000)
	MCFG_I8237_OUT_EOP_CB(DEVWRITELINE(I8259A_TAG, pic8259_device, ir7_w))
	MCFG_I8237_IN_IOR_1_CB(DEVREAD8(WD2797_TAG, wd_fdc_t, data_r))
	MCFG_I8237_OUT_IOW_1_CB(DEVWRITE8(WD2797_TAG, wd_fdc_t, data_w))
	MCFG_PIC8259_ADD(I8259A_TAG, INPUTLINE(I8086_TAG, INPUT_LINE_IRQ0), VCC, NULL)

	MCFG_DEVICE_ADD(I8253A5_TAG, PIT8253, 0)
	MCFG_PIT8253_CLK0(2000000)
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE(I8259A_TAG, pic8259_device, ir0_w))
	MCFG_PIT8253_CLK1(2000000)
	MCFG_PIT8253_CLK2(2000000)

	MCFG_Z80SIO0_ADD(Z80SIO0_TAG, 2500000, 0, 0, 0, 0)
	MCFG_Z80DART_OUT_INT_CB(DEVWRITELINE(I8259A_TAG, pic8259_device, ir4_w))

	MCFG_WD2797_ADD(WD2797_TAG, 2000000)
	MCFG_WD_FDC_INTRQ_CALLBACK(DEVWRITELINE(I8259A_TAG, pic8259_device, ir1_w))
	MCFG_WD_FDC_DRQ_CALLBACK(DEVWRITELINE(I8237_TAG, am9517a_device, dreq1_w))

	MCFG_FLOPPY_DRIVE_ADD(WD2797_TAG ":0", fp_floppies, "d32w", fp_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(WD2797_TAG ":1", fp_floppies, NULL,   fp_state::floppy_formats)

	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(fp_state, write_centronics_busy))
	MCFG_CENTRONICS_SELECT_HANDLER(WRITELINE(fp_state, write_centronics_select))
	MCFG_CENTRONICS_FAULT_HANDLER(WRITELINE(fp_state, write_centronics_fault))
	MCFG_CENTRONICS_PERROR_HANDLER(WRITELINE(fp_state, write_centronics_perror))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", CENTRONICS_TAG)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("256K")
	MCFG_RAM_EXTRA_OPTIONS("512K,1M")
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( fp )
//-------------------------------------------------

ROM_START( fp )
	ROM_REGION( 0x8000, I8086_TAG, 0 )
	ROM_LOAD16_BYTE( "lo_fp_3.1.ic20", 0x0000, 0x4000, CRC(0572add2) SHA1(c7ab0e5ced477802e37f9232b5673f276b8f5623) )   // Labelled 11212721 F97E PORT LO VR 3.1
	ROM_LOAD16_BYTE( "hi_fp_3.1.ic9",  0x0001, 0x4000, CRC(3903674b) SHA1(8418682dcc0c52416d7d851760fea44a3cf2f914) )   // Labelled 11212721 BD2D PORT HI VR 3.1

	ROM_REGION( 0x1000, HD63B01V1_TAG, 0 )
	ROM_LOAD( "voice interface hd63b01v01.ic29", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "tbp24s10.ic73", 0x000, 0x100, NO_DUMP ) // address decoder 256x4

	ROM_REGION( 0x100, "plds", 0 )
	ROM_LOAD( "pal1 pal12l6.ic2", 0x000, 0x100, NO_DUMP ) // ?
	ROM_LOAD( "pal2 pal10l8.ic35", 0x000, 0x100, NO_DUMP ) // address decoder
	ROM_LOAD( "pal3 pal12l6.ic77", 0x000, 0x100, NO_DUMP ) // ?
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT   INIT     COMPANY             FULLNAME        FLAGS
COMP( 1984, fp,    0,      0,      fp,   fp, driver_device,    0,     "ACT",   "Apricot Portable / FP", MACHINE_NOT_WORKING )
