// license:BSD-3-Clause
// copyright-holders:Curt Coder
#include "emu.h"

#include "bus/centronics/ctronics.h"
#include "bus/ecbbus/ecbbus.h"
#include "cpu/z180/z180.h"
#include "imagedev/floppy.h"
#include "bus/rs232/rs232.h"
#include "machine/74259.h"
#include "machine/mk3835.h"
#include "machine/pcf8583.h"
#include "machine/ram.h"
#include "machine/rescap.h"
#include "machine/upd765.h"
#include "screen.h"
#include "softlist_dev.h"

#define HD64180_TAG             "hd64180"
#define FDC9268_TAG             "fdc9268"
#define FDC9229_TAG             "fdc9229"
#define MK3835_TAG              "mk3835"
#define SCREEN_TAG              "screen"
#define CENTRONICS_TAG          "centronics"

namespace {

class prof180x_state : public driver_device
{
public:
	prof180x_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, HD64180_TAG),
		m_fdc(*this, FDC9268_TAG),
		m_floppy(*this, FDC9268_TAG":%u", 0U),
		m_mk3835(*this, MK3835_TAG),
		m_pcf8583(*this, "pcf8583"),
		m_centronics(*this, CENTRONICS_TAG),
		m_ecb(*this, "bus"),
		m_ram(*this, RAM_TAG),
		m_config(*this, "CONFIG")
	{
	}

	void prof180x(machine_config &config);
	void prof180xp(machine_config &config);

private:
	required_device<z180_device> m_maincpu;
	required_device<fdc9266_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppy;
	optional_device<mk3835_device> m_mk3835;
	optional_device<pcf8583_device> m_pcf8583;
	required_device<centronics_device> m_centronics;
	required_device<ecbbus_device> m_ecb;
	required_device<ram_device> m_ram;
	required_ioport m_config;

	virtual void machine_start() override ATTR_COLD;

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	uint8_t status0_r();
	uint8_t status1_r();
	uint8_t status_r(offs_t offset);

	void motor(int state);
	void index_w(int state) { m_index = state; };
	uint8_t fdc_r(offs_t offset);
	void fdc_w(offs_t offset, uint8_t data);
	TIMER_CALLBACK_MEMBER(motor_off_tick) { motor(1); }

	void c0_flag_w(int state);
	void c1_flag_w(int state);
	void c2_flag_w(int state) { m_c2 = state; };
	void mini_flag_w(int state) { };
	void peps_flag_w(int state) { };
	void mm0_flag_w(int state) { m_mm0 = state; };
	void mm1_flag_w(int state) { m_mm1 = state; };

	void prof180x_base(machine_config &config);

	int m_c0 = 0;
	int m_c1 = 0;
	int m_c2 = 0;
	int m_mm0 = 0;
	int m_mm1 = 0;
	int m_motor = 1;
	int m_index = 1;

	emu_timer *m_motor_off_timer = nullptr;

	void prof180x_io(address_map &map) ATTR_COLD;
	void prof180x_mem(address_map &map) ATTR_COLD;
};

uint8_t prof180x_state::read(offs_t offset)
{
	if (offset < 0x4000)
	{
		switch ((m_mm0 << 1) | m_mm1)
		{
		case 0:
			return memregion(HD64180_TAG)->base()[offset];

		case 1:
		case 3:
			return m_ram->read(offset);

		case 2:
			return 0xff;
		}
	}
	else if (offset < 0x8000)
	{
		if (((m_mm0 << 1) | m_mm1) != 3)
			return m_ram->read(offset);
		return 0xff;
	}
	else if (m_ram->size() == 0x20000)
	{
		return m_ram->read((offset & 0xffff) | ((offset >> 2) & 0x10000));
	}
	else if (offset < m_ram->size())
	{
		return m_ram->read(offset);
	}
	else if (offset >= 0x40000 && offset < (0x40000 + m_ram->size()))
	{
		return m_ram->read(offset - 0x40000);
	}

	return 0xff;
}

