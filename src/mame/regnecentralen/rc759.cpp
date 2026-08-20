// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Regnecentralen RC759 Piccoline

    16-bit school/office micro (1984), sibling of the RC750 Partner (see
    rc750.cpp). Both derive from rc75x_state (see rc75x.h), which holds the
    shared 80186 + 8259A + 8255 + 82730 + MM58167 + NVM + sound + keyboard
    core; this file adds the Piccoline-specific floppy/cassette/expansion
    side. Runs Concurrent CP/M-86.

    HARDWARE OVERVIEW (from the PICCOLINE Programmer's Guide v2,
    CCP/M-86 3.1 / XIOS 2.3; verified against this driver):

      CPU            Intel 80186 @ 6 MHz (2 DMA ch, 3 timers, PIC on-chip)
      Extra PIC      Intel 8259A            I/O 0x00,  cascaded to 80186 INT0
      Keyboard       HLE serial kbd         I/O 0x20,  8259A IR1
      Sound          SN76489A               I/O 0x56
      RTC            MM58167 (32.768 kHz)   I/O 0x5a/0x5c, 8259A IR3
      CRT control    Intel 82730 text proc  I/O 0x60 (ctrl), 0x230 reset,
                                             0x240 chan-attn, sint -> IR4;
                                             32 KB pixel RAM @ 0xD0000
      Palette        32 cells x 2 IRGB nib  I/O 0x180-0x1be (even)
      PPI            Intel 8255             I/O 0x70-0x76 (port C bit6 = gfx)
      NVM            256x4 CMOS, bank-sw     I/O 0x80-0xfe
      Floppy         WD2797                 I/O 0x280-0x286, ctrl 0x288,
                                             reserve/release 0x28e-0x290,
                                             intrq -> IR0
      Parallel print Centronics (local)     I/O 0x250/0x260
      Cassette tape  (Piccoline only)       via PPI port A / port C
      Serial         iSBX serial module     iSBX slot 0x300-0x330
                     (optional; the Partner has a built-in 8274 instead)
      Net (optional) Intel 82586 Ethernet   I/O 0x100, IR5 (not emulated)
      DPC (optional) Disk/Printer-Adaptor   0x28a-0x28c, IR2 (not emulated)

    Video modes (82730, per the guide) -- only alphanumeric is currently
    emulated, and only in monochrome (see rc75x.cpp txt_update_row):
      alphanumeric        560x250, char cell from 32 KB pixel RAM
      high-res graphics   560x256, 1 bit/pixel   (m_gfx_mode, not emulated)
      medium-res graphics 280x256, 2 bits/pixel  (m_gfx_mode, not emulated)

    TODO:
    - Needs better I82730 emulation: use the 32-entry IRGB palette + the
      per-character palette-select bits instead of hard-coded black/white;
      implement the graphics (bitmap) mode selected via PPI port C bit 6
      (OUT 76H,0CH = graphics / 0DH = alphanumeric).
    - Floppy I/O errors
    - Many more things

    Notes:
    - Press SPACE during self-test for an extended menu

    References:
    - Intel 82730 Text Coprocessor datasheet (Preliminary), for the CRT
      controller programming model (mode block, cursor, soft scroll, etc.):
      https://archive.org/details/Intel-82730TextCoprocessor-PreliminaryOCR
    - PICCOLINE Programmer's Guide v2 (CCP/M-86 3.1 / XIOS 2.3) for the
      RC759-specific I/O map, NVM config layout and console escapes.

***************************************************************************/

#include "emu.h"
#include "rc75x.h"

