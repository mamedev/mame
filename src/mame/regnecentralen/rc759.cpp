// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Regnecentralen RC759 Piccoline

    TODO:
    - Needs better I82730 emulation
    - Floppy I/O errors
    - Many more things

    Notes:
    - Press SPACE during self-test for an extended menu

***************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "machine/i8255.h"
#include "machine/mm58167.h"
#include "machine/nvram.h"
#include "machine/pic8259.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "rc759_kbd.h"
#include "sound/sn76496.h"
#include "sound/spkrdev.h"
#include "video/i82730.h"
#include "bus/centronics/ctronics.h"
#include "bus/isbx/isbx.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "formats/rc759_dsk.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class rc759_state : public driver_device
{
public:
	rc759_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pic(*this, "pic"),
		m_nvram(*this, "nvram"),
		m_ppi(*this, "ppi"),
		m_txt(*this, "txt"),
		m_palette(*this, "palette"),
		m_cas(*this, "cas"),
		m_isbx(*this, "isbx"),
		m_speaker(*this, "speaker"),
		m_snd(*this, "snd"),
		m_rtc(*this, "rtc"),
		m_centronics(*this, "centronics"),
		m_fdc(*this, "fdc"),
		m_floppy(*this, "fdc:%u", 0),
		m_vram(*this, "vram"),
		m_config(*this, "config"),
		m_kbd(*this, "kbd"),
		m_cas_enabled(0), m_cas_data(0),
		m_drq_source(0),
		m_nvram_bank(0),
		m_gfx_mode(0),
		m_centronics_strobe(0), m_centronics_init(0), m_centronics_select_in(0), m_centronics_busy(0),
		m_centronics_ack(0), m_centronics_fault(0), m_centronics_perror(0), m_centronics_select(0),
		m_centronics_data(0xff)
	{ }

	void rc759(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<i80186_cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic;
	required_device<nvram_device> m_nvram;
	required_device<i8255_device> m_ppi;
	required_device<i82730_device> m_txt;
	required_device<palette_device> m_palette;
	required_device<cassette_image_device> m_cas;
	required_device<isbx_slot_device> m_isbx;
	required_device<speaker_sound_device> m_speaker;
	required_device<sn76489a_device> m_snd;
	required_device<mm58167_device> m_rtc;
	required_device<centronics_device> m_centronics;
	required_device<wd2797_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_shared_ptr<uint16_t> m_vram;
	required_ioport m_config;
	required_device<rc759_kbd_hle_device> m_kbd;

	static void floppy_formats(format_registration &fr);
	void floppy_control_w(uint8_t data);
	uint8_t floppy_ack_r();
	void floppy_reserve_w(uint8_t data);
	void floppy_release_w(uint8_t data);

	uint8_t ppi_porta_r();
	uint8_t ppi_portb_r();
	void ppi_portc_w(uint8_t data);

	void centronics_busy_w(int state);
	void centronics_ack_w(int state);
	void centronics_fault_w(int state);
	void centronics_perror_w(int state);
	void centronics_select_w(int state);

	uint8_t centronics_data_r();
	void centronics_data_w(uint8_t data);
	uint8_t centronics_control_r();
	void centronics_control_w(uint8_t data);

	I82730_UPDATE_ROW(txt_update_row);
	void txt_ca_w(uint16_t data);
	void txt_irst_w(uint16_t data);
	uint8_t palette_r(offs_t offset);
	void palette_w(offs_t offset, uint8_t data);

	void i186_timer0_w(int state);
	void i186_timer1_w(int state);

	void nvram_init(nvram_device &nvram, void *data, size_t size);
	uint8_t nvram_r(offs_t offset);
	void nvram_w(offs_t offset, uint8_t data);

	void rtc_data_w(uint8_t data);
	uint8_t rtc_data_r();
	void rtc_addr_w(uint8_t data);

	uint8_t irq_callback();

	void rc759_io(address_map &map) ATTR_COLD;
	void rc759_map(address_map &map) ATTR_COLD;

	std::vector<uint8_t> m_nvram_mem;

	int m_cas_enabled;
	int m_cas_data;
	int m_drq_source;
	int m_nvram_bank;
	int m_gfx_mode;

	bool m_floppy_reserved;

	uint8_t m_rtc_read_addr;
	uint8_t m_rtc_read_data;
	uint8_t m_rtc_write_addr;
	uint8_t m_rtc_write_data;
	uint8_t m_rtc_strobe;

	int m_centronics_strobe;
	int m_centronics_init;
	int m_centronics_select_in;
	int m_centronics_busy;
	int m_centronics_ack;
	int m_centronics_fault;
	int m_centronics_perror;
	int m_centronics_select;
	uint8_t m_centronics_data;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void rc759_state::rc759_map(address_map &map)
{
	map(0x00000, 0x3ffff).ram();
	map(0x40000, 0x7ffff).ram();
	map(0xd0000, 0xd7fff).mirror(0x08000).ram().share("vram");
	map(0xe8000, 0xeffff).mirror(0x10000).rom().region("bios", 0);
}

void rc759_state::rc759_io(address_map &map)
{
	map.unmap_value_high();
	map(0x000, 0x003).mirror(0x0c).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x020, 0x020).r(m_kbd, FUNC(rc759_kbd_hle_device::read));
	map(0x056, 0x056).w(m_snd, FUNC(sn76494_device::write));
	map(0x056, 0x057).nopr();
	map(0x05a, 0x05a).w(FUNC(rc759_state::rtc_data_w));
	map(0x05c, 0x05c).rw(FUNC(rc759_state::rtc_data_r), FUNC(rc759_state::rtc_addr_w));
//  map(0x060, 0x06f).w(FUNC(rc759_state::crt_control_w)).umask16(0x00ff);
	map(0x070, 0x077).mirror(0x08).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x080, 0x0ff).rw(FUNC(rc759_state::nvram_r), FUNC(rc759_state::nvram_w)).umask16(0x00ff);
//  map(0x100, 0x101) net
	map(0x180, 0x1bf).rw(FUNC(rc759_state::palette_r), FUNC(rc759_state::palette_w)).umask16(0x00ff);
	map(0x230, 0x231).w(FUNC(rc759_state::txt_irst_w));
	map(0x240, 0x241).w(FUNC(rc759_state::txt_ca_w));
	map(0x250, 0x250).rw(FUNC(rc759_state::centronics_data_r), FUNC(rc759_state::centronics_data_w));
	map(0x260, 0x260).rw(FUNC(rc759_state::centronics_control_r), FUNC(rc759_state::centronics_control_w));
	map(0x280, 0x287).rw(m_fdc, FUNC(wd2797_device::read), FUNC(wd2797_device::write)).umask16(0x00ff);
	map(0x288, 0x288).w(FUNC(rc759_state::floppy_control_w));
//  map(0x28a, 0x28b) external printer data
//  map(0x28d, 0x28d) external printer control
	map(0x28e, 0x28e).rw(FUNC(rc759_state::floppy_ack_r), FUNC(rc759_state::floppy_reserve_w));
	map(0x290, 0x290).w(FUNC(rc759_state::floppy_release_w));
//  map(0x292, 0x293).rw(FUNC(rc759_state::printer_ack_r), FUNC(rc759_state::printer_reserve_w)).umask16(0x00ff);
//  map(0x294, 0x295).w(FUNC(rc759_state::printer_release_w)).umask16(0x00ff);
	map(0x300, 0x30f).rw(m_isbx, FUNC(isbx_slot_device::mcs0_r), FUNC(isbx_slot_device::mcs0_w)).umask16(0x00ff);
	map(0x310, 0x31f).rw(m_isbx, FUNC(isbx_slot_device::mcs1_r), FUNC(isbx_slot_device::mcs1_w)).umask16(0x00ff);
//  map(0x320, 0x321) isbx dma ack
//  map(0x330, 0x331) isbx tc
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( rc759 )
	PORT_START("config")
	PORT_CONFNAME(0x20, 0x00, "Monitor Type")
	PORT_CONFSETTING(0x00, "Color")
	PORT_CONFSETTING(0x20, "Monochrome")
	PORT_CONFNAME(0x40, 0x00, "Monitor Frequency")
	PORT_CONFSETTING(0x00, "15 kHz")
	PORT_CONFSETTING(0x40, "22 kHz")
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

I82730_UPDATE_ROW( rc759_state::txt_update_row )
{
	for (int i = 0; i < x_count; i++)
	{
		uint16_t gfx = m_vram[(data[i] & 0x3ff) << 4 | lc];

		// pretty crude detection if char sizes have been initialized, need something better
		if ((gfx & 0xff) == 0)
			continue;

		// figure out char width
		int width;
		for (width = 0; width < 16; width++)
			if (BIT(gfx, width) == 0)
				break;

		width = 15 - width;

		for (int p = 0; p < width; p++)
			bitmap.pix(y, i * width + p) = BIT(gfx, 15 - p) ? rgb_t::white() : rgb_t::black();
	}
}

void rc759_state::txt_ca_w(uint16_t data)
{
	m_txt->ca_w(1);
	m_txt->ca_w(0);
}

void rc759_state::txt_irst_w(uint16_t data)
{
	m_txt->irst_w(1);
	m_txt->irst_w(0);
}

uint8_t rc759_state::palette_r(offs_t offset)
{
	// not sure if it's possible to read back
	logerror("palette_r(%02x)\n", offset);
	return 0xff;
}

void rc759_state::palette_w(offs_t offset, uint8_t data)
{
	logerror("palette_w(%02x): %02x\n", offset, data);

	// two colors/byte. format: IRGBIRGB
	static constexpr uint8_t val[4] = { 0x00, 0x55, 0xaa, 0xff };
	int r, g, b;

	r = (BIT(data, 2) << 1) | BIT(data, 3);
	g = (BIT(data, 1) << 1) | BIT(data, 3);
	b = (BIT(data, 0) << 1) | BIT(data, 3);

	m_palette->set_pen_color(offset * 2 + 0, rgb_t(val[r], val[g], val[b]));

	r = (BIT(data, 6) << 1) | BIT(data, 7);
	g = (BIT(data, 5) << 1) | BIT(data, 7);
	b = (BIT(data, 4) << 1) | BIT(data, 7);

	m_palette->set_pen_color(offset * 2 + 1, rgb_t(val[r], val[g], val[b]));
}


//**************************************************************************
//  FLOPPY
//**************************************************************************

void rc759_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_RC759_FORMAT);
}

