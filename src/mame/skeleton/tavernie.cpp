// license:BSD-3-Clause
// copyright-holders: Robbbert
/*********************************************************************************

Tavernier CPU09 and IVG09 (Realisez votre ordinateur individuel)

2013-12-08 Skeleton driver.

This system was described in a French magazine "Le Haut-Parleur".

CPU09 includes 6809, 6821, 6840, 6850, cassette, rs232
IVG09 includes 6845, another 6821, beeper
IFD09 includes WD1795

ToDo:
- Graphics
- Character rom is not dumped
- Graphics rom is not dumped
        (note 2020-05-29: added what are thought to be the correct roms, but the proper
                          operation is uncertain).
- 3x 7611 proms not dumped
- Test FDC
- Need software (there are floppy images, but they are not yet in a supported format)


List of commands (must be in UPPERCASE):
A - Memory transfer
B - breakpoint management
C - call a user subroutine
D - Dump memory (^X to break)
G - coding of indexed addresses
I - memory initialisation
L - Load cassette (Use L 0), where the number is an offset from the original address.
M - examine/modify memory
N - terminal adaptation (??)
O - calculation of displacements
P - Save cassette
Q - printer activation
R - Display/Alter Registers
S - automatic inhibition of single-step
T - single-step operation
U - memory page change
V - Verify cassette
W - definition of a window
X - DOS loading from TAVBUG09
Y - launch of DOS since TAVBUG09
Z - more scan lines per row (cursor is bigger)
/ - fast memory exam
  - multiple single-steps


**********************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/m6809/m6809.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"
#include "machine/timer.h"
#include "machine/keyboard.h"
#include "machine/wd_fdc.h"
#include "sound/beep.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class cpu09_state : public driver_device
{
public:
	cpu09_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cass(*this, "cassette")
		, m_pia0(*this, "pia0")
		, m_acia(*this, "acia")
		, m_ptm(*this, "ptm")
	{ }

	void cpu09(machine_config &config);

protected:
	u8 pa_r();
	void pa_w(u8 data);
	void pb_w(u8 data);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);
	void cpu09_mem(address_map &map) ATTR_COLD;
	u8 m_pa = 0U;
	bool m_cassold = false;
	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cass;
	required_device<pia6821_device> m_pia0;
	required_device<acia6850_device> m_acia;
	required_device<ptm6840_device> m_ptm;

private:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
};

class ivg09_state : public cpu09_state
{
public:
	ivg09_state(const machine_config &mconfig, device_type type, const char *tag)
		: cpu09_state(mconfig, type, tag)
		, m_pia1(*this, "pia1")
		, m_crtc(*this, "crtc")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_beep(*this, "beeper")
		, m_palette(*this, "palette")
		, m_p_chargen(*this, "chargen")
	{ }

	void ivg09(machine_config &config);

private:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	void ivg09_palette(palette_device &palette) const;
	void ivg09_mem(address_map &map) ATTR_COLD;
	void pa_ivg_w(u8 data);
	u8 pb_ivg_r();
	void vram_w(offs_t offset, u8 data);
	u8 vram_r(offs_t offset);
	void kbd_put(u8 data);
	void ds_w(u8 data);
	MC6845_UPDATE_ROW(crtc_update_row);
	u8 m_term_data = 0U;
	u8 m_ivg_pa = 0U;
	u8 m_flashcnt = 0U;
	std::unique_ptr<u16[]> m_vram; // 12x 4044
	required_device<pia6821_device> m_pia1;
	required_device<mc6845_device> m_crtc;
	required_device<wd2795_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<beep_device> m_beep;
	required_device<palette_device> m_palette;
	required_region_ptr<u8> m_p_chargen;
};


void cpu09_state::cpu09_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x1000, 0x2081).noprw();
	map(0xeb00, 0xeb03).rw(m_pia0, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xeb04, 0xeb05).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xeb08, 0xeb0f).rw(m_ptm, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));
	map(0xec00, 0xefff).ram(); // 1Kx8 RAM MK4118
	map(0xf000, 0xffff).rom().region("roms", 0);
}

void ivg09_state::ivg09_mem(address_map &map)
{
	map.unmap_value_high();
	cpu09_mem(map);
	map(0x1000, 0x1fff).rw(FUNC(ivg09_state::vram_r), FUNC(ivg09_state::vram_w));
	map(0x2000, 0x2003).rw(m_pia1, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x2080, 0x2080).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x2081, 0x2081).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0xe000, 0xe003).rw(m_fdc, FUNC(wd2795_device::read), FUNC(wd2795_device::write));
	map(0xe080, 0xe080).w(FUNC(ivg09_state::ds_w));
}


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

void cpu09_state::machine_reset()
{
}

void ivg09_state::machine_reset()
{
	m_beep->set_state(1);
	m_term_data = 0;
	m_pia1->cb1_w(1);
}

void cpu09_state::machine_start()
{
	save_item(NAME(m_pa));
	save_item(NAME(m_cassold));
}

void ivg09_state::machine_start()
{
	// 4 bits of attribute ram
	m_vram = make_unique_clear<u16[]>(0x1000);
	save_pointer(NAME(m_vram), 0x1000);
	save_item(NAME(m_pa));
	save_item(NAME(m_cassold));
	save_item(NAME(m_term_data));
	save_item(NAME(m_ivg_pa));
	save_item(NAME(m_flashcnt));  // not essential
}

static void ifd09_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

// can support 3 drives
void ivg09_state::ds_w(u8 data)
{
	floppy_image_device *floppy = nullptr;
	if ((data & 3) == 1) floppy = m_floppy0->get_device();
	//if ((data & 3) == 2) floppy = m_floppy1->get_device();
	//if ((data & 3) == 3) floppy = m_floppy2->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy)
	{
		floppy->mon_w(0);
		m_fdc->dden_w(!BIT(data, 2));
	}
}

// Attributes when high: 0 = alpha rom; 1 = flash; 2 = reverse video; 3 = highlight off
MC6845_UPDATE_ROW( ivg09_state::crtc_update_row )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u32 *p = &bitmap.pix(y);
	m_flashcnt++;

	for (u16 x = 0; x < x_count; x++)
	{
		u16 const mem = (ma + x) & 0xfff;
		u8 const attr = m_vram[mem] >> 8;
		u8 const inv = ((x == cursor_x) ^ (BIT(attr, 2)) ^ (BIT(attr, 1) && BIT(m_flashcnt, 6))) ? 0xff : 0;
		u8 const gfx = m_p_chargen[((m_vram[mem] & 0x1ff)<<4) | ra] ^ inv;   // takes care of attr bit 0 too
		u8 const pen = BIT(attr, 3) ? 1 : 2;

		/* Display a scanline of a character */
		*p++ = palette[BIT(gfx, 7) ? pen : 0];
		*p++ = palette[BIT(gfx, 6) ? pen : 0];
		*p++ = palette[BIT(gfx, 5) ? pen : 0];
		*p++ = palette[BIT(gfx, 4) ? pen : 0];
		*p++ = palette[BIT(gfx, 3) ? pen : 0];
		*p++ = palette[BIT(gfx, 2) ? pen : 0];
		*p++ = palette[BIT(gfx, 1) ? pen : 0];
		*p++ = palette[BIT(gfx, 0) ? pen : 0];
	}
}


