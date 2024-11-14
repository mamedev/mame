// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Ampro Little Z80 Board

2013-09-02 Skeleton driver.

Chips: Z80A @4MHz, Z80CTC, Z80DART, WD1770/2, NCR5380. Crystal: 16 MHz

This is a complete CP/M single-board computer that could be mounted
on top of a standard 13cm floppy drive. You needed to supply your own
power supply and serial terminal.

The later versions included a SCSI chip (NCR5380) enabling the use
of a hard drive of up to 88MB.

****************************************************************************/

#include "emu.h"
#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/ncr5380.h"
#include "bus/nscsi/devices.h"
#include "machine/output_latch.h"
#include "machine/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "machine/wd_fdc.h"
#include "machine/timer.h"
#include "softlist_dev.h"


namespace {

class ampro_state : public driver_device
{
public:
	ampro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		, m_bank1(*this, "bank1")
		, m_dart(*this, "dart")
		, m_ctc(*this, "ctc")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:%u", 0U)
		, m_ncr(*this, "scsi:7:ncr")
		, m_printer(*this, "printer")
	{ }

	void ampro(machine_config &config);

private:
	TIMER_DEVICE_CALLBACK_MEMBER(ctc_tick);
	void port00_w(uint8_t data);
	void set_strobe(uint8_t data);
	void clear_strobe(uint8_t data);
	uint8_t ctc_r(offs_t offset);
	uint8_t dart_r(offs_t offset);
	void ctc_w(offs_t offset, uint8_t data);
	void dart_w(offs_t offset, uint8_t data);
	void machine_reset() override ATTR_COLD;
	void machine_start() override ATTR_COLD;

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
	required_memory_bank    m_bank1;
	required_device<z80dart_device> m_dart;
	required_device<z80ctc_device> m_ctc;
	required_device<wd1772_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppy;
	required_device<ncr5380_device> m_ncr;
	required_device<centronics_device> m_printer;
};

/*
d0..d3 Drive select 0-3
d4     Side select 0=side0
d5     /DDEN
d6     Banking 0=rom
d7     FDC master clock 0=8MHz 1=16MHz (for 20cm disks, not emulated)
*/
void ampro_state::port00_w(uint8_t data)
{
	m_bank1->set_entry(BIT(~data, 6));
	m_fdc->dden_w(BIT(data, 5));
	floppy_image_device *floppy = nullptr;
	for (int n = 0; n < 4; n++)
		if (BIT(data, n))
			floppy = m_floppy[n]->get_device();
	m_fdc->set_floppy(floppy);
	if (floppy)
		floppy->ss_w(BIT(data, 4));
}

void ampro_state::set_strobe(uint8_t data)
{
	m_printer->write_strobe(0);
}

void ampro_state::clear_strobe(uint8_t data)
{
	m_printer->write_strobe(1);
}

uint8_t ampro_state::ctc_r(offs_t offset)
{
	return m_ctc->read(offset>>4);
}

uint8_t ampro_state::dart_r(offs_t offset)
{
	return m_dart->ba_cd_r(offset>>2);
}

void ampro_state::ctc_w(offs_t offset, uint8_t data)
{
	m_ctc->write(offset>>4, data);
}

void ampro_state::dart_w(offs_t offset, uint8_t data)
{
	m_dart->ba_cd_w(offset>>2, data);
}

void ampro_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).ram().share("mainram");
	map(0x0000, 0x0fff).bankr("bank1");
}

void ampro_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(ampro_state::port00_w)); // system
	map(0x01, 0x01).w("pio", FUNC(output_latch_device::write)); // printer data
	map(0x02, 0x02).w(FUNC(ampro_state::set_strobe)); // printer strobe
	map(0x03, 0x03).w(FUNC(ampro_state::clear_strobe)); // printer strobe
	map(0x20, 0x27).rw(m_ncr, FUNC(ncr5380_device::read), FUNC(ncr5380_device::write)); // scsi chip
	map(0x28, 0x28).rw(m_ncr, FUNC(ncr5380_device::dma_r), FUNC(ncr5380_device::dma_w)); // scsi control
	map(0x29, 0x29).portr("ID"); // ID port
	map(0x40, 0x7f).rw(FUNC(ampro_state::ctc_r), FUNC(ampro_state::ctc_w));
	map(0x80, 0x8f).rw(FUNC(ampro_state::dart_r), FUNC(ampro_state::dart_w));
	map(0xc0, 0xc3).w(m_fdc, FUNC(wd1772_device::write));
	map(0xc4, 0xc7).r(m_fdc, FUNC(wd1772_device::read));
}

static const z80_daisy_config daisy_chain_intf[] =
{
	{ "ctc" },
	{ "dart" },
	{ nullptr }
};

static void ampro_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("525qd", FLOPPY_525_QD);
}