#include "machine/wd_fdc.h"
#include "bus/centronics/ctronics.h"
#include "bus/isbx/isbx.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "formats/rc759_dsk.h"
#include "speaker.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class rc759_state : public rc75x_state
{
public:
	rc759_state(const machine_config &mconfig, device_type type, const char *tag) :
		rc75x_state(mconfig, type, tag),
		m_cas(*this, "cas"),
		m_isbx(*this, "isbx"),
		m_centronics(*this, "centronics"),
		m_fdc(*this, "fdc"),
		m_floppy(*this, "fdc:%u", 0),
		m_cas_enabled(0), m_cas_data(0),
		m_centronics_strobe(0), m_centronics_init(0), m_centronics_select_in(0), m_centronics_busy(0),
		m_centronics_ack(0), m_centronics_fault(0), m_centronics_perror(0), m_centronics_select(0),
		m_centronics_data(0xff)
	{ }

	void rc759(machine_config &config);

private:
	required_device<cassette_image_device> m_cas;
	required_device<isbx_slot_device> m_isbx;
	required_device<centronics_device> m_centronics;
	required_device<wd2797_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;

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

	void i186_timer0_w(int state);

	void rc759_io(address_map &map) ATTR_COLD;
	void rc759_map(address_map &map) ATTR_COLD;

	int m_cas_enabled;
	int m_cas_data;

	bool m_floppy_reserved = false;

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
	map(0x40000, 0x5ffff).ram(); // 384K total, matches working PCE rc759 cfg.ram=384K
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
	data |= 0 << 5; // mem ident1 (bit4=1,bit5=0 = 384k installed, matches working PCE cfg.ram=384K)
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
//  MACHINE EMULATION
//**************************************************************************

void rc759_state::i186_timer0_w(int state)
{
	m_cas_data = 1;

	if (m_cas_enabled)
		m_cas_data = state ? 0 : 1;

	m_cas->output(m_cas_data ? -1.0 : 1.0);
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
	I80186(config, m_maincpu, 6'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &rc759_state::rc759_map);
	m_maincpu->set_addrmap(AS_IO, &rc759_state::rc759_io);
	m_maincpu->tmrout0_handler().set(FUNC(rc759_state::i186_timer0_w)); // cassette (Piccoline only)

	// shared 80186/8259/8255-slave/82730/rtc/nvm/sound/keyboard core
	add_common_devices(config);

	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(rc759_state::ppi_porta_r));
	m_ppi->in_pb_callback().set(FUNC(rc759_state::ppi_portb_r));
	m_ppi->out_pc_callback().set(FUNC(rc759_state::ppi_portc_w));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(rc759_state::centronics_busy_w));
	m_centronics->ack_handler().set(FUNC(rc759_state::centronics_ack_w));
	m_centronics->fault_handler().set(FUNC(rc759_state::centronics_fault_w));
	m_centronics->perror_handler().set(FUNC(rc759_state::centronics_perror_w));
	m_centronics->select_handler().set(FUNC(rc759_state::centronics_select_w));

	CASSETTE(config, m_cas);
	m_cas->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cas->add_route(ALL_OUTPUTS, "mono", 0.05);

	// expansion slot
	// FIXME: set clock to MCLK frequency
	ISBX_SLOT(config, m_isbx, 0, isbx_cards, nullptr);
	m_isbx->mintr0().set("maincpu", FUNC(i80186_cpu_device::int1_w));
	m_isbx->mintr1().set("maincpu", FUNC(i80186_cpu_device::int3_w));
	m_isbx->mdrqt().set("maincpu",  FUNC(i80186_cpu_device::drq0_w));

	// floppy
	WD2797(config, m_fdc, 2'000'000);
	m_fdc->intrq_wr_callback().set(m_pic, FUNC(pic8259_device::ir0_w));
	m_fdc->drq_wr_callback().set(m_maincpu, FUNC(i80186_cpu_device::drq0_w));

	FLOPPY_CONNECTOR(config, "fdc:0", rc759_floppies, "hd", rc759_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", rc759_floppies, "hd", rc759_state::floppy_formats);
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
COMP( 1984, rc759, 0,      0,      rc759,   rc759, rc759_state, empty_init, "Regnecentralen", "RC759 Piccoline", MACHINE_SUPPORTS_SAVE )
