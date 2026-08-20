// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Regnecentralen RC750 Partner

    16-bit office micro, sibling of the RC759 Piccoline (see rc759.cpp).
    Both derive from rc75x_state (see rc75x.h), which holds the shared
    80186 + 8259A + 8255 + 82730 + MM58167 + NVM + sound + keyboard core;
    this file adds the Partner-specific floppy/serial/mass-storage side.
    Runs Concurrent DOS.

    HARDWARE OVERVIEW (from the PARTNER Programmer's Guide v3, jun 1986;
    the shared core is verified against rc759, the Partner-specific ports
    are NOT yet verified -- Appendix B of the guide is an OCR-blank scanned
    figure, so the WD1797/8274/SCSI I/O addresses below are placeholders):

      CPU            Intel 80186 (2 DMA ch, 3 timers, PIC on-chip)
      Coprocessor    Intel 8087            optional (not emulated)
      Extra PIC      Intel 8259A           I/O 0x00, cascaded to 80186 INT0
      Keyboard       HLE serial kbd        I/O 0x20, 8259A IR1
      Sound          SN76489A              I/O 0x56
      RTC            MM58167 (32.768 kHz)  I/O 0x5a/0x5c, 8259A IR3
      CRT control    Intel 82730 text proc I/O 0x60 (ctrl), 0x230/0x240;
                                            32 KB pixel RAM @ 0xD0000
      Palette        32 cells x 2 IRGB nib I/O 0x180-0x1be (even)
      PPI            Intel 8255            I/O 0x70-0x76 (port C bit6 = gfx)
      NVM            256x4 CMOS, bank-sw    I/O 0x80-0xfe
      Floppy         WD1797 FDC            modelled as FD1797; standard
      Serial         Intel 8274 (dual)     standard; via 80186 INT1
      Mass storage   SCSI bus interface    standard (not emulated)
      I/O expansion  expansion connector   (not emulated)

    Differences from the Piccoline: WD1797 instead of WD2797, a built-in
    Intel 8274 dual serial controller and a SCSI host adapter instead of
    the Piccoline's cassette / iSBX slot / micronet / DPC options, and an
    optional 8087. No boot ROM dump is available yet, so the machine is
    flagged MACHINE_NOT_WORKING.

    TODO:
    - Obtain a Partner boot ROM dump (none on hampa.ch/pce or rc700.dk).
    - Verify the Partner I/O port map (guide Appendix B is unreadable);
      the WD1797/8274/SCSI addresses here are provisional.
    - Wire the SCSI host adapter and the optional 8087.
    - Same 82730 video limitations as the Piccoline (mono, no gfx mode).

***************************************************************************/

#include "emu.h"
#include "rc75x.h"

#include "machine/wd_fdc.h"
#include "machine/z80sio.h"
#include "imagedev/floppy.h"
#include "formats/rc759_dsk.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class rc750_state : public rc75x_state
{
public:
	rc750_state(const machine_config &mconfig, device_type type, const char *tag) :
		rc75x_state(mconfig, type, tag),
		m_fdc(*this, "fdc"),
		m_floppy(*this, "fdc:%u", 0),
		m_sio(*this, "sio")
	{ }

	void rc750(machine_config &config);

private:
	required_device<fd1797_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<i8274_device> m_sio;

	static void floppy_formats(format_registration &fr);
	void floppy_control_w(uint8_t data);

	uint8_t ppi_porta_r();
	uint8_t ppi_portb_r();
	void ppi_portc_w(uint8_t data);

	void rc750_io(address_map &map) ATTR_COLD;
	void rc750_map(address_map &map) ATTR_COLD;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void rc750_state::rc750_map(address_map &map)
{
	map(0x00000, 0x3ffff).ram();
	map(0x40000, 0x5ffff).ram();
	map(0xd0000, 0xd7fff).mirror(0x08000).ram().share("vram");
	map(0xe8000, 0xeffff).mirror(0x10000).rom().region("bios", 0);
}

void rc750_state::rc750_io(address_map &map)
{
	map.unmap_value_high();
	// shared core ports (verified against the Piccoline)
	map(0x000, 0x003).mirror(0x0c).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x020, 0x020).r(m_kbd, FUNC(rc759_kbd_hle_device::read));
	map(0x056, 0x056).w(m_snd, FUNC(sn76494_device::write));
	map(0x056, 0x057).nopr();
	map(0x05a, 0x05a).w(FUNC(rc750_state::rtc_data_w));
	map(0x05c, 0x05c).rw(FUNC(rc750_state::rtc_data_r), FUNC(rc750_state::rtc_addr_w));
	map(0x070, 0x077).mirror(0x08).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x080, 0x0ff).rw(FUNC(rc750_state::nvram_r), FUNC(rc750_state::nvram_w)).umask16(0x00ff);
	map(0x180, 0x1bf).rw(FUNC(rc750_state::palette_r), FUNC(rc750_state::palette_w)).umask16(0x00ff);
	map(0x230, 0x231).w(FUNC(rc750_state::txt_irst_w));
	map(0x240, 0x241).w(FUNC(rc750_state::txt_ca_w));
	// Partner-specific ports (PROVISIONAL - not verified against the guide)
	map(0x280, 0x287).rw(m_fdc, FUNC(fd1797_device::read), FUNC(fd1797_device::write)).umask16(0x00ff);
	map(0x288, 0x288).w(FUNC(rc750_state::floppy_control_w));
	map(0x2a0, 0x2a7).rw(m_sio, FUNC(i8274_device::ba_cd_r), FUNC(i8274_device::ba_cd_w)).umask16(0x00ff);
