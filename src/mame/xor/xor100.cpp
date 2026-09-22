// license:BSD-3-Clause
// copyright-holders:Curt Coder
/****************************************************************************************************

XOR S-100-12

XOR Data Science was apparently a 1982 reincorporation of a Huntington Beach-based
company previously known as Delta Products. At least some of the S-100 boards used
in XOR's systems were originally developed and documented under the former company
name.

*****************************************************************************************************

All input must be in upper case.
Summary of Monitor commands:

D xxxx yyyy           = dump memory to screen
F xxxx yyyy zz        = fill memory from xxxx to yyyy-1 with zz
G xxxx                = execute program at xxxx
H xxxx yyyy aa bb...  = Search memory for a string of bytes
L xxxx                = edit memory (. to exit)
M xxxx yyyy zzzz      = Move (copy) memory
R                     = Read cassette (not in all bios versions)
V xxxx                = Ascii dump of memory to the screen (cr to continue, space to exit)
W                     = Write cassette (not in all bios versions)
X n                   = Select a bank (0 works, others freeze)
^C                    = Boot from floppy

Note some of the commands are a bit buggy, eg F doesn't fill the last byte


TODO:
- Fix floppy. It needs to WAIT the cpu whenever port 0xFC is read, wait
  for either DRQ or INTRQ to assert, then release the cpu and then do the
  actual port read.
- The only available disks crash MAME when loaded.
- honor jumper settings
- CTC signal header
- serial printer
- cassette (no information, assumed to be on another card)

*****************************************************************************************************/


#include "emu.h"

#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "bus/s100/s100.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/com8116.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "machine/z80ctc.h"

namespace {

#define SCREEN_TAG      "screen"
#define Z80_TAG         "5b"
#define I8251_A_TAG     "12b"
#define I8251_B_TAG     "14b"
#define I8255A_TAG      "8a"
#define COM5016_TAG     "15c"
#define Z80CTC_TAG      "11b"
#define WD1795_TAG      "wd1795"
#define CENTRONICS_TAG  "centronics"
#define RS232_A_TAG     "rs232a"
#define RS232_B_TAG     "rs232b"
#define S100_TAG        "s100"

class xor100_state : public driver_device
{
public:
	xor100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, Z80_TAG)
		, m_uart_a(*this, I8251_A_TAG)
		, m_uart_b(*this, I8251_B_TAG)
		, m_fdc(*this, WD1795_TAG)
		, m_ctc(*this, Z80CTC_TAG)
		, m_ram(*this, RAM_TAG)
		, m_centronics(*this, CENTRONICS_TAG)
		, m_s100(*this, S100_TAG)
		, m_floppy(*this, WD1795_TAG":%u", 0U)
		, m_rom(*this, Z80_TAG)
		, m_bank1(*this, "bank1")
		, m_bank2(*this, "bank2")
		, m_bank3(*this, "bank3")
	{ }

	void xor100(machine_config &config);

private:
	void mmu_w(uint8_t data);
	void prom_toggle_w(uint8_t data);
	uint8_t prom_disable_r();
	uint8_t fdc_wait_r();
	void fdc_dcont_w(uint8_t data);
	void fdc_dsel_w(uint8_t data);
	void fdc_intrq_w(int state);
	void fdc_drq_w(int state);

	uint8_t i8255_pc_r();
	void ctc_z0_w(int state);
	void ctc_z1_w(int state);
	void ctc_z2_w(int state);
	void write_centronics_busy(int state);
	void write_centronics_select(int state);

	void xor100_io(address_map &map) ATTR_COLD;
	void xor100_mem(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void bankswitch();
	void post_load();

	required_device<cpu_device> m_maincpu;
	required_device<i8251_device> m_uart_a;
	required_device<i8251_device> m_uart_b;
	required_device<fd1795_device> m_fdc;
	required_device<z80ctc_device> m_ctc;
	required_device<ram_device> m_ram;
	required_device<centronics_device> m_centronics;
	required_device<s100_bus_device> m_s100;
	required_device_array<floppy_connector, 4> m_floppy;
	required_memory_region m_rom;
	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	required_memory_bank m_bank3;

	// memory state
	int m_mode = 0;
	int m_bank = 0;

	// floppy state
	bool m_fdc_irq = false;
	bool m_fdc_drq = false;
	int m_fdc_dden = 0;

	int m_centronics_busy = 0;
	int m_centronics_select = 0;
};

/* Read/Write Handlers */

enum
{
	EPROM_0000 = 0,
	EPROM_F800,
	EPROM_OFF
};

void xor100_state::bankswitch()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	int banks = m_ram->size() / 0x10000;

