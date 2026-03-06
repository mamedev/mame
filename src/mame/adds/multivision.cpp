// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    ADDS Multivision

    Desktop computer system. A basic system is called "Multivision 1". This
    can be enhanced to a "Multivision 2" system by adding a hard disk
    module. It can further enhanced to a "Multivision 3" multi-user system
    with an expansion module.

    Hardware:
    - P8085A-2
    - P8259A
    - P8253-5
    - P8237-5
    - P8251A x2
    - 20 MHz XTAL
    - 256 bytes NVRAM (battery backup up)
    - 64k RAM with 4-position dip switch (on RAM board)
    - FD1793B-02 with 20 MHz XTAL (on floppy board)

    TODO:
    - Test when software shows up
    - Centronics port/Letter quality port
    - RAM Parity?
    - Emulate "Multivision 2" and "Multivision 3" variants

    Notes:
    - No software available
    - Switches to 9600 baud once CMOS has been initialized

***************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "imagedev/floppy.h"
#include "machine/am9517a.h"
#include "machine/i8251.h"
#include "machine/nvram.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/wd_fdc.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class multivision_state : public driver_device
{
public:
	multivision_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_nvram(*this, "nvram"),
		m_pit(*this, "pit"),
		m_pic(*this, "pic"),
		m_dma(*this, "dma"),
		m_usart(*this, "usart%u", 0U),
		m_console(*this, "console"),
		m_communication(*this, "communication"),
		m_fdc(*this, "fdc"),
		m_floppy(*this, "fdc:%u", 0U),
		m_rambank(*this, "rambank%u", 0U),
		m_ramview{ { *this, "bank0"}, { *this, "bank1" }, { *this, "bank2" }, { *this, "bank3" } }
	{}

	void multivision(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	enum
	{
		MAP_RAM = 0,
		MAP_UNMAPPED = 1,
		MAP_BOOTSTRAP = 2
	};

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	void floppy_select_w(uint8_t data);

	void ram_select_w(uint8_t data);
	uint8_t ram_parity_r();
	void ram_program_w(uint8_t data);

	void serial_control_w(uint8_t data);
	uint8_t serial_status_r();
	void console_txd_w(int state);
	void console_cts_w(int state);
	void communication_txd_w(int state);
	void communication_cts_w(int state);

	void cmos_addr_w(uint8_t data);
	uint8_t cmos_data_r();
	void cmos_data_w(uint8_t data);
	void rtc_reset_w(uint8_t data);
	TIMER_CALLBACK_MEMBER(ac_line_frequency_tick);
	void cmos_control_w(uint8_t data);
	void eprom_control_w(uint8_t data);

	void dma_hrq_w(int state);
	uint8_t dma_mem_r(offs_t offset);
	void dma_mem_w(offs_t offset, uint8_t data);

	required_device<i8085a_cpu_device> m_maincpu;
	required_device<nvram_device> m_nvram;
	required_device<pit8253_device> m_pit;
	required_device<pic8259_device> m_pic;
	required_device<am9517a_device> m_dma;
	required_device_array<i8251_device, 2> m_usart;
	required_device<rs232_port_device> m_console;
	required_device<rs232_port_device> m_communication;
	required_device<fd1793_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_memory_bank_array<4> m_rambank;
	memory_view m_ramview[4];

	std::unique_ptr<uint8_t[]> m_ram;
	std::unique_ptr<uint8_t[]> m_cmos;

	emu_timer *m_ac_line_frequency_timer;

	uint8_t m_ram_select;
	bool m_rambank_enable[4];
	uint8_t m_serial_control;
	uint8_t m_cmos_addr_latch;
	bool m_cmos_enable;
	uint8_t m_rtc_counter;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void multivision_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).view(m_ramview[0]);
	m_ramview[0][0](0x0000, 0x3fff).bankrw(m_rambank[0]);
	m_ramview[0][1](0x0000, 0x3fff).unmaprw();
	m_ramview[0][2](0x0000, 0x0fff).rom().region("bootstrap", 0);
	map(0x4000, 0x7fff).view(m_ramview[1]);
	m_ramview[1][0](0x4000, 0x7fff).bankrw(m_rambank[1]);
	m_ramview[1][1](0x4000, 0x7fff).unmaprw();
	map(0x8000, 0xbfff).view(m_ramview[2]);
	m_ramview[2][0](0x8000, 0xbfff).bankrw(m_rambank[2]);
	m_ramview[2][1](0x8000, 0xbfff).unmaprw();
	map(0xc000, 0xffff).view(m_ramview[3]);
	m_ramview[3][0](0xc000, 0xffff).bankrw(m_rambank[3]);
	m_ramview[3][1](0xc000, 0xffff).unmaprw();
}

