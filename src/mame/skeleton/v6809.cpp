// license:BSD-3-Clause
// copyright-holders:Robbbert
/**********************************************************************************************

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

- Colours (Looks like characters 0xc0-0xff produce coloured lores gfx).

- Connect the RTC interrupt pin (not supported currently)

- Find the missing character generator rom.

- Schematic is almost useless, riddled with omissions and errors. All documents are in
    French. The parts list only has half of the parts.

- Need software (there are floppy images, but they are not yet in a supported format)


*******************************************************************************************/

#include "emu.h"

#include "cpu/m6809/m6809.h"
#include "imagedev/floppy.h"
#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/keyboard.h"
#include "machine/mm58174.h"
#include "machine/wd_fdc.h"
#include "sound/spkrdev.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class v6809_state : public driver_device
{
public:
	v6809_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_pia0(*this, "pia0")
		, m_maincpu(*this, "maincpu")
		, m_crtc(*this, "crtc")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_speaker(*this, "speaker")
		, m_palette(*this, "palette")
		, m_p_chargen(*this, "chargen")
	{ }

	void v6809(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void speaker_en_w(int state);
	void speaker_w(int state);
	u8 pb_r();
	void pa_w(u8 data);
	void videoram_w(u8 data);
	void v6809_address_w(u8 data);
	void v6809_register_w(u8 data);
	void kbd_put(u8 data);
	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_update_addr);

	void v6809_mem(address_map &map) ATTR_COLD;

	u16 m_video_address = 0U;
	bool m_speaker_en = false;
	u8 m_video_index = 0U;
	u8 m_term_data = 0U;
	u8 m_vidbyte = 0U;
	std::unique_ptr<u8[]> m_vram;
	required_device<pia6821_device> m_pia0;
	required_device<cpu_device> m_maincpu;
	required_device<mc6845_device> m_crtc;
	required_device<mb8876_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<speaker_sound_device> m_speaker;
	required_device<palette_device> m_palette;
	required_region_ptr<u8> m_p_chargen;
};


void v6809_state::v6809_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xefff).ram();
	map(0xf000, 0xf000).mirror(0xfe).r(m_crtc, FUNC(mc6845_device::status_r)).w(FUNC(v6809_state::v6809_address_w));
	map(0xf001, 0xf001).mirror(0xfe).r(m_crtc, FUNC(mc6845_device::register_r)).w(FUNC(v6809_state::v6809_register_w));
	map(0xf200, 0xf200).mirror(0xff).w(FUNC(v6809_state::videoram_w));
	map(0xf500, 0xf501).mirror(0x36).rw("acia0", FUNC(acia6850_device::read), FUNC(acia6850_device::write)); // modem
	map(0xf508, 0xf509).mirror(0x36).rw("acia1", FUNC(acia6850_device::read), FUNC(acia6850_device::write)); // printer
	map(0xf600, 0xf603).mirror(0x3c).rw(m_fdc, FUNC(mb8876_device::read), FUNC(mb8876_device::write));
	map(0xf640, 0xf64f).mirror(0x30).rw("rtc", FUNC(mm58174_device::read), FUNC(mm58174_device::write));
	map(0xf680, 0xf683).mirror(0x3c).rw(m_pia0, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xf6c0, 0xf6c7).mirror(0x08).rw("ptm", FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));
	map(0xf6d0, 0xf6d3).mirror(0x0c).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xf800, 0xffff).rom().region("maincpu", 0);
}


/* Input ports */
static INPUT_PORTS_START( v6809 )
INPUT_PORTS_END

void v6809_state::machine_start()
{
	m_vram = make_unique_clear<u8[]>(0x0800);
	save_pointer(NAME(m_vram), 0x0800);
	save_item(NAME(m_speaker_en));
	save_item(NAME(m_term_data));
	save_item(NAME(m_video_address));
	save_item(NAME(m_video_index));
	save_item(NAME(m_vidbyte));
}

void v6809_state::machine_reset()
{
	m_term_data = 0;
	m_pia0->cb1_w(1);
}

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

static GFXDECODE_START( gfx_v6809 )
	GFXDECODE_ENTRY( "chargen", 0x0000, v6809_charlayout, 0, 1 )
GFXDECODE_END

