// license:BSD-3-Clause
// copyright-holders:Robbbert
/******************************************************************************************************************

Regnecentralen Piccolo RC702/RC703

2016-09-10 Skeleton driver

Undumped prom at IC55 type 74S287 (address decoder for PROM0/PROM1 mapping)
Keyboard has 8048 and 2758, both undumped.

Machine variants:
  rc702      - RC702 with 8" DSDD floppy drives (maxi), 8 MHz FDC
  rc702mini  - RC702 with 5.25" DD floppy drives (mini), 4 MHz FDC
  rc703      - RC703 with 5.25" QD floppy drives (80-track), 4 MHz FDC
  rc703maxi  - RC703 with 8" DSDD floppy drives (maxi), 8 MHz FDC

ToDo:
- Printer
- Hard drive for RC703, ports 0x60-0x67. Extra CTC on HD board, ports 0x44-0x47
- Keyboard MCU (8048 + 2758) — currently using generic_keyboard

Keyboard (PIO port A):
- The real machine connects the keyboard to Z80 PIO port A.  The driver feeds key data via
  the PIO's in_pa_callback(); the BIOS/CP/M reads the data register and is signalled by strobe.
- prom1 (line program ROM) is undumped; the region is filled with 0xff to avoid a missing-ROM
  warning.

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
		, m_rom_prom1(*this, "prom1")
		, m_ram(*this, "mainram")
		, m_bank1(*this, "bank1")
		, m_bank2(*this, "bank2")
		, m_p_chargen(*this, "chargen")
		, m_ctc1(*this, "ctc1")
		, m_pio(*this, "pio")
		, m_dma(*this, "dma")
		, m_beep(*this, "beeper")
		, m_7474(*this, "7474")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
	{ }

	void rc702_base(machine_config &config);
	void rc702(machine_config &config);
	void rc702mini(machine_config &config);
	void rc703(machine_config &config);
	void rc703maxi(machine_config &config);

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
	uint8_t kbd_r();
	uint8_t m_kbd_data = 0U;

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
	required_region_ptr<u8> m_rom_prom1;
	required_shared_ptr<u8> m_ram;
	required_memory_bank    m_bank1;
	required_memory_bank    m_bank2;
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
	map(0x2000, 0x27ff).bankr("bank2");
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
	map(0x18, 0x1b).lw8(NAME([this] (u8 data) { m_bank1->set_entry(0); m_bank2->set_entry(0); })); // replace roms with ram
	map(0x1c, 0x1f).w(FUNC(rc702_state::port1c_w)); // sound
	map(0xf0, 0xff).rw(m_dma, FUNC(am9517a_device::read), FUNC(am9517a_device::write));
}

/* Input ports - PROM reads port 0x14 bit 7: set=mini, clear=maxi */
static INPUT_PORTS_START( rc702_maxi )
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
	PORT_DIPNAME( 0x80, 0x00, "S08 Minifloppy")
	PORT_DIPSETTING(    0x80, DEF_STR( On ))
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
INPUT_PORTS_END

static INPUT_PORTS_START( rc702_mini )
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
	PORT_DIPNAME( 0x80, 0x80, "S08 Minifloppy")
	PORT_DIPSETTING(    0x80, DEF_STR( On ))
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
INPUT_PORTS_END

void rc702_state::machine_reset()
{
	m_bank1->set_entry(1);
	m_bank2->set_entry(1);
	m_beepcnt = 0xffff;
	m_dack1 = 0;
	m_eop = 0;
	m_7474->preset_w(1);

	// Set FDC data rate: 8" maxi drives use 500 kbps, 5.25" mini use 250 kbps.
	// DIP switch S08 bit 7: clear = maxi (8"), set = mini (5.25").
	m_fdc->set_rate(BIT(ioport("DSW")->read(), 7) ? 250000 : 500000);

	m_maincpu->reset();
}

void rc702_state::machine_start()
{
	m_bank1->configure_entry(0, m_ram);
	m_bank1->configure_entry(1, m_rom);
	m_bank2->configure_entry(0, &m_ram[0x2000]);
	m_bank2->configure_entry(1, m_rom_prom1);
	save_item(NAME(m_q_state));
	save_item(NAME(m_qbar_state));
	save_item(NAME(m_drq_state));
	save_item(NAME(m_beepcnt));
	save_item(NAME(m_eop));
	save_item(NAME(m_dack1));
	save_item(NAME(m_kbd_data));
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
	// Mini floppy motor control: bit 0 = 1 starts motor, 0 stops it.
	// Maxi (8") drives have always-spinning motors so mon_w() is a no-op.
	// Do NOT call set_floppy() here — the FDC connector already binds flopi[0]
	// during device_start().  Calling set_floppy() would assign the same device
	// to all 4 internal FDC drive slots, causing 4 spurious ready-change
	// interrupts on a single drive event and deadlocking the boot PROM.
	floppy_image_device *floppy = m_floppy0->get_device();
	if (floppy)
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
	m_kbd_data = data;
	m_pio->strobe_a(0);
	m_pio->strobe_a(1);
}