void prof180x_state::write(offs_t offset, uint8_t data)
{
	if (offset < 0x4000)
	{
		if (((m_mm0 << 1) | m_mm1) != 2)
			m_ram->write(offset, data);
	}
	else if (offset < 0x8000)
	{
		if (((m_mm0 << 1) | m_mm1) != 3)
			m_ram->write(offset, data);
	}
	else if (m_ram->size() == 0x20000)
	{
		m_ram->write((offset & 0xffff) | ((offset >> 2) & 0x10000), data);
	}
	else if (offset < m_ram->size())
	{
		m_ram->write(offset, data);
	}
	else if (offset >= 0x40000 && offset < (0x40000 + m_ram->size()))
	{
		m_ram->write(offset - 0x40000, data);
	}
}

void prof180x_state::c0_flag_w(int state)
{
	// C0 (DATA)
	m_c0 = state;
	if (m_mk3835)
		m_mk3835->io_w(state);
	if (m_pcf8583)
		m_pcf8583->sda_w(state);
}

void prof180x_state::c1_flag_w(int state)
{
	// C1 (M0)
	m_c1 = state;
	if (m_mk3835)
		m_mk3835->sclk_w(state);
}

uint8_t prof180x_state::status0_r()
{
	/*

	    bit     description

	    0       BUSY
	    1
	    2
	    3
	    4       B-E
	    5       IDX
	    6
	    7       MOT

	*/

	u8 data = 0;

	data |= m_index << 5;
	data |= m_motor << 7;

	return data;
}

uint8_t prof180x_state::status1_r()
{
	/*

	    bit     description

	    0       FREE
	    1
	    2
	    3
	    4       J18
	    5       J19
	    6
	    7       TDO

	*/

	u8 data = 0;

	const u8 scan = (m_c0 ? 0x01 : 0x00) | (m_c1 ? 0x02 : 0x00)	| (m_c2 ? 0x04 : 0x00);
	const u8 config = m_config->read();

	if ((config & 0x07) & scan)
		data |= 0x10;

	if (((config >> 3) & 0x07) & scan)
		data |= 0x20;

	if (!(m_mk3835 ? m_mk3835->io_r() : m_pcf8583->sda_r()))
		data |= 0x80;

	return data;
}

uint8_t prof180x_state::status_r(offs_t offset)
{
	return BIT(offset, 8) ? status1_r() : status0_r();
}

void prof180x_state::motor(int state)
{
	for (auto &connector : m_floppy)
	{
		floppy_image_device *drive = connector->get_device();

		if (drive)
		{
			drive->mon_w(state);
		}
	}

	if (!state)
	{
		double const t = 0.45 * RES_K(220) * CAP_U(6.8); // 74HC123 monostable
		m_motor_off_timer->adjust(attotime::from_double(t));
	}

	m_motor = state;
}

uint8_t prof180x_state::fdc_r(offs_t offset)
{
	motor(0);

	if (offset)
		return m_fdc->fifo_r();
	else
		return m_fdc->msr_r();
}

void prof180x_state::fdc_w(offs_t offset, uint8_t data)
{
	motor(0);

	if (offset)
		m_fdc->fifo_w(data);
}

void prof180x_state::prof180x_mem(address_map &map)
{
	map(0x00000, 0x7ffff).rw(FUNC(prof180x_state::read), FUNC(prof180x_state::write));
}

void prof180x_state::prof180x_io(address_map &map)
{
	map(0x0000, 0x00d7).mirror(0xff00).rw(m_ecb, FUNC(ecbbus_device::io_r), FUNC(ecbbus_device::io_w));
	map(0x0000, 0x003f).noprw(); // Z180 internal registers
	map(0x00d8, 0x00d8).mirror(0xff00).w("syslatch", FUNC(ls259_device::write_nibble_d0));
	map(0x00d9, 0x00d9).select(0xff00).r(FUNC(prof180x_state::status_r));
	map(0x00da, 0x00da).mirror(0xff00).rw(m_fdc, FUNC(fdc9266_device::dma_r), FUNC(fdc9266_device::dma_w));
	map(0x00db, 0x00db).mirror(0xff00).w("cent_data_out", FUNC(output_latch_device::write));
	map(0x00dc, 0x00dd).mirror(0xff00).rw(FUNC(prof180x_state::fdc_r), FUNC(prof180x_state::fdc_w));
}