u8 cpu09_state::pa_r()
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
void cpu09_state::pa_w(u8 data)
{
	m_pa = data & 0x9f;
	m_cass->output(BIT(data, 7) ? -1.0 : +1.0);
}

// centronics
void cpu09_state::pb_w(u8 data)
{
}

u8 ivg09_state::pb_ivg_r()
{
	u8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

void ivg09_state::pa_ivg_w(u8 data)
{
// bits 0-3 are attribute bits
	m_ivg_pa = data & 15;
}

TIMER_DEVICE_CALLBACK_MEMBER( cpu09_state::kansas_r )
{
	if ((m_cass->get_state() & CASSETTE_MASK_UISTATE) != CASSETTE_PLAY)
		return;

	bool cass_ws = (m_cass->input() > +0.04) ? 1 : 0;
	if (cass_ws != m_cassold)
	{
		m_cassold = cass_ws;
		m_pia0->ca1_w(cass_ws);
	}
}

void ivg09_state::vram_w(offs_t offset, u8 data)
{
	m_vram[offset] = data | (m_ivg_pa << 8);
}

// return character; attributes cannot be read
u8 ivg09_state::vram_r(offs_t offset)
{
	return m_vram[offset] & 0xff;
}

static constexpr rgb_t ivg09_pens[3] =
{
	{ 0x00, 0x00, 0x00 }, // black
	{ 0xa0, 0xa0, 0xa0 }, // white
	{ 0xff, 0xff, 0xff }  // highlight
};

void ivg09_state::ivg09_palette(palette_device &palette) const
{
	palette.set_pen_colors(0, ivg09_pens);
}


/* F4 Character Displayer */
static const gfx_layout charlayout =
{
	8, 13,                   /* 8 x 9 characters */
	512,                    /* number of characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( ivg09_gfx )
	GFXDECODE_ENTRY( "chargen", 0x0000, charlayout, 0, 1 )
GFXDECODE_END

void ivg09_state::kbd_put(u8 data)
{
	m_term_data = data;
	m_pia1->cb1_w(0);
	m_pia1->cb1_w(1);
}

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_1200 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_1200 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END

void cpu09_state::cpu09(machine_config &config)
{
	/* basic machine hardware */
	MC6809(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &cpu09_state::cpu09_mem);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	/* Devices */
	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
	TIMER(config, "kansas_r").configure_periodic(FUNC(cpu09_state::kansas_r), attotime::from_hz(19200));

	PIA6821(config, m_pia0);
	m_pia0->readpa_handler().set(FUNC(cpu09_state::pa_r));
	m_pia0->ca1_w(0);
	m_pia0->writepa_handler().set(FUNC(cpu09_state::pa_w));
	m_pia0->writepb_handler().set(FUNC(cpu09_state::pb_w));

	PTM6840(config, m_ptm, 4_MHz_XTAL / 4);
	// all i/o lines connect to the 40-pin expansion connector
	m_ptm->set_external_clocks(0, 0, 0);
	m_ptm->o1_callback().set("acia", FUNC(acia6850_device::write_txc));
	m_ptm->o1_callback().append("acia", FUNC(acia6850_device::write_rxc));
	m_ptm->o2_callback().set_inputline("maincpu", INPUT_LINE_NMI);
	m_ptm->irq_callback().set_inputline("maincpu", M6809_IRQ_LINE);

	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_acia->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("acia", FUNC(acia6850_device::write_rxd));
	rs232.cts_handler().set("acia", FUNC(acia6850_device::write_cts));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));
}