void multivision_state::io_map(address_map &map)
{
	// mini-disk interface board
	map(0x30, 0x30).unmaprw(); // centronics
	map(0x31, 0x33).unmaprw(); // letter quality interface
	map(0x34, 0x37).rw(m_fdc, FUNC(fd1793_device::read), FUNC(fd1793_device::write));
	map(0x38, 0x38).w(FUNC(multivision_state::floppy_select_w));
	// ram board
	map(0x40, 0x40).w(FUNC(multivision_state::ram_select_w)); // read clears parity interrupt
	map(0x41, 0x41).rw(FUNC(multivision_state::ram_parity_r), FUNC(multivision_state::ram_program_w));
	// io expander board
	map(0x52, 0x52).nopr();
	// cpu board
	map(0x80, 0x83).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x84, 0x85).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x88, 0x89).rw(m_usart[0], FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x8c, 0x8d).rw(m_usart[1], FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x90, 0x90).w(FUNC(multivision_state::serial_control_w));
	map(0x91, 0x91).r(FUNC(multivision_state::serial_status_r));
	map(0x92, 0x92).w(FUNC(multivision_state::cmos_addr_w));
	map(0x93, 0x93).rw(FUNC(multivision_state::cmos_data_r), FUNC(multivision_state::cmos_data_w));
	map(0x94, 0x94).w(FUNC(multivision_state::rtc_reset_w));
	map(0x98, 0x98).w(FUNC(multivision_state::cmos_control_w));
	map(0x9c, 0x9c).w(FUNC(multivision_state::eprom_control_w));
	map(0xa0, 0xaf).rw(m_dma, FUNC(am9517a_device::read), FUNC(am9517a_device::write));
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( multivision )
INPUT_PORTS_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void multivision_state::floppy_select_w(uint8_t data)
{
	// 7654----  not used
	// ----3---  density
	// -----2--  side
	// ------1-  select drive b
	// -------0  select drive a

	floppy_image_device *floppy = nullptr;

	if (BIT(data, 0)) floppy = m_floppy[0]->get_device();
	if (BIT(data, 1)) floppy = m_floppy[1]->get_device();

	m_fdc->set_floppy(floppy);

	m_fdc->dden_w(BIT(data, 3));

	if (floppy)
	{
		floppy->ss_w(BIT(data, 2));
		floppy->mon_w(0); // not sure how motor on works here
	}
}

void multivision_state::ram_select_w(uint8_t data)
{
	// 7654----  not used
	// ----32--  ram board select
	// ------10  ram bank select

	m_ram_select = data;
}

uint8_t multivision_state::ram_parity_r()
{
	// 7654----  not used
	// ----3---  parity error bank 3 (active low)
	// -----2--  parity error bank 2 (active low)
	// ------1-  parity error bank 1 (active low)
	// -------0  parity error bank 0 (active low)

	return 0xff;
}

void multivision_state::ram_program_w(uint8_t data)
{
	// 7654----  not used
	// ----3---  enable bank
	// -----2--  accept address lines
	// ------10  address lines a15 and a14

	uint8_t bank = m_ram_select & 0x0f;
	uint8_t addr = data & 0x03;

	logerror("bank: %d, enabled: %d, accept: %d, address %04x\n", bank, BIT(data, 3), BIT(data, 2), (data & 0x03) << 14);

	if (bank < 4)
	{
		m_rambank_enable[bank] = BIT(data, 3);

		if (BIT(data, 2))
			m_rambank[addr]->set_entry(bank);

		// update view for banks that are currently set to this bank
		for (unsigned i = 0; i < 4; i++)
		{
			if (m_rambank[i]->entry() == bank && m_ramview[i].entry() != MAP_BOOTSTRAP)
				m_ramview[i].select(BIT(data, 3) ? MAP_RAM : MAP_UNMAPPED);
		}
	}
	else
	{
		// set invalid banks as unmapped
		if (BIT(data, 2))
			m_ramview[addr].select(MAP_UNMAPPED);
	}
}

void multivision_state::serial_control_w(uint8_t data)
{
	// 765432--  not used
	// ------1-  sync/async mode
	// -------0  loopback test

	if (BIT(data, 0))
	{
		m_usart[0]->write_cts(0);
		m_usart[1]->write_cts(0);
	}
	else
	{
		m_usart[0]->write_cts(m_console->cts_r());
		m_usart[1]->write_cts(m_communication->cts_r());
	}

	m_serial_control = data;
}

