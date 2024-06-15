// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

    ACT Apricot FP


11/09/2011 - modernised. The portable doesn't seem to have
             scroll registers, and it sets the palette to black.

****************************************************************************/

/*

    TODO:

    - devices
    - LCD
    - sound

*/

#include "emu.h"
#include "bus/centronics/ctronics.h"
#include "cpu/i86/i86.h"
#include "cpu/m6800/m6801.h"
#include "formats/apridisk.h"
#include "imagedev/floppy.h"
#include "machine/am9517a.h"
#include "apricotkb.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "machine/z80sio.h"
#include "sound/sn76496.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "apricotp.lh"


namespace {

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I8086_TAG       "ic7"
#define I8284_TAG       "ic30"
#define I8259A_TAG      "ic51"
#define TMS4500_TAG     "ic42"
#define MC6845_TAG      "ic69"
#define HD63B01V1_TAG   "ic29"
#define AD7574_TAG      "ic34"
#define AD1408_TAG      "ic37"
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
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, I8086_TAG)
		, m_soundcpu(*this, HD63B01V1_TAG)
		, m_dmac(*this, "ic17")
		, m_pic(*this, I8259A_TAG)
		, m_pit(*this, "ic20")
		, m_sio(*this, "ic6")
		, m_fdc(*this, WD2797_TAG)
		, m_crtc(*this, MC6845_TAG)
		, m_ram(*this, RAM_TAG)
		, m_floppy0(*this, WD2797_TAG":0")
		, m_floppy1(*this, WD2797_TAG":1")
		, m_floppy(nullptr)
		, m_centronics(*this, CENTRONICS_TAG)
		, m_video_ram(*this, "video_ram", 0x20000, ENDIANNESS_LITTLE)
	{ }

	void fp(machine_config &config);

private:
	static void floppy_formats(format_registration &fr);

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<am9517a_device> m_dmac;
	required_device<pic8259_device> m_pic;
	required_device<pit8253_device> m_pit;
	required_device<z80sio_device> m_sio;
	required_device<wd2797_device> m_fdc;
	required_device<mc6845_device> m_crtc;
	required_device<ram_device> m_ram;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	floppy_image_device *m_floppy;
	required_device<centronics_device> m_centronics;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	MC6845_UPDATE_ROW(update_row);
	uint16_t mem_r(offs_t offset);
	void mem_w(offs_t offset, uint16_t data);
	uint8_t prtr_snd_r();
	void pint_clr_w(uint8_t data);
	[[maybe_unused]] void ls_w(uint8_t data);
	void contrast_w(uint8_t data);
	void palette_w(uint8_t data);
	void video_w(uint16_t data);
	[[maybe_unused]] void lat_w(offs_t offset, uint8_t data);

	void lat_ls259_w(offs_t offset, int state);

	uint16_t *m_work_ram;

	// video state
	memory_share_creator<uint16_t> m_video_ram;
	uint8_t m_video;

	int m_centronics_busy;
	int m_centronics_select;
	int m_centronics_fault;
	int m_centronics_perror;

	void write_centronics_busy(int state);
	void write_centronics_select(int state);
	void write_centronics_fault(int state);
	void write_centronics_perror(int state);

	void fp_io(address_map &map);
	void fp_mem(address_map &map);
};


//**************************************************************************
//  VIDEO
//**************************************************************************

void fp_state::video_start()
{
}

MC6845_UPDATE_ROW( fp_state::update_row )
{
}

uint32_t fp_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	offs_t addr = (!BIT(m_video, 4) << 15) | (BIT(m_video, 1) << 14);

	for (int y = 0; y < 200; y++)
	{
		for (int sx = 0; sx < 40; sx++)
		{
			uint16_t data = m_video_ram[addr++];

			for (int x = 0; x < 16; x++)
			{
				int color = BIT(data, 15);

				bitmap.pix(y, (sx * 16) + x) = color;

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


static GFXDECODE_START( gfx_act_f1 )
	GFXDECODE_ENTRY( I8086_TAG, 0x0800, charset_8x8, 0, 1 )
GFXDECODE_END


uint8_t fp_state::prtr_snd_r()
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

	uint8_t data = 0;

	// centronics
	data |= m_centronics_busy;
	data |= m_centronics_select << 1;
	data |= m_centronics_fault << 2;
	data |= m_centronics_perror << 3;

	// floppy
	data |= (m_floppy ? m_floppy->dskchg_r() : 1) << 5;

	return data;
}

void fp_state::pint_clr_w(uint8_t data)
{
	m_pic->ir6_w(CLEAR_LINE);
}


void fp_state::ls_w(uint8_t data)
{
	m_centronics->write_strobe(!BIT(data, 0));
}


void fp_state::contrast_w(uint8_t data)
{
}


void fp_state::palette_w(uint8_t data)
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


void fp_state::video_w(uint16_t data)
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
			m_floppy = nullptr;

			if (state) m_floppy = m_floppy0->get_device();
			else m_floppy = m_floppy1->get_device();

			m_fdc->set_floppy(m_floppy);

			if (m_floppy)
				m_floppy->mon_w(0);
		}
		break;
	}
}

void fp_state::lat_w(offs_t offset, uint8_t data)
{
	lat_ls259_w((offset >> 1) & 0x07, BIT(data, 0));
}


uint16_t fp_state::mem_r(offs_t offset)
{
	uint16_t data = 0xffff;

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


void fp_state::mem_w(offs_t offset, uint16_t data)
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

void fp_state::fp_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0xf7fff).rw(FUNC(fp_state::mem_r), FUNC(fp_state::mem_w));
	map(0xf8000, 0xfffff).rom().region(I8086_TAG, 0);
}