uint8_t rc702_state::kbd_r()
{
	return m_kbd_data;
}

static void rc702_floppies(device_slot_interface &device)
{
	device.option_add("8dsdd", FLOPPY_8_DSDD);
}

static void rc702mini_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

static void rc703_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

void rc702_state::rc702_base(machine_config &config)
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
	m_pio->in_pa_callback().set(FUNC(rc702_state::kbd_r));
//  m_pio->out_pb_callback().set(FUNC(rc702_state::portxx_w)); // parallel port

	AM9517A(config, m_dma, 8_MHz_XTAL / 2);
	m_dma->out_hreq_callback().set(FUNC(rc702_state::hreq_w));
	m_dma->out_eop_callback().set(FUNC(rc702_state::eop_w)).invert();   // real line is active low, mame has it backwards
	m_dma->in_memr_callback().set(FUNC(rc702_state::memory_read_byte));
	m_dma->out_memw_callback().set(FUNC(rc702_state::memory_write_byte));
	m_dma->out_iow_callback<2>().set("crtc", FUNC(i8275_device::dack_w));
	m_dma->out_iow_callback<3>().set("crtc", FUNC(i8275_device::dack_w));

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

void rc702_state::rc702(machine_config &config)
{
	rc702_base(config);

	UPD765A(config, m_fdc, 8_MHz_XTAL, true, true);
	m_fdc->intrq_wr_callback().set(m_ctc1, FUNC(z80ctc_device::trg3));
	m_fdc->drq_wr_callback().set(m_dma, FUNC(am9517a_device::dreq1_w));
	m_dma->in_ior_callback<1>().set(m_fdc, FUNC(upd765a_device::dma_r));
	m_dma->out_iow_callback<1>().set(m_fdc, FUNC(upd765a_device::dma_w));
	m_dma->out_dack_callback<1>().set(FUNC(rc702_state::dack1_w));

	FLOPPY_CONNECTOR(config, "fdc:0", rc702_floppies, "8dsdd", floppy_image_device::default_mfm_floppy_formats).enable_sound(false);
	FLOPPY_CONNECTOR(config, "fdc:1", rc702_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats).enable_sound(false);
}