//  map(0x2c0, 0x2cf) SCSI host adapter (not emulated)
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( rc750 )
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

void rc750_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_RC759_FORMAT);
}

void rc750_state::floppy_control_w(uint8_t data)
{
	// bit layout provisional, modelled on the Piccoline's WD2797 control port
	logerror("floppy_control_w: %02x\n", data);

	m_fdc->set_floppy(m_floppy[BIT(data, 0)]->get_device());

	if (m_floppy[0]->get_device()) m_floppy[0]->get_device()->mon_w(!BIT(data, 1));
	if (m_floppy[1]->get_device()) m_floppy[1]->get_device()->mon_w(!BIT(data, 2));

	m_fdc->dden_w(BIT(data, 5));
}


//**************************************************************************
//  I/O (PPI)
//**************************************************************************

uint8_t rc750_state::ppi_porta_r()
{
	// PROVISIONAL. The Partner has no cassette/iSBX/DPC, so those inputs
	// (bits 0-3, 6 on the Piccoline) read as "not installed"; the memory
	// ident bits are kept the same as the working Piccoline setup.
	uint8_t data = 0;

	data |= 0 << 0; // no cassette input
	data |= 0 << 1; // no iSBX present
	data |= 0 << 2; // no iSBX opt0
	data |= 0 << 3; // no iSBX opt1
	data |= 1 << 4; // mem ident0
	data |= 0 << 5; // mem ident1 (bit4=1,bit5=0 = 384k installed)
	data |= 0 << 6; // dpc connect (0 = external floppy installed)
	data |= 1 << 7; // not used

	return data;
}

uint8_t rc750_state::ppi_portb_r()
{
	// PROVISIONAL, adapted from the Piccoline port B.
	uint8_t data = 0;

	data |= 1 << 0; // no micronet controller
	data |= 1 << 1; // rtc type
	data |= m_snd->ready_r() << 2;
	data |= 1 << 3; // not used
	data |= 1 << 4; // not used
	data |= m_config->read(); // monitor type and frequency
	data |= 1 << 7; // not used

	return data;
}

void rc750_state::ppi_portc_w(uint8_t data)
{
	// 7-------  keyboard enable
	// -6------  gfx mode
	// --54----  nvram bank
	// ----32--  drq source
	// ------10  unused on the Partner (cassette on the Piccoline)

	m_kbd->enable_w(BIT(data, 7));
	m_gfx_mode = BIT(data, 6);
	m_nvram_bank = (data >> 4) & 0x03;
	m_drq_source = (data >> 2) & 0x03;
}


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

static void rc750_floppies(device_slot_interface &device)
{
	device.option_add("hd", FLOPPY_525_HD);
}

void rc750_state::rc750(machine_config &config)
{
	I80186(config, m_maincpu, 6'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &rc750_state::rc750_map);
	m_maincpu->set_addrmap(AS_IO, &rc750_state::rc750_io);

	// shared 80186/8259/8255-slave/82730/rtc/nvm/sound/keyboard core
	add_common_devices(config);

	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(rc750_state::ppi_porta_r));
	m_ppi->in_pb_callback().set(FUNC(rc750_state::ppi_portb_r));
	m_ppi->out_pc_callback().set(FUNC(rc750_state::ppi_portc_w));

	// floppy (WD1797, modelled as FD1797)
	FD1797(config, m_fdc, 2'000'000);
	m_fdc->intrq_wr_callback().set(m_pic, FUNC(pic8259_device::ir0_w));
	m_fdc->drq_wr_callback().set(m_maincpu, FUNC(i80186_cpu_device::drq0_w));

	FLOPPY_CONNECTOR(config, "fdc:0", rc750_floppies, "hd", rc750_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", rc750_floppies, "hd", rc750_state::floppy_formats);

	// dual serial (Intel 8274); routed to the 80186 INT1 per the guide's
	// interrupt structure. Serial lines are not wired up yet.
	I8274(config, m_sio, 4'000'000);
	m_sio->out_int_callback().set(m_maincpu, FUNC(i80186_cpu_device::int1_w));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( rc750 )
	// No Partner boot ROM dump is available yet.
	ROM_REGION16_LE(0x8000, "bios", 0)
	ROM_LOAD("rc750.rom", 0x0000, 0x8000, NO_DUMP)
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY           FULLNAME         FLAGS
COMP( 1985, rc750, 0,      0,      rc750,   rc750, rc750_state, empty_init, "Regnecentralen", "RC750 Partner", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
