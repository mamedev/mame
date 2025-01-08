// license:BSD-3-Clause
// copyright-holders:Robbbert
/******************************************************************************************************************

Regnecentralen Piccolo RC702

2016-09-10 Skeleton driver

Undumped prom at IC55 type 74S287
Keyboard has 8048 and 2758, both undumped.

ToDo:
- Printer
- Hard drive, ports 0x60-0x67. Extra CTC on HD board, ports 0x44-0x47
- Other things

Issues:
- Floppy disc error. It reads 0x780 bytes from the wrong sector then gives diskette error (use bios 0)


****************************************************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/z80daisy.h"
#include "machine/7474.h"
#include "machine/am9517a.h"
#include "machine/clock.h"
#include "machine/keyboard.h"
#include "machine/upd765.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "sound/beep.h"
#include "video/i8275.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class rc702_state : public driver_device
{
public:
	rc702_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		, m_bank1(*this, "bank1")
		, m_p_chargen(*this, "chargen")
		, m_ctc1(*this, "ctc1")
		, m_pio(*this, "pio")
		, m_dma(*this, "dma")
		, m_beep(*this, "beeper")
		, m_7474(*this, "7474")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
	{ }

	void rc702(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	uint8_t memory_read_byte(offs_t offset);
	void memory_write_byte(offs_t offset, uint8_t data);
	void port14_w(uint8_t data);
	void port1c_w(uint8_t data);
	void crtc_drq_w(int state);
	void hreq_w(int state);
	void clock_w(int state);
	void eop_w(int state);
	void q_w(int state);
	void qbar_w(int state);
	void dack1_w(int state);
	I8275_DRAW_CHARACTER_MEMBER(display_pixels);
	void rc702_palette(palette_device &palette) const;
	void kbd_put(u8 data);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	bool m_q_state = false;
	bool m_qbar_state = false;
	bool m_drq_state = false;
	uint16_t m_beepcnt = 0U;
	bool m_eop = false;
	bool m_dack1 = false;
	required_device<palette_device> m_palette;
	required_device<z80_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
	required_memory_bank    m_bank1;
	required_region_ptr<u8> m_p_chargen;
	required_device<z80ctc_device> m_ctc1;
	required_device<z80pio_device> m_pio;
	required_device<am9517a_device> m_dma;
	required_device<beep_device> m_beep;
	required_device<ttl7474_device> m_7474;
	required_device<upd765a_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
};


void rc702_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).ram().share("mainram");
	map(0x0000, 0x07ff).bankr("bank1");
}

void rc702_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x01).rw("crtc", FUNC(i8275_device::read), FUNC(i8275_device::write));
	map(0x04, 0x05).m(m_fdc, FUNC(upd765a_device::map));
	map(0x08, 0x0b).rw("sio1", FUNC(z80dart_device::cd_ba_r), FUNC(z80dart_device::cd_ba_w)); // boot sequence doesn't program this
	map(0x0c, 0x0f).rw(m_ctc1, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x10, 0x13).rw(m_pio, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x14, 0x17).portr("DSW").w(FUNC(rc702_state::port14_w)); // motors
	map(0x18, 0x1b).lw8(NAME([this] (u8 data) { m_bank1->set_entry(0); })); // replace roms with ram
	map(0x1c, 0x1f).w(FUNC(rc702_state::port1c_w)); // sound
	map(0xf0, 0xff).rw(m_dma, FUNC(am9517a_device::read), FUNC(am9517a_device::write));
}

/* Input ports */
static INPUT_PORTS_START( rc702 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, "S01")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "S02")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "S03")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "S04")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "S05")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "S06")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "S07")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "Minifloppy") // also need to switch frequencies to fdc
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
INPUT_PORTS_END

void rc702_state::machine_reset()
{
	m_bank1->set_entry(1);
	m_beepcnt = 0xffff;
	m_dack1 = 0;
	m_eop = 0;
	m_7474->preset_w(1);
	m_fdc->set_ready_line_connected(1); // always ready for minifloppy; controlled by fdc for 20cm
	m_fdc->set_unscaled_clock(4000000); // 4MHz for minifloppy; 8MHz for 20cm
	m_maincpu->reset();
}

void rc702_state::machine_start()
{
	m_bank1->configure_entry(0, m_ram);
	m_bank1->configure_entry(1, m_rom);
	save_item(NAME(m_q_state));
	save_item(NAME(m_qbar_state));
	save_item(NAME(m_drq_state));
	save_item(NAME(m_beepcnt));
	save_item(NAME(m_eop));
	save_item(NAME(m_dack1));
}