uint8_t multivision_state::serial_status_r()
{
	// 76------  not used
	// --5-----  console reverse channel
	// ---4----  console cd
	// ----3---  console cts
	// -----2--  communications reverse channel
	// ------1-  communications cd
	// -------0  communications cts

	uint8_t data = 0;

	data |= m_console->dcd_r() << 4;
	data |= m_console->cts_r() << 3;
	data |= m_communication->dcd_r() << 1;
	data |= m_communication->cts_r() << 0;

	return data;
}

void multivision_state::console_txd_w(int state)
{
	if (BIT(m_serial_control, 0) == 0)
		m_console->write_txd(state);
	else
		m_usart[0]->write_rxd(state);
}

void multivision_state::console_cts_w(int state)
{
	if (BIT(m_serial_control, 0) == 0)
		m_usart[0]->write_cts(state);
}

void multivision_state::communication_txd_w(int state)
{
	if (BIT(m_serial_control, 0) == 0)
		m_communication->write_txd(state);
	else
		m_usart[1]->write_rxd(state);
}

void multivision_state::communication_cts_w(int state)
{
	if (BIT(m_serial_control, 0) == 0)
		m_usart[1]->write_cts(state);
}

void multivision_state::cmos_addr_w(uint8_t data)
{
	m_cmos_addr_latch = data;
}

uint8_t multivision_state::cmos_data_r()
{
	if (m_cmos_enable)
		return m_cmos[m_cmos_addr_latch];
	else
		return 0xff;
}

void multivision_state::cmos_data_w(uint8_t data)
{
	if (m_cmos_enable)
		m_cmos[m_cmos_addr_latch] = data;
}

void multivision_state::rtc_reset_w(uint8_t data)
{
	m_pic->ir6_w(0);
	m_rtc_counter = 10; // or 11 for 50 hz
	m_ac_line_frequency_timer->adjust(attotime::zero, 0, attotime::from_hz(60));
}

TIMER_CALLBACK_MEMBER(multivision_state::ac_line_frequency_tick)
{
	m_rtc_counter++;

	if (m_rtc_counter == 16)
	{
		m_pic->ir6_w(1);
		m_ac_line_frequency_timer->adjust(attotime::never);
	}
}

void multivision_state::cmos_control_w(uint8_t data)
{
	// 76543---  not used?
	// -----2--  cmos enable
	// ------10  not used?

	m_cmos_enable = bool(BIT(data, 2));
}

void multivision_state::eprom_control_w(uint8_t data)
{
	// 7654321-  not used?
	// -------0  eprom enable

	if (BIT(data, 0))
		m_ramview[0].select(MAP_BOOTSTRAP);
	else
		m_ramview[0].select(m_rambank_enable[0] ? MAP_RAM : MAP_UNMAPPED);
}

void multivision_state::dma_hrq_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);
	m_dma->hack_w(state);
}

uint8_t multivision_state::dma_mem_r(offs_t offset)
{
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

void multivision_state::dma_mem_w(offs_t offset, uint8_t data)
{
	m_maincpu->space(AS_PROGRAM).write_byte(offset, data);
}

void multivision_state::machine_start()
{
	m_ram = std::make_unique<uint8_t[]>(0x10000);
	m_cmos = std::make_unique<uint8_t[]>(0x100);
	m_nvram->set_base(m_cmos.get(), 0x100);

	m_rambank[0]->configure_entries(0, 4, m_ram.get(), 0x4000);
	m_rambank[1]->configure_entries(0, 4, m_ram.get(), 0x4000);
	m_rambank[2]->configure_entries(0, 4, m_ram.get(), 0x4000);
	m_rambank[3]->configure_entries(0, 4, m_ram.get(), 0x4000);

	m_ac_line_frequency_timer = timer_alloc(FUNC(multivision_state::ac_line_frequency_tick), this);

	// register for save states
	save_pointer(NAME(m_ram), 0x10000);
	save_pointer(NAME(m_cmos), 0x100);
	save_item(NAME(m_ram_select));
	save_item(NAME(m_rambank_enable));
	save_item(NAME(m_serial_control));
	save_item(NAME(m_cmos_addr_latch));
	save_item(NAME(m_cmos_enable));
	save_item(NAME(m_rtc_counter));
}

void multivision_state::machine_reset()
{
	// force bootstrap on reset for bank0
	m_ramview[0].select(MAP_BOOTSTRAP);

	// unmap other banks
	m_ramview[1].select(MAP_UNMAPPED);
	m_ramview[2].select(MAP_UNMAPPED);
	m_ramview[3].select(MAP_UNMAPPED);
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_110)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD", 0xff, RS232_BAUD_110)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_7)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY", 0xff, RS232_PARITY_ODD)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_2)
DEVICE_INPUT_DEFAULTS_END