/* Input ports */
static INPUT_PORTS_START( ampro )
	PORT_START("ID")
	PORT_DIPUNUSED_DIPLOC(0x80, 0x80, "J9:1") // actually pin pair 1-2
	PORT_DIPUNUSED_DIPLOC(0x40, 0x40, "J9:2") // actually pin pair 3-4
	PORT_DIPUNUSED_DIPLOC(0x20, 0x20, "J9:3") // actually pin pair 5-6
	PORT_DIPUNUSED_DIPLOC(0x10, 0x10, "J9:4") // actually pin pair 7-8
	PORT_DIPUNUSED_DIPLOC(0x08, 0x08, "J9:5") // actually pin pair 9-10
	PORT_DIPNAME(0x07, 0x07, "SCSI ID") PORT_DIPLOCATION("J9:8,7,6") // actually pin pairs 15-16, 13-14, 11-12
	PORT_DIPSETTING(0x00, "0")
	PORT_DIPSETTING(0x01, "1")
	PORT_DIPSETTING(0x02, "2")
	PORT_DIPSETTING(0x03, "3")
	PORT_DIPSETTING(0x04, "4")
	PORT_DIPSETTING(0x05, "5")
	PORT_DIPSETTING(0x06, "6")
	PORT_DIPSETTING(0x07, "7")
INPUT_PORTS_END

void ampro_state::machine_start()
{
	m_bank1->configure_entry(0, m_ram);
	m_bank1->configure_entry(1, m_rom);
}

void ampro_state::machine_reset()
{
	m_bank1->set_entry(1);

	port00_w(0);
	clear_strobe(0);
}

void ampro_state::ampro(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &ampro_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &ampro_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain_intf);

	/* Devices */
	Z80CTC(config, m_ctc, 16_MHz_XTAL / 4);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->set_clk<0>(16_MHz_XTAL / 8); // 2MHz
	m_ctc->set_clk<1>(16_MHz_XTAL / 8); // 2MHz
	m_ctc->zc_callback<0>().set(m_dart, FUNC(z80dart_device::txca_w));    // Z80DART Ch A, SIO Ch A
	m_ctc->zc_callback<0>().append(m_dart, FUNC(z80dart_device::rxca_w));
	m_ctc->zc_callback<1>().set(m_dart, FUNC(z80dart_device::rxtxcb_w));   // SIO Ch B

	Z80DART(config, m_dart, 16_MHz_XTAL / 4);
	m_dart->out_txda_callback().set("rs232a", FUNC(rs232_port_device::write_txd));
	m_dart->out_rtsa_callback().set("rs232a", FUNC(rs232_port_device::write_rts));
	m_dart->out_txdb_callback().set("rs232b", FUNC(rs232_port_device::write_txd));
	m_dart->out_rtsb_callback().set("rs232b", FUNC(rs232_port_device::write_rts));
	m_dart->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, "terminal"));
	rs232a.rxd_handler().set(m_dart, FUNC(z80dart_device::rxa_w));
	rs232a.cts_handler().set(m_dart, FUNC(z80dart_device::ctsa_w));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_dart, FUNC(z80dart_device::rxb_w));
	rs232b.cts_handler().set(m_dart, FUNC(z80dart_device::ctsb_w));

	output_latch_device &pio(OUTPUT_LATCH(config, "pio"));
	CENTRONICS(config, m_printer, centronics_devices, nullptr);
	m_printer->busy_handler().set(m_dart, FUNC(z80dart_device::rib_w));
	m_printer->set_output_latch(pio);

	WD1772(config, m_fdc, 16_MHz_XTAL / 2);
	//m_fdc->intrq_wr_callback().set(m_ctc, FUNC(z80ctc_device::trg3)); // only if JMP2-3 shorted
	//m_fdc->drq_wr_callback().set(m_dart, FUNC(z80dart_device::ria_w)); // only if JMP7 shorted
	m_fdc->ready_wr_callback().set(m_dart, FUNC(z80dart_device::dcdb_w)); // actually from the drive, and not used by the FDC at all
	for (auto &floppy : m_floppy)
		FLOPPY_CONNECTOR(config, floppy, ampro_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	SOFTWARE_LIST(config, "flop_list").set_original("ampro");

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("ncr", NCR5380).machine_config([] (device_t *device) {
		//downcast<ncr5380_device &>(*device).irq_handler().set(m_ctc, FUNC(z80ctc_device::trg2)); // only if JMP3 shorted
		//downcast<ncr5380_device &>(*device).drq_handler().set(m_dart, FUNC(z80dart_device::dcda_w)); // only if JMP8 shorted
	});
}

/* ROM definition */
ROM_START( ampro )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "mntr", "Monitor")
	ROMX_LOAD( "mntr", 0x0000, 0x1000, CRC(d59d0909) SHA1(936410f414b1e71445253840eea0045545e4ff0b), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "boot", "Boot")
	ROMX_LOAD( "boot", 0x0000, 0x1000, CRC(b3524046) SHA1(5466f7d28c1a04cfbf328095cb35ad1525e91f44), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "scsi", "SCSI Boot")
	ROMX_LOAD( "scsi", 0x0000, 0x1000, CRC(8eb20e5d) SHA1(0ab1ff65cf6d3c1a713a8ac5c1ee4c662ac3da0c), ROM_BIOS(2))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME            FLAGS
COMP( 1980, ampro, 0,      0,      ampro,   ampro, ampro_state, empty_init, "Ampro", "Little Z80 Board", MACHINE_SUPPORTS_SAVE )