	switch (m_mode)
	{
	case EPROM_0000:
		if (m_bank < banks)
		{
			program.install_write_bank(0x0000, 0xffff, m_bank1);
			m_bank1->set_entry(1 + m_bank);
		}
		else
		{
			program.unmap_write(0x0000, 0xffff);
		}

		program.install_read_bank(0x0000, 0x07ff, 0xf000, m_bank2);
		program.install_read_bank(0xf800, 0xffff, m_bank3);
		m_bank2->set_entry(0);
		m_bank3->set_entry(0);
		break;

	case EPROM_F800:
		if (m_bank < banks)
		{
			program.install_write_bank(0x0000, 0xffff, m_bank1);
			program.install_read_bank(0x0000, 0xf7ff, m_bank2);
			m_bank1->set_entry(1 + m_bank);
			m_bank2->set_entry(1 + m_bank);
		}
		else
		{
			program.unmap_write(0x0000, 0xffff);
			program.unmap_read(0x0000, 0xf7ff);
		}

		program.install_read_bank(0xf800, 0xffff, m_bank3);
		m_bank3->set_entry(0);
		break;

	case EPROM_OFF:
		if (m_bank < banks)
		{
			program.install_write_bank(0x0000, 0xffff, m_bank1);
			program.install_read_bank(0x0000, 0xf7ff, m_bank2);
			program.install_read_bank(0xf800, 0xffff, m_bank3);
			m_bank1->set_entry(1 + m_bank);
			m_bank2->set_entry(1 + m_bank);
			m_bank3->set_entry(1 + m_bank);
		}
		else
		{
			program.unmap_write(0x0000, 0xffff);
			program.unmap_read(0x0000, 0xf7ff);
			program.unmap_read(0xf800, 0xffff);
		}
		break;
	}
}

void xor100_state::mmu_w(uint8_t data)
{
	/*

	    bit     description

	    0       A16
	    1       A17
	    2       A18
	    3       A19
	    4
	    5
	    6
	    7

	*/

	m_bank = data & 0x07;

	bankswitch();
}

void xor100_state::prom_toggle_w(uint8_t data)
{
	switch (m_mode)
	{
	case EPROM_OFF: m_mode = EPROM_F800; break;
	case EPROM_F800: m_mode = EPROM_OFF; break;
	}

	bankswitch();
}

uint8_t xor100_state::prom_disable_r()
{
	m_mode = EPROM_F800;

	bankswitch();

	return 0xff;
}

uint8_t xor100_state::fdc_wait_r()
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5
	    6
	    7       FDC IRQ

	*/

	if (!machine().side_effects_disabled())
	{
		if (!m_fdc_irq && !m_fdc_drq)
		{
			//m_maincpu->set_input_line(Z80_INPUT_LINE_WAIT, ASSERT_LINE);
		}
	}

	return m_fdc_irq ? 0x7f : 0xff;
}

void xor100_state::fdc_dcont_w(uint8_t data)
{
	/*

	    bit     description

	    0       DS0
	    1       DS1
	    2       DS2
	    3       DS3
	    4
	    5
	    6
	    7       _HLSTB

	*/

	// drive select
	floppy_image_device *floppy = nullptr;

	for (int n = 0; n < 4; n++)
		if (BIT(data, n))
			floppy = m_floppy[n]->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy) floppy->mon_w(0);
}

void xor100_state::fdc_dsel_w(uint8_t data)
{
	/*

	    bit     description

	    0       J
	    1       K
	    2
	    3
	    4
	    5
	    6
	    7

	*/

	switch (data & 0x03)
	{
	case 0: break;
	case 1: m_fdc_dden = 1; break;
	case 2: m_fdc_dden = 0; break;
	case 3: m_fdc_dden = !m_fdc_dden; break;
	}

	m_fdc->dden_w(m_fdc_dden);
}

/* Memory Maps */

void xor100_state::xor100_mem(address_map &map)
{
	map(0x0000, 0xffff).bankw("bank1");
	map(0x0000, 0xf7ff).bankr("bank2");
	map(0xf800, 0xffff).bankr("bank3");
}