static INPUT_PORTS_START( prof180x )
	PORT_START("CONFIG")
	PORT_CONFNAME(0x07, 0x00, "J18 Console")
	PORT_CONFSETTING(0x00, "No jumper (GRIP)")
	PORT_CONFSETTING(0x07, "1-3 (Terminal)")
	PORT_CONFSETTING(0x06, "2-4 (GRADE-X)")
	PORT_CONFSETTING(0x05, "3-5 (USER)")
	PORT_CONFSETTING(0x03, "4-6 (Auto-Boot)")
	PORT_CONFNAME(0x38, 0x38, "J19 Baud Rate")
	PORT_CONFSETTING(0x00, "No jumper (19200 baud)")
	PORT_CONFSETTING(0x38, "1-3 (9600 baud)")
	PORT_CONFSETTING(0x30, "2-4 (2400 baud)")
	PORT_CONFSETTING(0x28, "3-5 (1200 baud)")
	PORT_CONFSETTING(0x18, "4-6 (300 baud)")
INPUT_PORTS_END

static void prof180x_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY", 0xff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_1)
DEVICE_INPUT_DEFAULTS_END

void prof180x_state::machine_start()
{
	m_motor_off_timer = timer_alloc(FUNC(prof180x_state::motor_off_tick), this);

	// state saving
	save_item(NAME(m_c0));
	save_item(NAME(m_c1));
	save_item(NAME(m_c2));
	save_item(NAME(m_mm0));
	save_item(NAME(m_mm1));
	save_item(NAME(m_motor));
}

void prof180x_state::prof180x_base(machine_config &config)
{
	HD64180RP(config, m_maincpu, XTAL(18'432'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &prof180x_state::prof180x_mem);
	m_maincpu->set_addrmap(AS_IO, &prof180x_state::prof180x_io);
	m_maincpu->tend1_wr_callback().set(m_fdc, FUNC(fdc9266_device::tc_line_w));
	m_maincpu->txa0_wr_callback().set("rs232a", FUNC(rs232_port_device::write_txd));
	m_maincpu->rts0_wr_callback().set("rs232a", FUNC(rs232_port_device::write_rts));
	m_maincpu->txa1_wr_callback().set("rs232b", FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, nullptr));
	rs232a.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));
	rs232a.rxd_handler().set(m_maincpu, FUNC(z180_device::rxa0_w));
	rs232a.dcd_handler().set(m_maincpu, FUNC(z180_device::dcd0_w));
	rs232a.cts_handler().set(m_maincpu, FUNC(z180_device::cts0_w));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_maincpu, FUNC(z180_device::rxa1_w));
	rs232b.cts_handler().set(m_maincpu, FUNC(z180_device::rxs_cts1_w));

	ls259_device &syslatch(LS259(config, "syslatch")); // Z41
	syslatch.q_out_cb<0>().set(FUNC(prof180x_state::c0_flag_w));
	syslatch.q_out_cb<1>().set(FUNC(prof180x_state::c1_flag_w));
	syslatch.q_out_cb<2>().set(FUNC(prof180x_state::c2_flag_w));
	syslatch.q_out_cb<3>().set(FUNC(prof180x_state::mini_flag_w));
	syslatch.q_out_cb<4>().set(FUNC(prof180x_state::mm0_flag_w));
	syslatch.q_out_cb<5>().set([this] (int state) {
		if (m_mk3835)
			m_mk3835->ce_w(!state);
		if (m_pcf8583)
			m_pcf8583->scl_w(!state);
	});
	syslatch.q_out_cb<6>().set(FUNC(prof180x_state::peps_flag_w));
	syslatch.q_out_cb<7>().set(FUNC(prof180x_state::mm1_flag_w));

	FDC9266(config, m_fdc, XTAL(8'000'000), true, true);
	m_fdc->drq_wr_callback().set_inputline(m_maincpu, Z180_INPUT_LINE_DREQ1);
	m_fdc->idx_wr_callback().set(FUNC(prof180x_state::index_w));

	FLOPPY_CONNECTOR(config, FDC9268_TAG ":0", prof180x_floppies, "35dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, FDC9268_TAG ":1", prof180x_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, FDC9268_TAG ":2", prof180x_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, FDC9268_TAG ":3", prof180x_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");

	output_latch_device &latch(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(latch);

	ECBBUS(config, m_ecb);
	ECBBUS_SLOT(config, "bus1", m_ecb, 1, ecbbus_cards, "grip21");
	ECBBUS_SLOT(config, "bus2", m_ecb, 2, ecbbus_cards, nullptr);
	ECBBUS_SLOT(config, "bus3", m_ecb, 3, ecbbus_cards, nullptr);
	ECBBUS_SLOT(config, "bus4", m_ecb, 4, ecbbus_cards, nullptr);
	ECBBUS_SLOT(config, "bus5", m_ecb, 5, ecbbus_cards, nullptr);

	RAM(config, RAM_TAG).set_default_size("128K").set_extra_options("512K");

	SOFTWARE_LIST(config, "flop_list").set_original("prof180x_flop");
}

