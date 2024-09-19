// license: BSD-3-Clause
// copyright-holders: Angelo Salese, Dirk Best
/***************************************************************************

    Casio FP-6000

    TODO:
    - Fix cassette (SAVE is at 300 baud Kansas City format, loadable
      on the super80, but LOAD throws RW error).
    - Floppy/HDD
    - Printer
    - gvram color pen is a rather crude guess (the layer is monochrome on
      BASIC?);
    - everything else

****************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "video/mc6845.h"
#include "sound/spkrdev.h"
#include "bus/centronics/ctronics.h"
#include "fp6000_kbd.h"
#include "imagedev/cassette.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class fp6000_state : public driver_device
{
public:
	fp6000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pic(*this, "pic"),
		m_pit(*this, "pit"),
		m_crtc(*this, "crtc"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_speaker(*this, "speaker"),
		m_cassette(*this, "cassette"),
		m_centronics(*this, "centronics"),
		m_gvram(*this, "gvram"),
		m_vram(*this, "vram"),
		m_pcg(*this, "pcg")
	{ }

	void fp6000(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic;
	required_device<pit8253_device> m_pit;
	required_device<mc6845_device>m_crtc;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<centronics_device> m_centronics;

	required_shared_ptr<uint16_t> m_gvram;
	required_shared_ptr<uint16_t> m_vram;
	required_shared_ptr<uint16_t> m_pcg;

	void fp6000_io(address_map &map) ATTR_COLD;
	void fp6000_map(address_map &map) ATTR_COLD;

	emu_timer *m_pit_timer = nullptr;
	void pit_timer0_w(int state);
	TIMER_CALLBACK_MEMBER(pit_timer0_clear);
	void pit_timer2_w(int state);

	uint8_t port_08_r();
	void port_08_w(uint8_t data);
	uint8_t port_09_r();
	void port_09_w(uint8_t data);
	void port_0a_w(uint8_t data);
	uint8_t port_0b_r();
	void port_0b_w(uint8_t data);
	uint8_t port_0c_r();
	void port_0c_w(uint8_t data);
	uint8_t port_0d_r();
	void port_0d_w(uint8_t data);
	uint8_t port_0e_r();
	uint8_t port_0f_r();
	void port_0f_w(uint8_t data);

	MC6845_UPDATE_ROW(crtc_update_row);
	uint16_t unk_r();

	void centronics_busy_w(int state) { m_centronics_busy = state; };
	void centronics_fault_w(int state) { m_centronics_fault = state; };
	void centronics_perror_w(int state) { m_centronics_perror = state; };

	uint8_t m_port_0a = 0;

	int m_centronics_busy = 0;
	int m_centronics_fault = 0;
	int m_centronics_perror = 0;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void fp6000_state::fp6000_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0xbffff).ram();
	map(0xc0000, 0xdffff).ram().share("gvram");
	map(0xe0000, 0xe0fff).ram().share("vram");
	map(0xe7000, 0xe7fff).ram().share("pcg");
	map(0xf0000, 0xfffff).rom().region("ipl", 0);
}

void fp6000_state::fp6000_io(address_map &map)
{
	map.unmap_value_high();
	map(0x08, 0x08).r(FUNC(fp6000_state::port_08_r));
	map(0x08, 0x08).w(FUNC(fp6000_state::port_08_w));
	map(0x09, 0x09).r(FUNC(fp6000_state::port_09_r));
	map(0x09, 0x09).w(FUNC(fp6000_state::port_09_w));
	map(0x0a, 0x0a).lr8(NAME([this] () { return ioport("cpudsw")->read(); }));
	map(0x0a, 0x0a).w(FUNC(fp6000_state::port_0a_w));
	map(0x0b, 0x0b).r(FUNC(fp6000_state::port_0b_r));
	map(0x0b, 0x0b).w(FUNC(fp6000_state::port_0b_w));
	map(0x0c, 0x0c).r(FUNC(fp6000_state::port_0c_r));
	map(0x0c, 0x0c).w(FUNC(fp6000_state::port_0c_w));
	map(0x0d, 0x0d).r(FUNC(fp6000_state::port_0d_r));
	map(0x0d, 0x0d).w(FUNC(fp6000_state::port_0d_w));
	map(0x0e, 0x0e).r(FUNC(fp6000_state::port_0e_r));
	map(0x0e, 0x0e).w("centronics_data_out", FUNC(output_latch_device::write));
	map(0x0f, 0x0f).r(FUNC(fp6000_state::port_0f_r));
	map(0x0f, 0x0f).w(FUNC(fp6000_state::port_0f_w));
	// 10-17 floppy?
	map(0x14, 0x14).lr8(NAME([this] () { return ioport("floppydsw")->read(); }));
	map(0x20, 0x23).rw("keyboard", FUNC(fp6000_kbd_device::read), FUNC(fp6000_kbd_device::write)).umask16(0x00ff);
	map(0x30, 0x33).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x38, 0x3f).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);
	// 50-5f dma?
	map(0x70, 0x70).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0x72, 0x72).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x74, 0x75).r(FUNC(fp6000_state::unk_r)); //bit 6 busy flag
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( fp6000 )
	PORT_START("cpudsw")
	PORT_DIPNAME(0x1f, 0x1e, "Printer type")
	PORT_DIPSETTING(   0x1f, "0")
	PORT_DIPSETTING(   0x1e, "1")
	PORT_DIPSETTING(   0x1d, "2")
	PORT_DIPSETTING(   0x1c, "3")
	PORT_DIPSETTING(   0x1b, "4")
	PORT_DIPSETTING(   0x1a, "5")
	PORT_DIPSETTING(   0x19, "6")
	PORT_DIPSETTING(   0x18, "7")
	PORT_DIPSETTING(   0x17, "8")
	PORT_DIPSETTING(   0x16, "9")
	PORT_DIPSETTING(   0x15, "10")
	PORT_DIPSETTING(   0x14, "11")
	PORT_DIPSETTING(   0x13, "12")
	PORT_DIPSETTING(   0x12, "13")
	PORT_DIPSETTING(   0x11, "14")
	PORT_DIPSETTING(   0x10, "15")
	PORT_DIPSETTING(   0x0f, "16")
	PORT_DIPSETTING(   0x0e, "17")
	PORT_DIPSETTING(   0x0d, "18")
	PORT_DIPSETTING(   0x0c, "19")
	PORT_DIPSETTING(   0x0b, "20")
	PORT_DIPSETTING(   0x0a, "21")
	PORT_DIPSETTING(   0x09, "22")
	PORT_DIPSETTING(   0x08, "23")
	PORT_DIPSETTING(   0x07, "24")
	PORT_DIPSETTING(   0x06, "25")
	PORT_DIPSETTING(   0x05, "26")
	PORT_DIPSETTING(   0x04, "27")
	PORT_DIPSETTING(   0x03, "28")
	PORT_DIPSETTING(   0x02, "29")
	PORT_DIPSETTING(   0x01, "30")
	PORT_DIPSETTING(   0x00, "31")
	PORT_DIPNAME(0xe0, 0x40, "Installed RAM banks")
	PORT_DIPSETTING(   0xe0, "0")
	PORT_DIPSETTING(   0xc0, "1")
	PORT_DIPSETTING(   0xa0, "2")
	PORT_DIPSETTING(   0x80, "3")
	PORT_DIPSETTING(   0x60, "4")
	PORT_DIPSETTING(   0x40, "5")
	PORT_DIPSETTING(   0x20, "6 (INVALID)") // exceeds 768KB limit (writes to gvram et al)
	PORT_DIPSETTING(   0x00, "7 (INVALID)")

	PORT_START("floppydsw")
	PORT_DIPNAME(0x07, 0x07, "Floppy type?")
	PORT_DIPSETTING(   0x07, DEF_STR( None ))
	PORT_DIPSETTING(   0x06, "1")
	PORT_DIPSETTING(   0x05, "2")
	PORT_DIPSETTING(   0x04, "3")
	PORT_DIPSETTING(   0x03, "4")
	PORT_DIPSETTING(   0x02, "5")
	PORT_DIPSETTING(   0x01, "6")
	PORT_DIPSETTING(   0x00, "7")
	PORT_DIPUNKNOWN(0x08, 0x08)
	PORT_DIPUNKNOWN(0x10, 0x10)
	PORT_DIPUNKNOWN(0x20, 0x20)
	PORT_DIPUNKNOWN(0x40, 0x40)
	PORT_DIPUNKNOWN(0x80, 0x80)
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

uint16_t fp6000_state::unk_r()
{
	// 7-------
	// -6------  ?
	// --5-----
	// ---4----
	// ----3---
	// -----2--
	// ------1-  screen lines: 0=200, 1=400
	// -------0

	return 0x40;
}

MC6845_UPDATE_ROW( fp6000_state::crtc_update_row )
{
	pen_t const *const pen = m_palette->pens();
	uint8_t const *const pcg = reinterpret_cast<uint8_t *>(m_pcg.target());
	uint32_t const *const vram = reinterpret_cast<uint32_t *>(m_gvram.target());

	for (int x = 0; x < x_count; x++)
	{
		// text mode
		uint8_t const code = (m_vram[ma + x] >> 0) & 0xff;
		uint8_t const color = (m_vram[ma + x] >> 8) & 0x0f;
		uint8_t gfx = pcg[(code << 4) | ra];

		// cursor?
		if (x == cursor_x)
			gfx = 0xff;

		// draw 8 pixels of the character
		for (int i = 0; i < 8; i++)
			bitmap.pix(y, x * 8 + i) = BIT(gfx, 7 - i) ? pen[color] : 0;

		// graphics
		uint32_t const data = vram[(ma << 3) + (ra * x_count) + x];

		// draw 8 gfx pixels
		if ((data >> 12) & 0x0f) bitmap.pix(y, x * 8 + 0) = pen[(data >> 12) & 0x0f];
		if ((data >>  8) & 0x0f) bitmap.pix(y, x * 8 + 1) = pen[(data >>  8) & 0x0f];
		if ((data >>  4) & 0x0f) bitmap.pix(y, x * 8 + 2) = pen[(data >>  4) & 0x0f];
		if ((data >>  0) & 0x0f) bitmap.pix(y, x * 8 + 3) = pen[(data >>  0) & 0x0f];
		if ((data >> 28) & 0x0f) bitmap.pix(y, x * 8 + 4) = pen[(data >> 28) & 0x0f];
		if ((data >> 24) & 0x0f) bitmap.pix(y, x * 8 + 5) = pen[(data >> 24) & 0x0f];
		if ((data >> 20) & 0x0f) bitmap.pix(y, x * 8 + 6) = pen[(data >> 20) & 0x0f];
		if ((data >> 16) & 0x0f) bitmap.pix(y, x * 8 + 7) = pen[(data >> 16) & 0x0f];
	}
}

static const gfx_layout charlayout =
{
	8, 16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	8*16
};

static GFXDECODE_START( gfx )
	GFXDECODE_RAM("pcg", 0, charlayout, 0, 1)
GFXDECODE_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

// 7-------  option rom available (1=no)
// -654----  unknown
// ----3---  cassette in
// -----21-  unknown
// -------0  cassette motor

uint8_t fp6000_state::port_08_r()
{
	uint8_t data = 0;

	data |= 0x80; // no option rom
	data |= (m_cassette->input() > 0 ? 0x00 : 0x08);

	return data;
}

void fp6000_state::port_08_w(uint8_t data)
{
	logerror("port_08 write %02x\n", data);

	m_cassette->change_state(BIT(data, 0) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
}

uint8_t fp6000_state::port_09_r()
{
	logerror("port_09 read\n");
	return 0xff;
}

void fp6000_state::port_09_w(uint8_t data)
{
	logerror("port_09 write %02x\n", data);
}

void fp6000_state::port_0a_w(uint8_t data)
{
	// 7-------  speaker/cassette output select
	// -6543210  unknown

	logerror("port_0a write %02x\n", data);

	m_port_0a = data;
}

uint8_t fp6000_state::port_0b_r()
{
	logerror("port_0b read\n");
	return 0xff;
}

void fp6000_state::port_0b_w(uint8_t data)
{
	// printer control?
	logerror("port_0b write %02x\n", data);
	m_pic->ir7_w(1); // ?
}

uint8_t fp6000_state::port_0c_r()
{
	logerror("port_0c read\n");
	return 0xff;
}

void fp6000_state::port_0c_w(uint8_t data)
{
	// 7-------  unknown
	// -6------  pit timer2 gate?
	// --543210  unknown

	logerror("port_0c write %02x\n", data);
}

uint8_t fp6000_state::port_0d_r()
{
	logerror("port_0d read\n");
	return 0xff;
}

void fp6000_state::port_0d_w(uint8_t data)
{
	// after writing printer data
	logerror("port_0d write %02x\n", data);

	// ?
	m_centronics->write_strobe(1);
	m_centronics->write_strobe(0);
}

uint8_t fp6000_state::port_0e_r()
{
	uint8_t data = 0;

	// 765-----  unknown
	// ---4321-  printer status lines
	// -------0  printer busy

	logerror("port_0e read\n");

	data |= m_centronics_perror << 2; // guess
	data |= m_centronics_fault << 1; // guess
	data |= m_centronics_busy << 0;

	return data;
}

uint8_t fp6000_state::port_0f_r()
{
	// read at end of timer interrupt routine, result discarded
	return 0xff;
}

void fp6000_state::port_0f_w(uint8_t data)
{
	logerror("port_0f write %02x\n", data);
	m_pic->ir7_w(0); // ?
}

void fp6000_state::pit_timer0_w(int state)
{
	// work around pit issue, it issues set and clear at the same time,
	// leaving the pic no time to react
	if (state)
		m_pic->ir0_w(1);
	else
		m_pit_timer->adjust(attotime::from_hz(100000)); // timing?
}

TIMER_CALLBACK_MEMBER(fp6000_state::pit_timer0_clear)
{
	m_pic->ir0_w(0);
}

void fp6000_state::pit_timer2_w(int state)
{
	if (BIT(m_port_0a, 7))
		m_speaker->level_w(state);
	else
		m_cassette->output(state ? -1.0 : +1.0);
}

void fp6000_state::machine_start()
{
	m_pit_timer = timer_alloc(FUNC(fp6000_state::pit_timer0_clear), this);
}

void fp6000_state::machine_reset()
{
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void fp6000_state::fp6000(machine_config &config)
{
	I8086(config, m_maincpu, 16000000 / 2); // 8 Mhz?
	m_maincpu->set_addrmap(AS_PROGRAM, &fp6000_state::fp6000_map);
	m_maincpu->set_addrmap(AS_IO, &fp6000_state::fp6000_io);
	m_maincpu->set_irq_acknowledge_callback(m_pic, FUNC(pic8259_device::inta_cb));

	PIC8259(config, m_pic, 0);
	m_pic->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	PIT8253(config, m_pit, 0);
	m_pit->set_clk<0>(16000000 / 16); // 1 MHz
	m_pit->out_handler<0>().set(FUNC(fp6000_state::pit_timer0_w)).invert();
	m_pit->set_clk<2>(16000000 / 8); // 2 MHz?
	m_pit->out_handler<2>().set(FUNC(fp6000_state::pit_timer2_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(16000000, 1024, 0, 640, 272, 0, 200); // 16 MHz?
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	MC6845(config, m_crtc, 16000000 / 8); // unknown variant, 2 MHz?
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(fp6000_state::crtc_update_row));

	PALETTE(config, m_palette).set_entries(16);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx);

	// audio hardware
	SPEAKER(config, "mono").front_center();

	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	// keyboard
	fp6000_kbd_device &keyboard(FP6000_KBD(config, "keyboard"));
	keyboard.int_handler().set(m_pic, FUNC(pic8259_device::ir1_w));

	// cassette
	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	// centronics printer
	output_latch_device &centronics_data_out(OUTPUT_LATCH(config, "centronics_data_out"));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->set_output_latch(centronics_data_out);
	m_centronics->ack_handler().set(m_pic, FUNC(pic8259_device::ir7_w)).invert();
	m_centronics->busy_handler().set(FUNC(fp6000_state::centronics_busy_w));
	m_centronics->fault_handler().set(FUNC(fp6000_state::centronics_fault_w));
	m_centronics->perror_handler().set(FUNC(fp6000_state::centronics_perror_w));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( fp6000 )
	ROM_REGION16_LE(0x10000, "ipl", 0)
	ROM_LOAD("ipl.rom", 0x0000, 0x10000, CRC(c72fe40a) SHA1(0e4c60dc27f6c7f461c4bc382b81602b3327a7a4))

	ROM_REGION(0x1000, "mcu", 0)
	ROM_LOAD("mcu", 0x0000, 0x1000, NO_DUMP) // unknown MCU type
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS        INIT         COMPANY  FULLNAME   FLAGS
COMP( 1985, fp6000, 0,      0,      fp6000,  fp6000, fp6000_state, empty_init, "Casio", "FP-6000", MACHINE_NOT_WORKING )
// Reportedly released as FP-6000 Jr in Scandinavia