void xor100_state::xor100_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw(m_uart_a, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x02, 0x03).rw(m_uart_b, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x04, 0x07).rw(I8255A_TAG, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x08, 0x08).w(FUNC(xor100_state::mmu_w));
	map(0x09, 0x09).w(FUNC(xor100_state::prom_toggle_w));
	map(0x0a, 0x0a).r(FUNC(xor100_state::prom_disable_r));
	map(0x0b, 0x0b).portr("DSW0").w(COM5016_TAG, FUNC(com8116_device::stt_str_w));
	map(0x0c, 0x0f).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0xf8, 0xfb).lrw8(
					NAME([this](offs_t offset) { return m_fdc->read(offset) ^ 0xff; }),
					NAME([this](offs_t offset, u8 data) { m_fdc->write(offset, data ^ 0xff); }));
	map(0xfc, 0xfc).rw(FUNC(xor100_state::fdc_wait_r), FUNC(xor100_state::fdc_dcont_w));
	map(0xfd, 0xfd).w(FUNC(xor100_state::fdc_dsel_w));
}

/* Input Ports */

static INPUT_PORTS_START( xor100 )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x0f, 0x05, "Serial Port A" )
	PORT_DIPSETTING(    0x00, "50 baud" )
	PORT_DIPSETTING(    0x01, "75 baud" )
	PORT_DIPSETTING(    0x02, "110 baud" )
	PORT_DIPSETTING(    0x03, "134.5 baud" )
	PORT_DIPSETTING(    0x04, "150 baud" )
	PORT_DIPSETTING(    0x05, "300 baud" )
	PORT_DIPSETTING(    0x06, "600 baud" )
	PORT_DIPSETTING(    0x07, "1200 baud" )
	PORT_DIPSETTING(    0x08, "1800 baud" )
	PORT_DIPSETTING(    0x09, "2000 baud" )
	PORT_DIPSETTING(    0x0a, "2400 baud" )
	PORT_DIPSETTING(    0x0b, "3600 baud" )
	PORT_DIPSETTING(    0x0c, "4800 baud" )
	PORT_DIPSETTING(    0x0d, "7200 baud" )
	PORT_DIPSETTING(    0x0e, "9600 baud" )
	PORT_DIPSETTING(    0x0f, "19200 baud" )
	PORT_DIPNAME( 0xf0, 0xe0, "Serial Port B" )
	PORT_DIPSETTING(    0x00, "50 baud" )
	PORT_DIPSETTING(    0x10, "75 baud" )
	PORT_DIPSETTING(    0x20, "110 baud" )
	PORT_DIPSETTING(    0x30, "134.5 baud" )
	PORT_DIPSETTING(    0x40, "150 baud" )
	PORT_DIPSETTING(    0x50, "300 baud" )
	PORT_DIPSETTING(    0x60, "600 baud" )
	PORT_DIPSETTING(    0x70, "1200 baud" )
	PORT_DIPSETTING(    0x80, "1800 baud" )
	PORT_DIPSETTING(    0x90, "2000 baud" )
	PORT_DIPSETTING(    0xa0, "2400 baud" )
	PORT_DIPSETTING(    0xb0, "3600 baud" )
	PORT_DIPSETTING(    0xc0, "4800 baud" )
	PORT_DIPSETTING(    0xd0, "7200 baud" )
	PORT_DIPSETTING(    0xe0, "9600 baud" )
	PORT_DIPSETTING(    0xf0, "19200 baud" )

	PORT_START("J1")
	PORT_CONFNAME( 0x01, 0x01, "J1 Wait State")
	PORT_CONFSETTING( 0x00, "No Wait States" )
	PORT_CONFSETTING( 0x01, "1 M1 Wait State" )

	PORT_START("J2")
	PORT_CONFNAME( 0x01, 0x01, "J2 CPU Speed")
	PORT_CONFSETTING( 0x00, "2 MHz" )
	PORT_CONFSETTING( 0x01, "4 MHz" )

	PORT_START("J3")
	PORT_CONFNAME( 0x01, 0x00, "J3")
	PORT_CONFSETTING( 0x00, "" )
	PORT_CONFSETTING( 0x01, "" )

	PORT_START("J4-J5")
	PORT_CONFNAME( 0x01, 0x01, "J4/J5 EPROM Type")
	PORT_CONFSETTING( 0x00, "2708" )
	PORT_CONFSETTING( 0x01, "2716" )

	PORT_START("J6")
	PORT_CONFNAME( 0x01, 0x01, "J6 EPROM")
	PORT_CONFSETTING( 0x00, "Disabled" )
	PORT_CONFSETTING( 0x01, "Enabled" )

	PORT_START("J7")
	PORT_CONFNAME( 0x01, 0x00, "J7 I/O Port Addresses")
	PORT_CONFSETTING( 0x00, "00-0F" )
	PORT_CONFSETTING( 0x01, "10-1F" )

	PORT_START("J8")
	PORT_CONFNAME( 0x01, 0x00, "J8")
	PORT_CONFSETTING( 0x00, "" )
	PORT_CONFSETTING( 0x01, "" )

	PORT_START("J9")
	PORT_CONFNAME( 0x01, 0x01, "J9 Power On RAM")
	PORT_CONFSETTING( 0x00, "Enabled" )
	PORT_CONFSETTING( 0x01, "Disabled" )