void prof180x_state::prof180x(machine_config &config)
{
	prof180x_base(config);

	MK3835(config, m_mk3835);
}

void prof180x_state::prof180xp(machine_config &config)
{
	prof180x_base(config);

	PCF8583(config, m_pcf8583, XTAL(32'768)).set_a0(1);
}

ROM_START( prof180x )
	ROM_REGION( 0x10000, HD64180_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "pmon13", "pmon v1.3" )
	ROMX_LOAD( "pmon1_3.z16", 0x00000, 0x04000, CRC(32986688) SHA1(a6229d7e66ef699722ca3d41179fe3f1b75185d4), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "pmon14", "pmon v1.4" )
	ROMX_LOAD( "pmon1_4.z16", 0x00000, 0x04000, CRC(ed03f49f) SHA1(e016f9e0b89ab64c6203e2e46501d0b09f74ee9b), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "eboot1", "eboot1" )
	ROMX_LOAD( "eboot1.z16", 0x00000, 0x08000, CRC(7a164b3c) SHA1(69367804b5cbc0633e3d7bbbcc256c2c8c9e7aca), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "epmon1", "epmon1" )
	ROMX_LOAD( "epmon1.z16", 0x00000, 0x10000, CRC(27aabfb4) SHA1(41adf038c474596dbf7d387a1a7f33ed86aa7869), ROM_BIOS(3) )

	//ROM_REGION( 0x157, "plds", 0 )
	//ROM_LOAD( "pal14l8.z10", 0x000, 0x157, NO_DUMP )

	ROM_REGION( 0x20, MK3835_TAG, 0 )
	ROM_LOAD( "mk3835.u40", 0x00, 0x20, CRC(65202043) SHA1(5f1e688efd9056d5b73d434a3610e9e6e95fc3b0) )
ROM_END

ROM_START( prof180xp )
	ROM_REGION( 0x10000, HD64180_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "pmon15", "pmon v1.5" )
	ROMX_LOAD( "pmon1_5.u13", 0x00000, 0x04000, CRC(f43d185c) SHA1(a7a219b3d48c74602b3116cfcd34e44d6e7bc423), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "pmon", "pmon" )
	ROMX_LOAD( "pmon.u13",    0x00000, 0x04000, CRC(4f3732d7) SHA1(7dc27262db4e0c8f109470253b9364a216909f2c), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "eboot2", "eboot2" )
	ROMX_LOAD( "eboot2.u13",  0x00000, 0x08000, CRC(0c2d4301) SHA1(f1a4f457e287b19e14d8ccdbc0383f183d8a3efe), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "epmon2", "epmon2" )
	ROMX_LOAD( "epmon2.u13",  0x00000, 0x10000, CRC(3b8a7b59) SHA1(33741f0725e3eaa21c6881c712579b3c1fd30607), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "epmon3", "epmon3" )
	ROMX_LOAD( "epmon3.u13",  0x00000, 0x10000, CRC(51313af1) SHA1(60c293171a1c7cb9a5ff6d681e61894f44fddbd1), ROM_BIOS(4) )

	//ROM_REGION( 0x157, "plds", 0 )
	//ROM_LOAD( "pal14l8.z10", 0x000, 0x157, NO_DUMP )

	ROM_REGION( 0x100, "pcf8583", 0 )
	ROM_LOAD( "pcf8583.u41", 0x000, 0x100, CRC(f0ae5d44) SHA1(d8f579a9bc20e62300c347f8dcc540181827296f) )
ROM_END

} // anonymous namespace

/*    YEAR  NAME       PARENT    COMPAT  MACHINE    INPUT     CLASS           INIT        COMPANY                 FULLNAME               FLAGS */
COMP( 1986, prof180x,  0,        0,      prof180x,  prof180x, prof180x_state, empty_init, "Conitec Datensysteme", "PROF-180X",           MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1986, prof180xp, prof180x, 0,      prof180xp, prof180x, prof180x_state, empty_init, "Conitec Datensysteme", "PROF-180X (PCF8583)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
