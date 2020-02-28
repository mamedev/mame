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
#include "bus/centronics/ctronics.h"
#include "cpu/i86/i86.h"
#include "formats/apridisk.h"
#include "imagedev/floppy.h"
#include "machine/apricotkb.h"
#include "machine/buffer.h"
#include "machine/input_merger.h"
#include "machine/wd_fdc.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "emupal.h"
#include "screen.h"


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
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, I8086_TAG)
		, m_ctc(*this, Z80CTC_TAG)
		, m_sio(*this, Z80SIO2_TAG)
		, m_fdc(*this, WD2797_TAG)
		, m_floppy0(*this, WD2797_TAG ":0")
		, m_floppy1(*this, WD2797_TAG ":1")
		, m_centronics(*this, CENTRONICS_TAG)
		, m_cent_data_out(*this, "cent_data_out")
		, m_irqs(*this, "irqs")
		, m_p_scrollram(*this, "p_scrollram")
		, m_p_paletteram(*this, "p_paletteram")
		, m_palette(*this, "palette")
	{ }

	void act_f1(machine_config &config);

private:
	DECLARE_FLOPPY_FORMATS(floppy_formats);

	virtual void machine_start() override;

	required_device<cpu_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_device<z80sio_device> m_sio;
	required_device<wd2797_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_device<input_merger_device> m_irqs;
	required_shared_ptr<uint16_t> m_p_scrollram;
	required_shared_ptr<uint16_t> m_p_paletteram;
	required_device<palette_device> m_palette;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ16_MEMBER(palette_r);
	DECLARE_WRITE16_MEMBER(palette_w);
	DECLARE_WRITE8_MEMBER(system_w);
	DECLARE_WRITE_LINE_MEMBER(ctc_z1_w);
	DECLARE_WRITE_LINE_MEMBER(ctc_z2_w);
	DECLARE_WRITE8_MEMBER(m1_w);

	int m_40_80;
	int m_200_256;

	void act_f1_io(address_map &map);
	void act_f1_mem(address_map &map);
};


//**************************************************************************
//  VIDEO
//**************************************************************************

uint32_t f1_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	int lines = m_200_256 ? 200 : 256;

	for (int y = 0; y < lines; y++)
	{
		offs_t addr = m_p_scrollram[y] << 1;

		for (int sx = 0; sx < 80; sx++)
		{
			uint16_t data = program.read_word(addr);

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

READ16_MEMBER(f1_state::palette_r)
{
	return m_p_paletteram[offset];
}

WRITE16_MEMBER(f1_state::palette_w)
{
	uint8_t i,r,g,b;
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


static GFXDECODE_START( gfx_act_f1 )
	GFXDECODE_ENTRY( I8086_TAG, 0x0800, charset_8x8, 0, 1 )
GFXDECODE_END


WRITE8_MEMBER(f1_state::system_w)
{
	switch(offset)
	{
	case 0: // centronics data port
		m_cent_data_out->write(data);
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

void f1_state::act_f1_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x01dff).ram();
	map(0x01e00, 0x01fff).ram().share("p_scrollram");
	map(0x02000, 0x3ffff).ram();
	map(0xe0000, 0xe001f).rw(FUNC(f1_state::palette_r), FUNC(f1_state::palette_w)).share("p_paletteram");
	map(0xf8000, 0xfffff).rom().region(I8086_TAG, 0);
}


//-------------------------------------------------
//  ADDRESS_MAP( act_f1_io )
//-------------------------------------------------

void f1_state::act_f1_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x000f).w(FUNC(f1_state::system_w));
	map(0x0010, 0x0017).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write)).umask16(0x00ff);
	map(0x0020, 0x0027).rw(m_sio, FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w)).umask16(0x00ff);
	map(0x0030, 0x0030).w(FUNC(f1_state::m1_w));
	map(0x0040, 0x0047).rw(m_fdc, FUNC(wd2797_device::read), FUNC(wd2797_device::write)).umask16(0x00ff);
//  map(0x01e0, 0x01ff) winchester
}


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
//  Z80CTC
//-------------------------------------------------

WRITE_LINE_MEMBER(f1_state::ctc_z1_w)
{
	m_sio->rxcb_w(state);
	m_sio->txcb_w(state);
}

WRITE_LINE_MEMBER(f1_state::ctc_z2_w)
{
	m_sio->txca_w(state);
}

WRITE8_MEMBER(f1_state::m1_w)
{
	m_ctc->z80daisy_decode(data);
	m_sio->z80daisy_decode(data);
}

//-------------------------------------------------
//  floppy
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( f1_state::floppy_formats )
	FLOPPY_APRIDISK_FORMAT
FLOPPY_FORMATS_END

void apricotf_floppies(device_slot_interface &device)
{
	device.option_add("d31v", SONY_OA_D31V);
	device.option_add("d32w", SONY_OA_D32W);
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  machine_config( act_f1 )
//-------------------------------------------------

void f1_state::act_f1(machine_config &config)
{
	/* basic machine hardware */
	I8086(config, m_maincpu, 14_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &f1_state::act_f1_mem);
	m_maincpu->set_addrmap(AS_IO, &f1_state::act_f1_io);

	INPUT_MERGER_ANY_HIGH(config, "irqs").output_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	/* video hardware */
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(f1_state::screen_update));
	screen.set_size(640, 256);
	screen.set_visarea_full();
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(16);
	GFXDECODE(config, "gfxdecode", m_palette, gfx_act_f1);

	/* Devices */
	APRICOT_KEYBOARD(config, APRICOT_KEYBOARD_TAG, 0);

	Z80SIO(config, m_sio, 2500000);
	m_sio->out_int_callback().set("irqs", FUNC(input_merger_device::in_w<0>));

	Z80CTC(config, m_ctc, 2500000);
	m_ctc->intr_callback().set("irqs", FUNC(input_merger_device::in_w<1>));
	m_ctc->zc_callback<1>().set(FUNC(f1_state::ctc_z1_w));
	m_ctc->zc_callback<2>().set(FUNC(f1_state::ctc_z2_w));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(m_sio, FUNC(z80sio_device::ctsa_w));

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	// floppy
	WD2797(config, m_fdc, 4_MHz_XTAL / 2 /* ? */);
	m_fdc->intrq_wr_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_fdc->drq_wr_callback().set_inputline(m_maincpu, INPUT_LINE_TEST);

	FLOPPY_CONNECTOR(config, WD2797_TAG ":0", apricotf_floppies, "d32w", f1_state::floppy_formats);
	FLOPPY_CONNECTOR(config, WD2797_TAG ":1", apricotf_floppies, "d32w", f1_state::floppy_formats);
}



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

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS     INIT        COMPANY  FULLNAME       FLAGS
COMP( 1984, f1,   0,      0,      act_f1,  act,   f1_state, empty_init, "ACT",   "Apricot F1",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1984, f1e,  f1,     0,      act_f1,  act,   f1_state, empty_init, "ACT",   "Apricot F1e", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1984, f2,   f1,     0,      act_f1,  act,   f1_state, empty_init, "ACT",   "Apricot F2",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1985, f10,  f1,     0,      act_f1,  act,   f1_state, empty_init, "ACT",   "Apricot F10", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