//-------------------------------------------------
//  ADDRESS_MAP( fp_io )
//-------------------------------------------------

void fp_state::fp_io(address_map &map)
{
	map.unmap_value_high();
	map(0x000, 0x007).rw(m_fdc, FUNC(wd2797_device::read), FUNC(wd2797_device::write)).umask16(0x00ff);
	map(0x008, 0x00f).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);
	map(0x018, 0x01f).rw(m_sio, FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w)).umask16(0x00ff);
	map(0x020, 0x020).w("cent_data_out", FUNC(output_latch_device::write));
	map(0x022, 0x022).w(FUNC(fp_state::pint_clr_w));
	map(0x024, 0x024).r(FUNC(fp_state::prtr_snd_r));
	map(0x026, 0x026).w(SN76489AN_TAG, FUNC(sn76489a_device::write));
	map(0x028, 0x028).w(FUNC(fp_state::contrast_w));
	map(0x02a, 0x02a).w(FUNC(fp_state::palette_w));
	map(0x02e, 0x02f).w(FUNC(fp_state::video_w));
	map(0x040, 0x05f).rw(m_dmac, FUNC(am9517a_device::read), FUNC(am9517a_device::write)).umask16(0x00ff);
	map(0x068, 0x06b).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x06c, 0x06c).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0x06e, 0x06e).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
}



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

void fp_state::write_centronics_busy(int state)
{
	m_centronics_busy = state;
	if (!state) m_pic->ir6_w(ASSERT_LINE);
}

void fp_state::write_centronics_select(int state)
{
	m_centronics_select = state;
}

void fp_state::write_centronics_fault(int state)
{
	m_centronics_fault = state;
}

void fp_state::write_centronics_perror(int state)
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
	m_work_ram = reinterpret_cast<uint16_t *>(m_ram->pointer());
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

void fp_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();

	fr.add(FLOPPY_APRIDISK_FORMAT);
}

//-------------------------------------------------
//  machine_config( fp )
//-------------------------------------------------

void fp_state::fp(machine_config &config)
{
	/* basic machine hardware */
	I8086(config, m_maincpu, 15_MHz_XTAL / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &fp_state::fp_mem);
	m_maincpu->set_addrmap(AS_IO, &fp_state::fp_io);
	m_maincpu->set_irq_acknowledge_callback(I8259A_TAG, FUNC(pic8259_device::inta_cb));

	HD6301V1(config, m_soundcpu, 2000000);
	m_soundcpu->set_disable();

	/* video hardware */
	config.set_default_layout(layout_apricotp);

	screen_device &screen_lcd(SCREEN(config, SCREEN_LCD_TAG, SCREEN_TYPE_RASTER));
	screen_lcd.set_refresh_hz(50);
	screen_lcd.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen_lcd.set_screen_update(FUNC(fp_state::screen_update));
	screen_lcd.set_size(640, 200);
	screen_lcd.set_visarea_full();
	screen_lcd.set_palette("palette");

	screen_device &screen_crt(SCREEN(config, SCREEN_CRT_TAG, SCREEN_TYPE_RASTER));
	screen_crt.set_refresh_hz(50);
	screen_crt.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen_crt.set_screen_update(MC6845_TAG, FUNC(mc6845_device::screen_update));
	screen_crt.set_size(640, 256);
	screen_crt.set_visarea_full();

	PALETTE(config, "palette").set_entries(16);
	GFXDECODE(config, "gfxdecode", "palette", gfx_act_f1);

	MC6845(config, m_crtc, 4000000);
	m_crtc->set_screen(SCREEN_CRT_TAG);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(fp_state::update_row));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SN76489A(config, SN76489AN_TAG, 2000000).add_route(ALL_OUTPUTS, "mono", 1.00);

	/* Devices */
	APRICOT_KEYBOARD(config, APRICOT_KEYBOARD_TAG, 0);

	AM9517A(config, m_dmac, 250000);
	m_dmac->out_eop_callback().set(m_pic, FUNC(pic8259_device::ir7_w));
	m_dmac->in_ior_callback<1>().set(m_fdc, FUNC(wd2797_device::data_r));
	m_dmac->out_iow_callback<1>().set(m_fdc, FUNC(wd2797_device::data_w));

	PIC8259(config, m_pic, 0);
	m_pic->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	PIT8253(config, m_pit, 0);
	m_pit->set_clk<0>(2000000);
	m_pit->out_handler<0>().set(m_pic, FUNC(pic8259_device::ir0_w));
	m_pit->set_clk<1>(2000000);
	m_pit->set_clk<2>(2000000);

	Z80SIO(config, m_sio, 2500000);
	m_sio->out_int_callback().set(m_pic, FUNC(pic8259_device::ir4_w));

	WD2797(config, m_fdc, 2000000);
	m_fdc->intrq_wr_callback().set(m_pic, FUNC(pic8259_device::ir1_w));
	m_fdc->drq_wr_callback().set(m_dmac, FUNC(am9517a_device::dreq1_w));

	FLOPPY_CONNECTOR(config, m_floppy0, "d32w", SONY_OA_D32W, true,  floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy1, "d32w", SONY_OA_D32W, false, floppy_formats);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(fp_state::write_centronics_busy));
	m_centronics->select_handler().set(FUNC(fp_state::write_centronics_select));
	m_centronics->fault_handler().set(FUNC(fp_state::write_centronics_fault));
	m_centronics->perror_handler().set(FUNC(fp_state::write_centronics_perror));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("256K").set_extra_options("512K,1M");
}



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

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS     INIT        COMPANY  FULLNAME                 FLAGS
COMP( 1984, fp,   0,      0,      fp,      fp,    fp_state, empty_init, "ACT",   "Apricot Portable / FP", MACHINE_NOT_WORKING )