void rc702_state::q_w(int state)
{
	m_q_state = state;

	if (m_q_state && m_drq_state)
		m_dma->dreq3_w(1);
	else
		m_dma->dreq3_w(0);
}

void rc702_state::qbar_w(int state)
{
	m_qbar_state = state;

	if (m_qbar_state && m_drq_state)
		m_dma->dreq2_w(1);
	else
		m_dma->dreq2_w(0);
}

void rc702_state::crtc_drq_w(int state)
{
	m_drq_state = state;

	if (m_q_state && m_drq_state)
		m_dma->dreq3_w(1);
	else
		m_dma->dreq3_w(0);

	if (m_qbar_state && m_drq_state)
		m_dma->dreq2_w(1);
	else
		m_dma->dreq2_w(0);
}

void rc702_state::eop_w(int state)
{
	if (state == m_eop)
		return;

	m_eop = state;

	if (!m_eop && !m_dack1)
		m_fdc->tc_w(1);
	else
		m_fdc->tc_w(0);
}

void rc702_state::dack1_w(int state)
{
	if (state == m_dack1)
		return;

	m_dack1 = state;

	if (!m_eop && !m_dack1)
		m_fdc->tc_w(1);
	else
		m_fdc->tc_w(0);

	//m_fdc->dack_w = state;  // pin not emulated
}

void rc702_state::port14_w(uint8_t data)
{
	floppy_image_device *floppy = m_floppy0->get_device();
	m_fdc->set_floppy(floppy);
	floppy->mon_w(!BIT(data, 0));
}

void rc702_state::port1c_w(uint8_t data)
{
	m_beep->set_state(1);
	m_beepcnt = 0x3000;
}

// monitor is orange even when powered off
void rc702_state::rc702_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0xc0, 0x60, 0x00));
	palette.set_pen_color(1, rgb_t(0xff, 0xb4, 0x00));
}

I8275_DRAW_CHARACTER_MEMBER( rc702_state::display_pixels )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	uint8_t gfx = 0;

	using namespace i8275_attributes;

	if (!BIT(attrcode, VSP))
		gfx = m_p_chargen[(linecount & 15) | (charcode << 4)];

	if (BIT(attrcode, LTEN))
		gfx = 0xff;

	if (BIT(attrcode, RVV))
		gfx ^= 0xff;

	// Highlight not used
	bitmap.pix(y, x++) = palette[BIT(gfx, 1) ? 1 : 0];
	bitmap.pix(y, x++) = palette[BIT(gfx, 2) ? 1 : 0];
	bitmap.pix(y, x++) = palette[BIT(gfx, 3) ? 1 : 0];
	bitmap.pix(y, x++) = palette[BIT(gfx, 4) ? 1 : 0];
	bitmap.pix(y, x++) = palette[BIT(gfx, 5) ? 1 : 0];
	bitmap.pix(y, x++) = palette[BIT(gfx, 6) ? 1 : 0];
	bitmap.pix(y, x++) = palette[BIT(gfx, 7) ? 1 : 0];
}

// Baud rate generator. All inputs are 0.614MHz.
void rc702_state::clock_w(int state)
{
	m_ctc1->trg0(state);
	m_ctc1->trg1(state);
	if (m_beepcnt == 0)
		m_beep->set_state(0);
	if (m_beepcnt < 0xfe00)
		m_beepcnt--;
}

void rc702_state::hreq_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);
	m_dma->hack_w(state); // tell dma that bus has been granted
}