INPUT_PORTS_END

/* Printer 8255A Interface */

void xor100_state::write_centronics_busy(int state)
{
	m_centronics_busy = state;
}

void xor100_state::write_centronics_select(int state)
{
	m_centronics_select = state;
}

uint8_t xor100_state::i8255_pc_r()
{
	/*

	    bit     description

	    PC0
	    PC1
	    PC2
	    PC3
	    PC4     ON LINE
	    PC5     BUSY
	    PC6     _ACK
	    PC7

	*/

	uint8_t data = 0;

	/* on line */
	data |= m_centronics_select << 4;

	/* busy */
	data |= m_centronics_busy << 5;

	return data;
}

/* Z80-CTC Interface */

void xor100_state::ctc_z0_w(int state)
{
}

void xor100_state::ctc_z1_w(int state)
{
}

void xor100_state::ctc_z2_w(int state)
{
}

/* WD1795-02 Interface */

static void xor100_floppies(device_slot_interface &device)
{
	device.option_add("8ssdd", FLOPPY_8_SSDD); // Shugart SA-100
}

void xor100_state::fdc_intrq_w(int state)
{
	m_fdc_irq = state;
	m_ctc->trg0(state);

	if (state)
		m_maincpu->set_input_line(Z80_INPUT_LINE_WAIT, CLEAR_LINE);
}

void xor100_state::fdc_drq_w(int state)
{
	m_fdc_drq = state;

	if (state)
		m_maincpu->set_input_line(Z80_INPUT_LINE_WAIT, CLEAR_LINE);
}


static void xor100_s100_cards(device_slot_interface &device)
{
}

/* Machine Initialization */

void xor100_state::machine_start()
{
	int banks = m_ram->size() / 0x10000;
	uint8_t *ram = m_ram->pointer();
	uint8_t *rom = m_rom->base();
	m_bank = 0;

	/* setup memory banking */
	membank("bank1")->configure_entries(1, banks, ram, 0x10000);
	membank("bank2")->configure_entry(0, rom);
	membank("bank2")->configure_entries(1, banks, ram, 0x10000);
	membank("bank3")->configure_entry(0, rom);
	membank("bank3")->configure_entries(1, banks, ram + 0xf800, 0x10000);

	machine().save().register_postload(save_prepost_delegate(FUNC(xor100_state::post_load), this));

	/* register for state saving */
	save_item(NAME(m_mode));
	save_item(NAME(m_bank));
	save_item(NAME(m_fdc_irq));
	save_item(NAME(m_fdc_drq));
	save_item(NAME(m_fdc_dden));
	save_item(NAME(m_centronics_busy));
	save_item(NAME(m_centronics_select));
}

void xor100_state::machine_reset()
{
	m_mode = EPROM_0000;

	bankswitch();
}

void xor100_state::post_load()
{
	bankswitch();
}

/* Machine Driver */

