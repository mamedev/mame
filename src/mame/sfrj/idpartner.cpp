// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/******************************************************************************

    Iskra Delta produced:
        1984 - Partner series
            model with Winchester disk + floppy
        1987 - Partner G series
            1F/G - model with 1 floppy disk
            2F/G - model with 2 floppy disks
            WF/G - model with Winchester disk + floppy

    Schematics and info at :
        https://github.com/tstih/idp-doc/tree/main/iskra-delta
        http://matejhorvat.si/sl/slorac/delta/partner/index.htm

*******************************************************************************/

#include "emu.h"

#include "idpart_video.h"

#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "formats/idpart_dsk.h"
#include "machine/mc14411.h"
#include "machine/mm58167.h"
#include "machine/timer.h"
#include "machine/upd765.h"
#include "machine/z80ctc.h"
#include "machine/z80dma.h"
#include "machine/z80daisy.h"
#include "machine/z80daisy_generic.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "bus/idpartner/bus.h"
#include "bus/rs232/rs232.h"

// Partner use for all pheriperials Z80 daisy chain, and since Intel 8272
// does not support it, there is actual implementation done in logic on the
// board, that does clear interrupt status on reset and acknowledge
class idpartner_floppy_daisy_device : public device_t, public device_z80daisy_interface
{
public:
	idpartner_floppy_daisy_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	virtual ~idpartner_floppy_daisy_device() {};

	// callbacks
	auto int_handler() { return m_int_handler.bind(); }

	void set_vector(uint8_t vector) { m_vector = vector; }
	void int_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

private:
	devcb_write_line m_int_handler;

	int m_int;
	bool m_ius;
	int m_vector;
};

void idpartner_floppy_daisy_device::int_w(int state)
{
	m_int = state;
	m_int_handler(state);
}

void idpartner_floppy_daisy_device::device_start()
{
	save_item(NAME(m_int));
	save_item(NAME(m_ius));
	save_item(NAME(m_vector));
}

void idpartner_floppy_daisy_device::device_reset()
{
	m_int = CLEAR_LINE;
	m_ius = false;
	m_vector = 0xff;
}

int idpartner_floppy_daisy_device::z80daisy_irq_state()
{
	if (m_int)
		return Z80_DAISY_INT;
	else if (m_ius)
		return Z80_DAISY_IEO;
	else
		return 0;
}

int idpartner_floppy_daisy_device::z80daisy_irq_ack()
{
	if (m_int) {
		int_w(CLEAR_LINE);
		m_ius = true;
		return m_vector;
	}
	return 0;
}

void idpartner_floppy_daisy_device::z80daisy_irq_reti()
{
	if (m_ius)
		m_ius = false;
}

DEFINE_DEVICE_TYPE(IDPARTNER_FLOPPY_DAISY, idpartner_floppy_daisy_device, "idpartner_floppy_daisy", "Iskra Delta Partner floppy daisy chain device")

idpartner_floppy_daisy_device::idpartner_floppy_daisy_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, IDPARTNER_FLOPPY_DAISY, tag, owner, clock)
	, device_z80daisy_interface(mconfig, *this)
	, m_int_handler(*this)
	, m_int(0)
	, m_ius(false)
	, m_vector(0xff)
{
}

namespace {

class idpartner_state : public driver_device
{
public:
	idpartner_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_bus(*this, "j2")
		, m_conn(*this, "j2:%d", 0U)
		, m_sio1(*this, "sio1")
		, m_sio2(*this, "sio2")
		, m_ctc(*this, "ctc")
		, m_dma(*this, "dma")
		, m_pio(*this, "pio")
		, m_fdc(*this, "fdc")
		, m_fdc_daisy(*this, "fdc_daisy")
		, m_brg(*this, "brg")
		, m_rtc(*this, "rtc")
		, m_serial(*this, "serial_%d",0U)
		, m_floppy(*this, "fdc:%d",0U)
		, m_ram(*this, "ram", 0x20000, ENDIANNESS_LITTLE)
		, m_rom(*this, "maincpu")
		, m_bankr0(*this, "bankr0")
		, m_bankw0(*this, "bankw0")
		, m_bank1(*this, "bank1")
		, m_bank2(*this, "bank2")
	{ }

	void partner_base(machine_config &config);
	void partnerw(machine_config &config);
	void partner1fg(machine_config &config);
	void partnerwfg(machine_config &config);

	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

private:
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	void update_bank();