void rc759_state::floppy_control_w(uint8_t data)
{
	// 7-------  ready control
	// -6------  fdc clock?
	// --5-----  dden?
	// ---4----  precomp 125/250 nsec?
	// ----3---  write precomp
	// -----2--  motor 1
	// ------1-  motor 0
	// -------0  drive select

	logerror("floppy_control_w: %02x\n", data);

	m_fdc->set_floppy(m_floppy[BIT(data, 0)]->get_device());

	if (m_floppy[0]->get_device()) m_floppy[0]->get_device()->mon_w(!BIT(data, 1));
	if (m_floppy[1]->get_device()) m_floppy[1]->get_device()->mon_w(!BIT(data, 2));

	m_fdc->dden_w(BIT(data, 5));
	m_fdc->set_unscaled_clock(BIT(data, 6) ? 2000000 : 1000000);
	m_fdc->set_force_ready(BIT(data, 7));
}

uint8_t rc759_state::floppy_ack_r()
{
	// 7-------  floppy ack
	// -6543210  unused?

	return m_floppy_reserved ? 0x00 : 0x80;
}

void rc759_state::floppy_reserve_w(uint8_t data)
{
	m_floppy_reserved = true;
}

void rc759_state::floppy_release_w(uint8_t data)
{
	m_floppy_reserved = false;
}