void xor100_state::xor100(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 8_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &xor100_state::xor100_mem);
	m_maincpu->set_addrmap(AS_IO, &xor100_state::xor100_io);

	/* devices */
	I8251(config, m_uart_a, 0/*8_MHz_XTAL / 2,*/);
	m_uart_a->txd_handler().set(RS232_A_TAG, FUNC(rs232_port_device::write_txd));
	m_uart_a->dtr_handler().set(RS232_A_TAG, FUNC(rs232_port_device::write_dtr));
	m_uart_a->rts_handler().set(RS232_A_TAG, FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232a(RS232_PORT(config, RS232_A_TAG, default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_uart_a, FUNC(i8251_device::write_rxd));
	rs232a.dsr_handler().set(m_uart_a, FUNC(i8251_device::write_dsr));

	I8251(config, m_uart_b, 0/*8_MHz_XTAL / 2,*/);
	m_uart_b->txd_handler().set(RS232_B_TAG, FUNC(rs232_port_device::write_txd));
	m_uart_b->dtr_handler().set(RS232_B_TAG, FUNC(rs232_port_device::write_dtr));
	m_uart_b->rts_handler().set(RS232_B_TAG, FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232b(RS232_PORT(config, RS232_B_TAG, default_rs232_devices, "terminal"));
	rs232b.rxd_handler().set(m_uart_b, FUNC(i8251_device::write_rxd));
	rs232b.dsr_handler().set(m_uart_b, FUNC(i8251_device::write_dsr));
	rs232b.cts_handler().set(m_uart_b, FUNC(i8251_device::write_cts));

	com8116_device &brg(COM8116(config, COM5016_TAG, 5.0688_MHz_XTAL));
	brg.fr_handler().set(m_uart_a, FUNC(i8251_device::write_txc));
	brg.fr_handler().append(m_uart_a, FUNC(i8251_device::write_rxc));
	brg.ft_handler().set(m_uart_b, FUNC(i8251_device::write_txc));
	brg.ft_handler().append(m_uart_b, FUNC(i8251_device::write_rxc));

	i8255_device &ppi(I8255A(config, I8255A_TAG));
	ppi.out_pa_callback().set("cent_data_out", FUNC(output_latch_device::write));
	ppi.out_pb_callback().set(m_centronics, FUNC(centronics_device::write_strobe));
	ppi.in_pc_callback().set(FUNC(xor100_state::i8255_pc_r));

	Z80CTC(config, m_ctc, 8_MHz_XTAL / 2);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->zc_callback<0>().set(FUNC(xor100_state::ctc_z0_w));
	m_ctc->zc_callback<1>().set(FUNC(xor100_state::ctc_z1_w));
	m_ctc->zc_callback<2>().set(FUNC(xor100_state::ctc_z2_w));

	FD1795(config, m_fdc, 8_MHz_XTAL / 4);
	m_fdc->intrq_wr_callback().set(FUNC(xor100_state::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(xor100_state::fdc_drq_w));

	FLOPPY_CONNECTOR(config, m_floppy[0], xor100_floppies, "8ssdd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[1], xor100_floppies, "8ssdd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[2], xor100_floppies, nullptr,    floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[3], xor100_floppies, nullptr,    floppy_image_device::default_mfm_floppy_formats);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(I8255A_TAG, FUNC(i8255_device::pc4_w));
	m_centronics->busy_handler().set(FUNC(xor100_state::write_centronics_busy));
	m_centronics->select_handler().set(FUNC(xor100_state::write_centronics_select));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	// S-100
	S100_BUS(config, m_s100, 8_MHz_XTAL / 4);
	//m_s100->rdy().set_inputline(m_maincpu, Z80_INPUT_LINE_WAIT);
	S100_SLOT(config, S100_TAG ":1", xor100_s100_cards, nullptr);
	S100_SLOT(config, S100_TAG ":2", xor100_s100_cards, nullptr);
	S100_SLOT(config, S100_TAG ":3", xor100_s100_cards, nullptr);
	S100_SLOT(config, S100_TAG ":4", xor100_s100_cards, nullptr);
	S100_SLOT(config, S100_TAG ":5", xor100_s100_cards, nullptr);
	S100_SLOT(config, S100_TAG ":6", xor100_s100_cards, nullptr);
	S100_SLOT(config, S100_TAG ":7", xor100_s100_cards, nullptr);
	S100_SLOT(config, S100_TAG ":8", xor100_s100_cards, nullptr);
	S100_SLOT(config, S100_TAG ":9", xor100_s100_cards, nullptr);
	S100_SLOT(config, S100_TAG ":10", xor100_s100_cards, nullptr);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("64K").set_extra_options("128K,192K,256K,320K,384K,448K,512K");
}

/* ROMs */

ROM_START( xor100 )
	ROM_REGION( 0x800, Z80_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "v185", "v1.85" )
	ROMX_LOAD( "xp 185.8b", 0x000, 0x800, CRC(0d0bda8d) SHA1(11c83f7cd7e6a570641b44a2f2cc5737a7dd8ae3), ROM_BIOS(0) )
ROM_END

} // anonymous namespace

/* System Drivers */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   STATE         INIT        COMPANY             FULLNAME        FLAGS
COMP( 1980, xor100, 0,      0,      xor100,  xor100, xor100_state, empty_init, "XOR Data Science", "XOR S-100-12", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