	u8 rom_bank_r() { m_rom_enabled = false; update_bank(); return 0xff; }
	void rom_bank_w(u8 data) { m_rom_enabled = false; update_bank(); }
	u8 bank1_r() { m_bank = 0; update_bank(); return 0xff; }
	void bank1_w(u8 data) { m_bank = 0; update_bank(); }
	u8 bank2_r() { m_bank = 1; update_bank(); return 0xff; }
	void bank2_w(u8 data) { m_bank = 1; update_bank(); }
	void fdc_vector_w(u8 data) { m_fdc_daisy->set_vector(data); }
	void floppy_motor_w(u8 data) { m_floppy_motor = 1; update_floppy_motor(); }
	u8 floppy_motor_r() { return m_floppy_motor; }
	void update_floppy_motor();
	void xx2_w(int state) { if (state) { m_floppy_motor = 0; update_floppy_motor(); } }
	void write_speed1_clock(int state) { m_sio1->txca_w(state); m_sio1->rxca_w(state); }
	void write_speed2_clock(int state) { m_sio1->txcb_w(state); m_sio1->rxcb_w(state); }
	void write_speed3_clock(int state) { m_sio2->txca_w(state); m_sio2->rxca_w(state); }
	void write_speed4_clock(int state) { m_sio2->txcb_w(state); m_sio2->rxcb_w(state); }
	uint8_t memory_read_byte(offs_t offset) { return m_program.read_byte(offset); }
	void memory_write_byte(offs_t offset, uint8_t data) { m_program.write_byte(offset, data); }
	uint8_t io_read_byte(offs_t offset) { if (offset==0xf1) return m_fdc->dma_r(); else return m_io.read_byte(offset); }
	void io_write_byte(offs_t offset, uint8_t data) { if (offset==0xf1) m_fdc->dma_w(data); else m_io.write_byte(offset, data); }

	u8 m_bank;
	bool m_rom_enabled;
	u8 m_floppy_motor;

	required_device<z80_device> m_maincpu;
	required_device<bus::idpartner::bus_device> m_bus;
	required_device_array<bus::idpartner::bus_connector_device, 2> m_conn;
	required_device<z80sio_device> m_sio1;
	required_device<z80sio_device> m_sio2;
	required_device<z80ctc_device> m_ctc;
	optional_device<z80dma_device> m_dma;
	required_device<z80pio_device> m_pio;
	required_device<i8272a_device> m_fdc;
	required_device<idpartner_floppy_daisy_device> m_fdc_daisy;
	required_device<mc14411_device> m_brg;
	required_device<mm58167_device> m_rtc;
	required_device_array<rs232_port_device,4> m_serial;
	required_device_array<floppy_connector, 2> m_floppy;
	memory_share_creator<u8> m_ram;
	required_region_ptr<u8> m_rom;
	required_memory_bank m_bankr0;
	required_memory_bank m_bankw0;
	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_program;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_io;
};

void idpartner_state::machine_start()
{
	// 16 KB actually unused
	m_bankr0->configure_entry(0, m_ram + 0x00000);
	m_bankr0->configure_entry(1, m_ram + 0x10000);
	m_bankr0->configure_entry(2, m_rom);
	m_bankw0->configure_entry(0, m_ram + 0x00000);
	m_bankw0->configure_entry(1, m_ram + 0x10000);
	m_bankw0->configure_entry(2, m_rom);

	m_bank1->configure_entry(0, m_ram + 0x02000);
	m_bank1->configure_entry(1, m_ram + 0x12000);

	// Last 16KB is always same
	m_bank2->configure_entry(0, m_ram + 0x0c000);
	m_bank2->set_entry(0);

	m_maincpu->space(AS_PROGRAM).specific(m_program);
	m_maincpu->space(AS_IO).specific(m_io);
}

void idpartner_state::update_bank()
{
	if (m_rom_enabled)
		m_bankr0->set_entry(2);
	else
	{
		m_bankr0->set_entry(m_bank);
		m_bankw0->set_entry(m_bank);
	}
	m_bank1->set_entry(m_bank);
}

void idpartner_state::update_floppy_motor()
{
	for (auto &drive : m_floppy)
	{
		if (drive->get_device())
			drive->get_device()->mon_w(m_floppy_motor ^ 1);
	}
}

void idpartner_state::machine_reset()
{
	m_bank = 0;
	m_floppy_motor = 0;
	update_floppy_motor();
	m_rom_enabled = true;
	update_bank();
}

/* Address maps */
void idpartner_state::mem_map(address_map &map)
{
	map(0x0000, 0x1fff).bankr("bankr0").bankw("bankw0");
	map(0x2000, 0xbfff).bankrw("bank1");
	map(0xc000, 0xffff).bankrw("bank2");
}
void idpartner_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x80,0x87).rw(FUNC(idpartner_state::rom_bank_r), FUNC(idpartner_state::rom_bank_w)); // ROM bank
	map(0x88,0x8f).rw(FUNC(idpartner_state::bank1_r), FUNC(idpartner_state::bank1_w)); // RAM bank 1
	map(0x90,0x97).rw(FUNC(idpartner_state::bank2_r), FUNC(idpartner_state::bank2_w)); // RAM bank 2
	map(0x98,0x9f).rw(FUNC(idpartner_state::floppy_motor_r), FUNC(idpartner_state::floppy_motor_w)); // floppy motors
	map(0xa0,0xbf).rw(m_rtc, FUNC(mm58167_device::read), FUNC(mm58167_device::write));
	map(0xc0,0xc7).rw(m_dma, FUNC(z80dma_device::read), FUNC(z80dma_device::write));
	map(0xc8,0xcb).mirror(0x04).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));  // CTC - A2 not connected
	map(0xd0,0xd3).mirror(0x04).rw(m_pio, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0xd8,0xdb).mirror(0x04).rw(m_sio1, FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w)); // SIO1 - A2 not connected
	map(0xe0,0xe3).mirror(0x04).rw(m_sio2, FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w)); // SIO2 - A2 not connected
	map(0xe8,0xef).w(FUNC(idpartner_state::fdc_vector_w)); // FDC interrupt vector
	map(0xf0,0xf1).mirror(0x06).m(m_fdc, FUNC(i8272a_device::map));
	//map(0xf8,0xff) //
}