//**************************************************************************
//  I/O
//**************************************************************************

uint8_t rc759_state::ppi_porta_r()
{
	uint8_t data = 0;

	data |= m_cas_enabled ? m_cas_data : (m_cas->input() > 0 ? 1 : 0);
	data |= m_isbx->mpst_r() << 1;
	data |= m_isbx->opt0_r() << 2;
	data |= m_isbx->opt1_r() << 3;
	data |= 1 << 4; // mem ident0
	data |= 1 << 5; // mem ident1 (both 1 = 256k installed)
	data |= 0 << 6; // dpc connect (0 = external floppy/printer installed)
	data |= 1 << 7; // not used

	return data;
}

uint8_t rc759_state::ppi_portb_r()
{
	uint8_t data = 0;

	data |= 1 << 0; // 0 = micronet controller installed
	data |= 1 << 1; // rtc type, mm58167/cdp1879
	data |= m_snd->ready_r() << 2;
	data |= 1 << 3; // not used
	data |= 1 << 4; // not used
	data |= m_config->read(); // monitor type and frequency
	data |= 1 << 7; // 0 = enable remote hardware debug (using an isbx351 module)

	return data;
}

void rc759_state::ppi_portc_w(uint8_t data)
{
	// 7-------  keyboard enable
	// -6------  gfx mode
	// --54----  nvram bank
	// ----32--  drq source
	// ------1-  cassette motor
	// -------0  cassette enable

	m_kbd->enable_w(BIT(data, 7));
	m_gfx_mode = BIT(data, 6);
	m_nvram_bank = (data >> 4) & 0x03;
	m_drq_source = (data >> 2) & 0x03;
	m_cas->change_state(BIT(data, 1) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);
	m_cas_enabled = BIT(data, 0);
}