uint8_t rc702_state::memory_read_byte(offs_t offset)
{
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

void rc702_state::memory_write_byte(offs_t offset, uint8_t data)
{
	m_maincpu->space(AS_PROGRAM).write_byte(offset,data);
}

static const z80_daisy_config daisy_chain_intf[] =
{
	{ "ctc1" },
	{ "sio1" },
	{ "pio" },
	{ nullptr }
};

void rc702_state::kbd_put(u8 data)
{
	m_pio->port_a_write(data);
	m_pio->strobe_a(0);
	m_pio->strobe_a(1);
}

static void floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

void rc702_state::rc702(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(8'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &rc702_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &rc702_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain_intf);

	CLOCK(config, "ctc_clock", 614000).signal_handler().set(FUNC(rc702_state::clock_w));

	Z80CTC(config, m_ctc1, 8_MHz_XTAL / 2);
	m_ctc1->zc_callback<0>().set("sio1", FUNC(z80dart_device::txca_w));
	m_ctc1->zc_callback<0>().append("sio1", FUNC(z80dart_device::rxca_w));
	m_ctc1->zc_callback<1>().set("sio1", FUNC(z80dart_device::rxtxcb_w));
	m_ctc1->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80dart_device& dart(Z80DART(config, "sio1", XTAL(8'000'000) / 2));
	dart.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80PIO(config, m_pio, 8_MHz_XTAL / 2);
	m_pio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
//  m_pio->out_pb_callback().set(FUNC(rc702_state::portxx_w)); // parallel port

	AM9517A(config, m_dma, 8_MHz_XTAL / 2);
	m_dma->out_hreq_callback().set(FUNC(rc702_state::hreq_w));
	m_dma->out_eop_callback().set(FUNC(rc702_state::eop_w)).invert();   // real line is active low, mame has it backwards
	m_dma->in_memr_callback().set(FUNC(rc702_state::memory_read_byte));
	m_dma->out_memw_callback().set(FUNC(rc702_state::memory_write_byte));
	m_dma->in_ior_callback<1>().set(m_fdc, FUNC(upd765a_device::dma_r));
	m_dma->out_iow_callback<1>().set(m_fdc, FUNC(upd765a_device::dma_w));
	m_dma->out_iow_callback<2>().set("crtc", FUNC(i8275_device::dack_w));
	m_dma->out_iow_callback<3>().set("crtc", FUNC(i8275_device::dack_w));
	m_dma->out_dack_callback<1>().set(FUNC(rc702_state::dack1_w));

	UPD765A(config, m_fdc, 8_MHz_XTAL, true, true);
	m_fdc->intrq_wr_callback().set(m_ctc1, FUNC(z80ctc_device::trg3));
	m_fdc->drq_wr_callback().set(m_dma, FUNC(am9517a_device::dreq1_w));
	FLOPPY_CONNECTOR(config, "fdc:0", floppies, "525qd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	/* Keyboard */
	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(rc702_state::kbd_put));

	TTL7474(config, m_7474, 0);
	m_7474->output_cb().set(FUNC(rc702_state::q_w));
	m_7474->comp_output_cb().set(FUNC(rc702_state::qbar_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_size(272*2, 200+4*8);
	screen.set_visarea(0, 272*2-1, 0, 200-1);
	screen.set_screen_update("crtc", FUNC(i8275_device::screen_update));

	i8275_device &crtc(I8275(config, "crtc", 11640000/7));
	crtc.set_character_width(7);
	crtc.set_display_callback(FUNC(rc702_state::display_pixels));
	crtc.irq_wr_callback().set(m_7474, FUNC(ttl7474_device::clear_w)).invert();
	crtc.irq_wr_callback().append(m_ctc1, FUNC(z80ctc_device::trg2));
	crtc.drq_wr_callback().set(FUNC(rc702_state::crtc_drq_w));

	PALETTE(config, m_palette, FUNC(rc702_state::rc702_palette), 2);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 1000).add_route(ALL_OUTPUTS, "mono", 0.50);
}


/* ROM definition */
ROM_START( rc702 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_SYSTEM_BIOS(0, "rc702", "RC702")
	ROMX_LOAD( "roa375.ic66", 0x0000, 0x0800, CRC(034cf9ea) SHA1(306af9fc779e3d4f51645ba04f8a99b11b5e6084), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "rc703", "RC703")
	ROMX_LOAD( "rob357.rom", 0x0000, 0x0800,  CRC(dcf84a48) SHA1(7190d3a898bcbfa212178a4d36afc32bbbc166ef), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "rc700", "RC700")
	ROMX_LOAD( "rob358.rom", 0x0000, 0x0800,  CRC(254aa89e) SHA1(5fb1eb8df1b853b931e670a2ff8d062c1bd8d6bc), ROM_BIOS(2))

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "roa296.rom", 0x0000, 0x0800, CRC(7d7e4548) SHA1(efb8b1ece5f9eeca948202a6396865f26134ff2f) ) // char
	ROM_LOAD( "roa327.rom", 0x0800, 0x0800, CRC(bed7ddb0) SHA1(201ae9e4ac3812577244b9c9044fadd04fb2b82f) ) // semi_gfx
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY           FULLNAME         FLAGS
COMP( 1979, rc702, 0,      0,      rc702,   rc702, rc702_state, empty_init, "Regnecentralen", "RC702 Piccolo", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