void ivg09_state::ivg09(machine_config &config)
{
	cpu09(config);
	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &ivg09_state::ivg09_mem);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(80*8, 25*10);
	screen.set_visarea(0, 80*8-1, 0, 25*10-1);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));
	GFXDECODE(config, "gfxdecode", m_palette, ivg09_gfx);
	PALETTE(config, m_palette, FUNC(ivg09_state::ivg09_palette), 3);

	/* sound hardware */
	BEEP(config, m_beep, 950).add_route(ALL_OUTPUTS, "mono", 0.50); // guess

	/* Devices */
	subdevice<rs232_port_device>("rs232")->set_default_option(nullptr);

	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(ivg09_state::kbd_put));

	MC6845(config, m_crtc, 1008000); // unknown clock
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(ivg09_state::crtc_update_row));

	PIA6821(config, m_pia1);
	m_pia1->readpb_handler().set(FUNC(ivg09_state::pb_ivg_r));
	m_pia1->writepa_handler().set(FUNC(ivg09_state::pa_ivg_w));
	m_pia1->cb2_handler().set(m_beep, FUNC(beep_device::set_state));

	WD2795(config, m_fdc, 8_MHz_XTAL / 8);
	FLOPPY_CONNECTOR(config, "fdc:0", ifd09_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
}

/* ROM definition */
ROM_START( cpu09 )
	ROM_REGION( 0x1000, "roms", 0 )
	ROM_LOAD( "tavbug.bin",   0x0000, 0x1000, CRC(77945cae) SHA1(d89b577bc0b4e15e9a49a849998681bdc6cf5fbe) )
ROM_END

ROM_START( ivg09 )
	ROM_REGION( 0x19f0, "roms", 0 )
	ROM_LOAD( "tavbug.bin",   0x0000, 0x1000, CRC(77945cae) SHA1(d89b577bc0b4e15e9a49a849998681bdc6cf5fbe) )
	// these 2 are not used. Boottav is copied to ram at c100
	ROM_LOAD_OPTIONAL( "promon.bin",   0x1000, 0x0800, CRC(43256bf2) SHA1(e81acb5b659d50d7b019b97ad5d2a8f129da39f6) )
	ROM_LOAD_OPTIONAL( "boottav.bin",  0x1800, 0x01f0, CRC(ae1a858d) SHA1(ab2144a00afd5b75c6dcb15c2c3f9d6910a159ae) )

	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "big.bin",   0x0000, 0x1000, CRC(f27f6bfe) SHA1(d9509c6e1d10e042ad3cdfaec31114148dee9ff4)) // first half is rubbish
	ROM_CONTINUE(0x0000, 0x1000) // big
	ROM_LOAD( "small.bin", 0x1000, 0x1000, CRC(16e25eed) SHA1(5d31f127fe635be4bca06840b15a1bd77f971492)) // small
ROM_END

} // Anonymous namespace

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS           INIT        COMPANY         FULLNAME                      FLAGS
COMP( 1982, cpu09, 0,      0,      cpu09,   cpu09, cpu09_state, empty_init, "C. Tavernier", "CPU09",                      MACHINE_NOT_WORKING )
COMP( 1983, ivg09, cpu09,  0,      ivg09,   ivg09, ivg09_state, empty_init, "C. Tavernier", "CPU09 with IVG09 and IFD09", MACHINE_NOT_WORKING )