void rc702_state::rc702mini(machine_config &config)
{
	rc702_base(config);

	UPD765A(config, m_fdc, 8_MHz_XTAL / 2, true, true);  // 4 MHz for 5.25" mini drives
	m_fdc->intrq_wr_callback().set(m_ctc1, FUNC(z80ctc_device::trg3));
	m_fdc->drq_wr_callback().set(m_dma, FUNC(am9517a_device::dreq1_w));
	m_dma->in_ior_callback<1>().set(m_fdc, FUNC(upd765a_device::dma_r));
	m_dma->out_iow_callback<1>().set(m_fdc, FUNC(upd765a_device::dma_w));
	m_dma->out_dack_callback<1>().set(FUNC(rc702_state::dack1_w));

	FLOPPY_CONNECTOR(config, "fdc:0", rc702mini_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(false);
	FLOPPY_CONNECTOR(config, "fdc:1", rc702mini_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats).enable_sound(false);
}

void rc702_state::rc703(machine_config &config)
{
	rc702_base(config);

	UPD765A(config, m_fdc, 8_MHz_XTAL / 2, true, true);  // 4 MHz for 5.25" QD drives
	m_fdc->intrq_wr_callback().set(m_ctc1, FUNC(z80ctc_device::trg3));
	m_fdc->drq_wr_callback().set(m_dma, FUNC(am9517a_device::dreq1_w));
	m_dma->in_ior_callback<1>().set(m_fdc, FUNC(upd765a_device::dma_r));
	m_dma->out_iow_callback<1>().set(m_fdc, FUNC(upd765a_device::dma_w));
	m_dma->out_dack_callback<1>().set(FUNC(rc702_state::dack1_w));

	FLOPPY_CONNECTOR(config, "fdc:0", rc703_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats).enable_sound(false);
	FLOPPY_CONNECTOR(config, "fdc:1", rc703_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats).enable_sound(false);
	// TODO: Hard disk ports 0x60-0x67, CTC2 ports 0x44-0x47
}

void rc702_state::rc703maxi(machine_config &config)
{
	rc702_base(config);

	UPD765A(config, m_fdc, 8_MHz_XTAL, true, true);  // 8 MHz for 8" drives
	m_fdc->intrq_wr_callback().set(m_ctc1, FUNC(z80ctc_device::trg3));
	m_fdc->drq_wr_callback().set(m_dma, FUNC(am9517a_device::dreq1_w));
	m_dma->in_ior_callback<1>().set(m_fdc, FUNC(upd765a_device::dma_r));
	m_dma->out_iow_callback<1>().set(m_fdc, FUNC(upd765a_device::dma_w));
	m_dma->out_dack_callback<1>().set(FUNC(rc702_state::dack1_w));

	FLOPPY_CONNECTOR(config, "fdc:0", rc702_floppies, "8dsdd", floppy_image_device::default_mfm_floppy_formats).enable_sound(false);
	FLOPPY_CONNECTOR(config, "fdc:1", rc702_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats).enable_sound(false);
	// TODO: Hard disk ports 0x60-0x67, CTC2 ports 0x44-0x47
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

	ROM_REGION( 0x0800, "prom1", ROMREGION_ERASEFF )
	ROM_FILL( 0x0000, 0x0800, 0xff ) // line program ROM (ROB388 on MIC705) - undumped prom1.ic65

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "roa296.rom", 0x0000, 0x0800, CRC(7d7e4548) SHA1(efb8b1ece5f9eeca948202a6396865f26134ff2f) ) // char
	ROM_LOAD( "roa327.rom", 0x0800, 0x0800, CRC(bed7ddb0) SHA1(201ae9e4ac3812577244b9c9044fadd04fb2b82f) ) // semi_gfx
ROM_END

/* RC703 maxi: rob358 (RC700) as default BIOS */
ROM_START( rc703maxi )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_SYSTEM_BIOS(0, "rc700", "RC700")
	ROMX_LOAD( "rob358.rom", 0x0000, 0x0800,  CRC(254aa89e) SHA1(5fb1eb8df1b853b931e670a2ff8d062c1bd8d6bc), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "rc702", "RC702")
	ROMX_LOAD( "roa375.ic66", 0x0000, 0x0800, CRC(034cf9ea) SHA1(306af9fc779e3d4f51645ba04f8a99b11b5e6084), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "rc703", "RC703")
	ROMX_LOAD( "rob357.rom", 0x0000, 0x0800,  CRC(dcf84a48) SHA1(7190d3a898bcbfa212178a4d36afc32bbbc166ef), ROM_BIOS(2))

	ROM_REGION( 0x0800, "prom1", ROMREGION_ERASEFF )
	ROM_FILL( 0x0000, 0x0800, 0xff )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "roa296.rom", 0x0000, 0x0800, CRC(7d7e4548) SHA1(efb8b1ece5f9eeca948202a6396865f26134ff2f) )
	ROM_LOAD( "roa327.rom", 0x0800, 0x0800, CRC(bed7ddb0) SHA1(201ae9e4ac3812577244b9c9044fadd04fb2b82f) )
ROM_END

} // anonymous namespace


/* Driver */

#define rom_rc702mini  rom_rc702
#define rom_rc703      rom_rc702

//    YEAR  NAME       PARENT  COMPAT  MACHINE    INPUT        CLASS        INIT        COMPANY           FULLNAME                     FLAGS
COMP( 1979, rc702,     0,      0,      rc702,     rc702_maxi,  rc702_state, empty_init, "Regnecentralen", "RC702 Piccolo (8\")",        MACHINE_SUPPORTS_SAVE )
COMP( 1979, rc702mini, rc702,  0,      rc702mini, rc702_mini,  rc702_state, empty_init, "Regnecentralen", "RC702 Piccolo (5.25\")",     MACHINE_SUPPORTS_SAVE )
COMP( 1982, rc703,     rc702,  0,      rc703,     rc702_mini,  rc702_state, empty_init, "Regnecentralen", "RC703 Piccolo (5.25\")",     MACHINE_SUPPORTS_SAVE )
COMP( 1982, rc703maxi, rc702,  0,      rc703maxi, rc702_maxi,  rc702_state, empty_init, "Regnecentralen", "RC703 Piccolo (8\")",        MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