void rc759_state::centronics_busy_w(int state)
{
	m_centronics_busy = state;
	m_pic->ir6_w(state);
}

void rc759_state::centronics_ack_w(int state)
{
	m_centronics_ack = state;
}

void rc759_state::centronics_fault_w(int state)
{
	m_centronics_fault = state;
}

void rc759_state::centronics_perror_w(int state)
{
	m_centronics_perror = state;
}

void rc759_state::centronics_select_w(int state)
{
	m_centronics_select = state;
}

uint8_t rc759_state::centronics_data_r()
{
	return m_centronics_data;
}

void rc759_state::centronics_data_w(uint8_t data)
{
	m_centronics_data = data;

	m_centronics->write_data0(BIT(data, 0));
	m_centronics->write_data1(BIT(data, 1));
	m_centronics->write_data2(BIT(data, 2));
	m_centronics->write_data3(BIT(data, 3));
	m_centronics->write_data4(BIT(data, 4));
	m_centronics->write_data5(BIT(data, 5));
	m_centronics->write_data6(BIT(data, 6));
	m_centronics->write_data7(BIT(data, 7));
}

uint8_t rc759_state::centronics_control_r()
{
	uint8_t data = 0;

	data |= m_centronics_busy << 0;
	data |= m_centronics_ack << 1;
	data |= m_centronics_fault << 2;
	data |= m_centronics_perror << 3;
	data |= m_centronics_select << 4;
	data |= !m_centronics_strobe << 5;
	data |= !m_centronics_init << 6;
	data |= !m_centronics_select_in << 7;

	return data;
}

void rc759_state::centronics_control_w(uint8_t data)
{
	logerror("centronics_control_w: %02x\n", data);

	m_centronics_strobe = BIT(data, 0);
	m_centronics_init = BIT(data, 2);
	m_centronics_select_in = BIT(data, 4);

	m_centronics->write_strobe(m_centronics_strobe);
	m_centronics->write_autofd(BIT(data, 1));
	m_centronics->write_init(m_centronics_init);
	m_centronics->write_select_in(m_centronics_select_in);
}