/* Input ports */
static INPUT_PORTS_START( idpartner )
INPUT_PORTS_END

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ "dma" },
	{ "sio1" },
	{ "sio2" },
	{ "pio" },
	{ "fdc_daisy" },
	{ nullptr }
};

static void partner_floppies(device_slot_interface &device)
{
	device.option_add("fdd", FLOPPY_525_QD);
}

static void partner_floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_IDPART_FORMAT);
}


static DEVICE_INPUT_DEFAULTS_START(keyboard)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_300)
DEVICE_INPUT_DEFAULTS_END


void partner_rs232_devices(device_slot_interface &device)
{
	default_rs232_devices(device);
	device.option_add("idpart_video",  IDPART_VIDEO);
}

/* Machine driver */
void idpartner_state::partner_base(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(8'000'000) / 2);
	m_maincpu->set_daisy_config(daisy_chain);
	m_maincpu->set_addrmap(AS_PROGRAM, &idpartner_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &idpartner_state::io_map);
	m_maincpu->busack_cb().set(m_dma, FUNC(z80dma_device::bai_w));

	Z80SIO(config, m_sio1, XTAL(8'000'000) / 2);
	m_sio1->out_txda_callback().set(m_serial[0], FUNC(rs232_port_device::write_txd));
	m_sio1->out_txdb_callback().set(m_serial[1], FUNC(rs232_port_device::write_txd));
	m_sio1->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	MC14411(config, m_brg, XTAL(1'843'200));
	m_brg->rsa_w(0);
	m_brg->rsb_w(1);
	m_brg->out_f<1>().set(FUNC(idpartner_state::write_speed2_clock));
	m_brg->out_f<1>().append(FUNC(idpartner_state::write_speed3_clock));
	m_brg->out_f<1>().append(FUNC(idpartner_state::write_speed4_clock));
	m_brg->out_f<13>().set(m_ctc, FUNC(z80ctc_device::trg0)); // signal XX1

	RS232_PORT(config, m_serial[0], partner_rs232_devices, nullptr);
	m_serial[0]->rxd_handler().set(m_sio1, FUNC(z80sio_device::rxa_w));

	RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);
	m_serial[1]->rxd_handler().set(m_sio1, FUNC(z80sio_device::rxb_w));

	Z80SIO(config, m_sio2, XTAL(8'000'000) / 2);
	m_sio2->out_txda_callback().set(m_serial[2], FUNC(rs232_port_device::write_txd));
	m_sio2->out_txdb_callback().set(m_serial[3], FUNC(rs232_port_device::write_txd));
	m_sio2->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	RS232_PORT(config, m_serial[2], default_rs232_devices, nullptr);
	m_serial[2]->rxd_handler().set(m_sio2, FUNC(z80sio_device::rxa_w));

	RS232_PORT(config, m_serial[3], default_rs232_devices, nullptr);
	m_serial[3]->rxd_handler().set(m_sio2, FUNC(z80sio_device::rxb_w));

	Z80CTC(config, m_ctc, XTAL(8'000'000) / 2);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->zc_callback<0>().set(m_ctc, FUNC(z80ctc_device::trg1));
	m_ctc->zc_callback<1>().set(FUNC(idpartner_state::xx2_w));
	m_ctc->zc_callback<2>().set(m_ctc, FUNC(z80ctc_device::trg3));  // optional

	Z80DMA(config, m_dma, XTAL(8'000'000) / 2);
	m_dma->out_busreq_callback().set_inputline(m_maincpu, Z80_INPUT_LINE_BUSRQ);
	m_dma->out_int_callback().set(m_fdc, FUNC(i8272a_device::tc_line_w));
	m_dma->in_mreq_callback().set(FUNC(idpartner_state::memory_read_byte));
	m_dma->out_mreq_callback().set(FUNC(idpartner_state::memory_write_byte));
	m_dma->in_iorq_callback().set(FUNC(idpartner_state::io_read_byte));
	m_dma->out_iorq_callback().set(FUNC(idpartner_state::io_write_byte));

	IDPARTNER_FLOPPY_DAISY(config, m_fdc_daisy);
	m_fdc_daisy->int_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	I8272A(config, m_fdc, XTAL(8'000'000), false);
	m_fdc->intrq_wr_callback().set(m_fdc_daisy, FUNC(idpartner_floppy_daisy_device::int_w));
	m_fdc->drq_wr_callback().set(m_dma, FUNC(z80dma_device::rdy_w));
	FLOPPY_CONNECTOR(config, m_floppy[0], partner_floppies, "fdd",   partner_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], partner_floppies, nullptr, partner_floppy_formats).enable_sound(true);

	MM58167(config, m_rtc, XTAL(32'768));

	Z80PIO(config, m_pio, XTAL(8'000'000) / 2);
	m_pio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	// There is one bus connector J2, but cable goes to up to two devices
	IDPARTNER_BUS(config, m_bus, 0);
	m_bus->set_io_space(m_maincpu, AS_IO);
	m_bus->int_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_bus->nmi_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_bus->drq_handler().set(m_dma, FUNC(z80dma_device::rdy_w));
	IDPARTNER_BUS_CONNECTOR(config, m_conn[0], m_bus, idpartner_exp_devices, nullptr);
	IDPARTNER_BUS_CONNECTOR(config, m_conn[1], m_bus, idpartner_exp_devices, nullptr);
}

void idpartner_state::partnerw(machine_config &config)
{
	partner_base(config);

	m_brg->out_f<1>().append(FUNC(idpartner_state::write_speed1_clock));

	m_serial[0]->set_default_option("idpart_video");

	m_conn[1]->set_default_option("sasi");
}

void idpartner_state::partner1fg(machine_config &config)
{
	partner_base(config);

	m_brg->out_f<9>().set(FUNC(idpartner_state::write_speed1_clock));

	m_serial[0]->set_default_option("keyboard");
	m_serial[0]->set_option_device_input_defaults("keyboard", DEVICE_INPUT_DEFAULTS_NAME(keyboard));

	m_conn[0]->set_default_option("gdp");
}

void idpartner_state::partnerwfg(machine_config &config)
{
	partner_base(config);

	m_brg->out_f<9>().set(FUNC(idpartner_state::write_speed1_clock));
	// TODO: This should be 13 from MC14411 but there are timing issues when
	// SIO is at lower speed ( < 1200 baud )
	m_brg->out_f<14>().set(m_ctc, FUNC(z80ctc_device::trg0)); // signal XX1
	m_brg->out_f<13>().set_nop();

	m_serial[0]->set_default_option("keyboard");
	m_serial[0]->set_option_device_input_defaults("keyboard", DEVICE_INPUT_DEFAULTS_NAME(keyboard));

	m_conn[0]->set_default_option("gdp");
	m_conn[1]->set_default_option("sasi");
}

/* ROM definition */

ROM_START( partnerw )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "partner.e51",     0x0000, 0x800, CRC(cabcf36e) SHA1(9c391bacb8d1a742cf74803c61cc061707ab23f4) )
	// e50 is empty
ROM_END

ROM_START( partner1fg )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "partner1fg.e51",  0x0000, 0x800, CRC(571e297a) SHA1(05379c75d6ceb338e49958576f3a1c844f202a00) )
	// e50 is empty
ROM_END

ROM_START( partnerwfg )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "partnerwfg.e51",  0x0000, 0x800, CRC(81a2a3db) SHA1(22b23969d38cf2b400be0042dbdd6f8cff2536be) )
	// e50 is empty
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME        PARENT      COMPAT  MACHINE        INPUT        CLASS            INIT        COMPANY         FULLNAME         FLAGS */
COMP( 1984, partnerw,   0,          0,      partnerw,      idpartner,   idpartner_state, empty_init, "Iskra Delta",  "Partner",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1987, partner1fg, partnerw,   0,      partner1fg,    idpartner,   idpartner_state, empty_init, "Iskra Delta",  "Partner 1F/G",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1987, partnerwfg, partnerw,   0,      partnerwfg,    idpartner,   idpartner_state, empty_init, "Iskra Delta",  "Partner WF/G",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