void multivision_state::multivision(machine_config &config)
{
	I8085A(config, m_maincpu, 20_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &multivision_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &multivision_state::io_map);
	m_maincpu->in_inta_func().set(m_pic, FUNC(pic8259_device::acknowledge));

	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0);

	PIC8259(config, m_pic);
	m_pic->out_int_callback().set_inputline(m_maincpu, I8085_INTR_LINE);

	PIT8253(config, m_pit);
	m_pit->set_clk<0>(20_MHz_XTAL / 10);
	m_pit->out_handler<0>().set_inputline(m_maincpu, I8085_RST75_LINE);
	m_pit->set_clk<1>(20_MHz_XTAL / 10);
	m_pit->out_handler<1>().set(m_usart[1], FUNC(i8251_device::write_rxc));
	m_pit->out_handler<1>().append(m_usart[1], FUNC(i8251_device::write_txc));
	m_pit->set_clk<2>(20_MHz_XTAL / 10);
	m_pit->out_handler<2>().set(m_usart[0], FUNC(i8251_device::write_rxc));
	m_pit->out_handler<2>().append(m_usart[0], FUNC(i8251_device::write_txc));

	AM9517A(config, m_dma, 20_MHz_XTAL / 4);
	m_dma->out_hreq_callback().set(FUNC(multivision_state::dma_hrq_w));
	m_dma->in_memr_callback().set(FUNC(multivision_state::dma_mem_r));
	m_dma->out_memw_callback().set(FUNC(multivision_state::dma_mem_w));
	m_dma->in_ior_callback<2>().set(m_fdc, FUNC(fd1793_device::data_r));
	m_dma->out_iow_callback<2>().set(m_fdc, FUNC(fd1793_device::data_w));

	I8251(config, m_usart[0], 0);
	m_usart[0]->rxrdy_handler().set_inputline(m_maincpu, I8085_RST55_LINE);
	m_usart[0]->txrdy_handler().set(m_pic, FUNC(pic8259_device::ir7_w));
	m_usart[0]->txd_handler().set(FUNC(multivision_state::console_txd_w));
	m_usart[0]->rts_handler().set(m_console, FUNC(rs232_port_device::write_rts));

	I8251(config, m_usart[1], 0);
	m_usart[1]->rxrdy_handler().set_inputline(m_maincpu, I8085_RST65_LINE);
	m_usart[1]->txrdy_handler().set(m_pic, FUNC(pic8259_device::ir5_w));
	m_usart[1]->txd_handler().set(FUNC(multivision_state::communication_txd_w));
	m_usart[1]->rts_handler().set(m_communication, FUNC(rs232_port_device::write_rts));

	RS232_PORT(config, m_console, default_rs232_devices, "terminal");
	m_console->rxd_handler().set(m_usart[0], FUNC(i8251_device::write_rxd));
	m_console->cts_handler().set(FUNC(multivision_state::console_cts_w));
	m_console->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	RS232_PORT(config, m_communication, default_rs232_devices, nullptr);
	m_communication->rxd_handler().set(m_usart[1], FUNC(i8251_device::write_rxd));
	m_communication->cts_handler().set(FUNC(multivision_state::communication_cts_w));

	FD1793(config, m_fdc, 20_MHz_XTAL / 20);
	m_fdc->intrq_wr_callback().set(m_pic, FUNC(pic8259_device::ir3_w));
	m_fdc->drq_wr_callback().set(m_dma, FUNC(am9517a_device::dreq2_w));

	FLOPPY_CONNECTOR(config, m_floppy[0], "525dd", FLOPPY_525_DD, true, floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[1], "525dd", FLOPPY_525_DD, true, floppy_image_device::default_mfm_floppy_formats);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START(multivision)
	ROM_REGION(0x1000, "bootstrap", 0)
	ROM_LOAD("223-09102_mv_131.bin", 0x0000, 0x1000, CRC(ebe4a410) SHA1(73797ed5f7c589ba1f61ce6c6edda41d14432618))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//   YEAR  NAME         PARENT  COMPAT  MACHINE      INPUT        CLASS              INIT        COMPANY                         FULLNAME            FLAGS
COMP(1981, multivision, 0,      0,      multivision, multivision, multivision_state, empty_init, "Applied Digital Data Systems", "ADDS Multivision", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE)