//**************************************************************************
//  SOUND/RTC
//**************************************************************************

void rc759_state::rtc_data_w(uint8_t data)
{
	m_rtc_write_data = data;
}

uint8_t rc759_state::rtc_data_r()
{
	return m_rtc_read_data;
}

void rc759_state::rtc_addr_w(uint8_t data)
{
	if (BIT(data, 7))
		m_rtc_read_addr = data & 0x1f;
	else
		m_rtc_write_addr = data & 0x1f;

	if (BIT(data, 6) && BIT(m_rtc_strobe, 6) == 0)
		m_rtc->write(m_rtc_write_addr, m_rtc_write_data);

	if (BIT(data, 5) && BIT(m_rtc_strobe, 5) == 0)
		m_rtc_read_data = m_rtc->read(m_rtc_read_addr);

	m_rtc_strobe = data;
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void rc759_state::i186_timer0_w(int state)
{
	m_cas_data = 1;

	if (m_cas_enabled)
		m_cas_data = state ? 0 : 1;

	m_cas->output(m_cas_data ? -1.0 : 1.0);
}

void rc759_state::i186_timer1_w(int state)
{
	m_speaker->level_w(state);
}

// 256x4 nvram is bank-switched using ppi port c, bit 4 and 5
void rc759_state::nvram_init(nvram_device &nvram, void *data, size_t size)
{
	memset(data, 0x00, size);
	memset(data, 0xaa, 1);
}

uint8_t rc759_state::nvram_r(offs_t offset)
{
	offs_t addr = (m_nvram_bank << 6) | offset;

	logerror("nvram_r(%02x)\n", addr);

	if (addr & 1)
		return (m_nvram_mem[addr >> 1] & 0xf0) >> 4;
	else
		return (m_nvram_mem[addr >> 1] & 0x0f) >> 0;
}

void rc759_state::nvram_w(offs_t offset, uint8_t data)
{
	offs_t addr = (m_nvram_bank << 6) | offset;

	logerror("nvram_w(%02x): %02x\n", addr, data);

	if (addr & 1)
		m_nvram_mem[addr >> 1] = ((data << 4) & 0xf0) | (m_nvram_mem[addr >> 1] & 0x0f);
	else
		m_nvram_mem[addr >> 1] = (m_nvram_mem[addr >> 1] & 0xf0) | (data & 0x0f);
}

uint8_t rc759_state::irq_callback()
{
	return m_pic->acknowledge();
}

void rc759_state::machine_start()
{
	m_nvram_mem.resize(256 / 2);
	m_nvram->set_base(&m_nvram_mem[0], 256 / 2);
}

void rc759_state::machine_reset()
{
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

static void rc759_floppies(device_slot_interface &device)
{
	device.option_add("hd", FLOPPY_525_HD);
}

void rc759_state::rc759(machine_config &config)
{
	I80186(config, m_maincpu, 6000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &rc759_state::rc759_map);
	m_maincpu->set_addrmap(AS_IO, &rc759_state::rc759_io);
	m_maincpu->read_slave_ack_callback().set(FUNC(rc759_state::irq_callback));
	m_maincpu->tmrout0_handler().set(FUNC(rc759_state::i186_timer0_w));
	m_maincpu->tmrout1_handler().set(FUNC(rc759_state::i186_timer1_w));

	PIC8259(config, m_pic);
	m_pic->out_int_callback().set(m_maincpu, FUNC(i80186_cpu_device::int0_w));

	NVRAM(config, "nvram").set_custom_handler(FUNC(rc759_state::nvram_init));

	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(rc759_state::ppi_porta_r));
	m_ppi->in_pb_callback().set(FUNC(rc759_state::ppi_portb_r));
	m_ppi->out_pc_callback().set(FUNC(rc759_state::ppi_portc_w));

	MM58167(config, "rtc", 32.768_kHz_XTAL).irq().set(m_pic, FUNC(pic8259_device::ir3_w));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(rc759_state::centronics_busy_w));
	m_centronics->ack_handler().set(FUNC(rc759_state::centronics_ack_w));
	m_centronics->fault_handler().set(FUNC(rc759_state::centronics_fault_w));
	m_centronics->perror_handler().set(FUNC(rc759_state::centronics_perror_w));
	m_centronics->select_handler().set(FUNC(rc759_state::centronics_select_w));

	// video
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(1250000 * 16, 896, 96, 816, 377, 4, 364); // 22 kHz setting
	screen.set_screen_update("txt", FUNC(i82730_device::screen_update));
	screen.screen_vblank().set(m_maincpu, FUNC(i80186_cpu_device::tmrin0_w)); // TMRIN0 source not documented, but self-test needs something like this

	I82730(config, m_txt, 1250000, m_maincpu);
	m_txt->set_screen("screen");
	m_txt->set_update_row_callback(FUNC(rc759_state::txt_update_row));
	m_txt->sint().set(m_pic, FUNC(pic8259_device::ir4_w));

	PALETTE(config, m_palette).set_entries(64);

	// sound
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);
	SN76489A(config, m_snd, 20_MHz_XTAL / 10).add_route(ALL_OUTPUTS, "mono", 1.0);

	CASSETTE(config, m_cas);
	m_cas->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cas->add_route(ALL_OUTPUTS, "mono", 0.05);

	// expansion slot
	ISBX_SLOT(config, m_isbx, 0, isbx_cards, nullptr);
	m_isbx->mintr0().set("maincpu", FUNC(i80186_cpu_device::int1_w));
	m_isbx->mintr1().set("maincpu", FUNC(i80186_cpu_device::int3_w));
	m_isbx->mdrqt().set("maincpu", FUNC(i80186_cpu_device::drq0_w));

	// floppy
	WD2797(config, m_fdc, 2000000);
	m_fdc->intrq_wr_callback().set(m_pic, FUNC(pic8259_device::ir0_w));
	m_fdc->drq_wr_callback().set(m_maincpu, FUNC(i80186_cpu_device::drq0_w));

	FLOPPY_CONNECTOR(config, "fdc:0", rc759_floppies, "hd", rc759_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", rc759_floppies, "hd", rc759_state::floppy_formats);

	// keyboard
	RC759_KBD_HLE(config, m_kbd);
	m_kbd->int_handler().set(m_pic, FUNC(pic8259_device::ir1_w));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( rc759 )
	ROM_REGION16_LE(0x8000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "1-21", "1? Version 2.1")
	ROMX_LOAD("rc759-1-2.1.rom", 0x0000, 0x8000, CRC(3a777d56) SHA1(a8592d61d5e1f92651a6f5e41c4ba14c9b6cc39b), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "1-51", "1? Version 5.1")
	ROMX_LOAD("rc759-1-5.1.rom", 0x0000, 0x8000, CRC(e1d53845) SHA1(902dc5ce28efd26b4f9c631933e197c2c187a7f1), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "2-40", "2? Version 4.0")
	ROMX_LOAD("rc759-2-4.0.rom", 0x0000, 0x8000, CRC(d3cb752a) SHA1(f50afe5dfa1b33a36a665d32d57c8c41d6685005), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "2-51", "2? Version 5.1")
	ROMX_LOAD("rc759-2-5.1.rom", 0x0000, 0x8000, CRC(00a31948) SHA1(23c4473c641606a56473791773270411d1019248), ROM_BIOS(3))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY           FULLNAME           FLAGS
COMP( 1984, rc759, 0,      0,      rc759,   rc759, rc759_state, empty_init, "Regnecentralen", "RC759 Piccoline", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