MC6845_UPDATE_ROW( v6809_state::crtc_update_row )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u32 *p = &bitmap.pix(y);

	for (u16 x = 0; x < x_count; x++)
	{
		u16 mem = (ma + x) & 0x7ff;
		u8 chr = m_vram[mem];
		u8 gfx = m_p_chargen[(chr<<4) | ra] ^ ((x == cursor_x) ? 0xff : 0);

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

MC6845_ON_UPDATE_ADDR_CHANGED( v6809_state::crtc_update_addr )
{
/* not sure what goes in here - parameters passed are device, address, strobe */
	m_video_address = address & 0x7ff;
}

void v6809_state::videoram_w(u8 data)
{
	m_vidbyte = data;
}

void v6809_state::v6809_address_w(u8 data)
{
	m_crtc->address_w(data);

	m_video_index = data & 0x1f;

	if (m_video_index == 31)
		m_vram[m_video_address] = m_vidbyte;
}

void v6809_state::v6809_register_w(u8 data)
{
	u16 temp = m_video_address;

	m_crtc->register_w(data);

	// Get transparent address
	if (m_video_index == 18)
		m_video_address = ((data & 7) << 8 ) | (temp & 0xff);
	else
	if (m_video_index == 19)
		m_video_address = data | (temp & 0xff00);
}

// **** Keyboard ****

void v6809_state::kbd_put(u8 data)
{
	m_term_data = data;
	m_pia0->cb1_w(0);
	m_pia0->cb1_w(1);
}

u8 v6809_state::pb_r()
{
	u8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

// can support 4 drives
void v6809_state::pa_w(u8 data)
{
	floppy_image_device *floppy = nullptr;
	if ((data & 3) == 0) floppy = m_floppy0->get_device();
	if ((data & 3) == 1) floppy = m_floppy1->get_device();
	//if ((data & 3) == 2) floppy = m_floppy2->get_device();
	//if ((data & 3) == 3) floppy = m_floppy3->get_device();

	m_fdc->set_floppy(floppy);

// Bits 2 and 3 go to the floppy connector but are not documented

	if (floppy)
	{
		floppy->mon_w(0);
		m_fdc->dden_w(BIT(data, 4));
	}
}

void v6809_state::speaker_en_w(int state)
{
	m_speaker_en = state;
}

void v6809_state::speaker_w(int state)
{
	if (m_speaker_en)
		m_speaker->level_w(state);
}

static void v6809_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}


// *** Machine ****

void v6809_state::v6809(machine_config &config)
{
	/* basic machine hardware */
	MC6809(config, m_maincpu, 16_MHz_XTAL / 4); // divided by 4 again internally
	m_maincpu->set_addrmap(AS_PROGRAM, &v6809_state::v6809_mem);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update("crtc", FUNC(sy6545_1_device::screen_update));
	PALETTE(config, m_palette, palette_device::MONOCHROME);
	GFXDECODE(config, "gfxdecode", m_palette, gfx_v6809);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	/* devices */
	SY6545_1(config, m_crtc, 16_MHz_XTAL / 8);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(v6809_state::crtc_update_row));
	m_crtc->set_on_update_addr_change_callback(FUNC(v6809_state::crtc_update_addr));

	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(v6809_state::kbd_put));

	// port A = drive select and 2 control lines ; port B = keyboard
	PIA6821(config, m_pia0);
	m_pia0->readpb_handler().set(FUNC(v6809_state::pb_r));
	m_pia0->writepa_handler().set(FUNC(v6809_state::pa_w));
	m_pia0->irqa_handler().set_inputline("maincpu", M6809_IRQ_LINE);
	m_pia0->irqb_handler().set_inputline("maincpu", M6809_IRQ_LINE);

	// no idea what this does
	pia6821_device &pia1(PIA6821(config, "pia1"));
	pia1.irqa_handler().set_inputline("maincpu", M6809_IRQ_LINE);
	pia1.irqb_handler().set_inputline("maincpu", M6809_IRQ_LINE);

	ptm6840_device &ptm(PTM6840(config, "ptm", 16_MHz_XTAL / 4));
	ptm.set_external_clocks(4000000.0/14.0, 4000000.0/14.0, (4000000.0/14.0)/8.0);
	ptm.o1_callback().set(FUNC(v6809_state::speaker_en_w));
	ptm.o2_callback().set(FUNC(v6809_state::speaker_w));
	ptm.irq_callback().set_inputline("maincpu", M6809_IRQ_LINE);

	ACIA6850(config, "acia0", 0);

	ACIA6850(config, "acia1", 0);

	clock_device &acia_clock(CLOCK(config, "acia_clock", 153600));
	acia_clock.signal_handler().set("acia0", FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append("acia0", FUNC(acia6850_device::write_rxc));
	acia_clock.signal_handler().append("acia1", FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append("acia1", FUNC(acia6850_device::write_rxc));

	MM58174(config, "rtc", 0);
	//rtc.irq_handler().set(m_pia0, FUNC(pia6821_device::cb2_w));   // unsupported by RTC emulation

	MB8876(config, m_fdc, 16_MHz_XTAL / 16);
	FLOPPY_CONNECTOR(config, "fdc:0", v6809_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", v6809_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
}

/* ROM definition */
ROM_START( v6809 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_SYSTEM_BIOS(0, "v1", "Original")
	ROMX_LOAD( "v6809.rom",   0x0000, 0x0800, CRC(54bf5f32) SHA1(10d1d70f0b51e2b90e5c29249d3eab4c6b0033a1), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v3", "Ver 3.3")
	ROMX_LOAD( "v6809v3.bin", 0x0000, 0x0800, CRC(f9cd126d) SHA1(2da719820e393efde801057d76b2a63dcfbd8541), ROM_BIOS(1))
	/* character generator not dumped, using the one from 'h19' for now */
	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "2716_444-29_h19font.bin", 0x0000, 0x0800, BAD_DUMP CRC(d595ac1d) SHA1(130fb4ea8754106340c318592eec2d8a0deaf3d0))
	ROM_RELOAD(0x0800, 0x0800)
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY     FULLNAME      FLAGS
COMP( 1982, v6809, 0,      0,      v6809,   v6809, v6809_state, empty_init, "Microkit", "Vegas 6809", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
