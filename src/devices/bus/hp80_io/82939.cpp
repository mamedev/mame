// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    82939.cpp

    82939 module (RS232 interface)

    Thanks to Tim Nye & Everett Kaser for dumping the 8049 ROM

    Main reference for this module is:
    HP 82939-90001, jun 81, HP82939A Serial Interface - Installation
    and Theory of operation manual

*********************************************************************/

#include "emu.h"
#include "82939.h"

// Debugging
#define VERBOSE 0
#include "logmacro.h"

// Bit manipulation
namespace {
	template<typename T> constexpr T BIT_MASK(unsigned n)
	{
		return (T)1U << n;
	}

	template<typename T> void BIT_SET(T& w , unsigned n)
	{
		w |= BIT_MASK<T>(n);
	}
}

hp82939_io_card_device::hp82939_io_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig , HP82939_IO_CARD , tag , owner , clock),
	  device_hp80_io_interface(mconfig, *this),
	  m_cpu(*this , "cpu"),
	  m_translator(*this , "xlator"),
	  m_rs232(*this , "rs232"),
	  m_uart(*this , "uart"),
	  m_sw12(*this , "sw12")
{
}

hp82939_io_card_device::~hp82939_io_card_device()
{
}

void hp82939_io_card_device::install_read_write_handlers(address_space& space , uint16_t base_addr)
{
	space.install_readwrite_handler(base_addr, base_addr + 1, read8sm_delegate(*m_translator, FUNC(hp_1mb5_device::cpu_r)), write8sm_delegate(*m_translator, FUNC(hp_1mb5_device::cpu_w)));
}

void hp82939_io_card_device::inten()
{
	m_translator->inten();
}

void hp82939_io_card_device::clear_service()
{
	m_translator->clear_service();
}

static INPUT_PORTS_START(hp82939_port)
	PORT_HP80_IO_SC(10)
	PORT_START("sw12")
	PORT_DIPNAME(0x40 , 0 , "Auto Handshake")
	PORT_DIPLOCATION("S1:1")
	PORT_DIPSETTING(0x00 , "Ignore")
	PORT_DIPSETTING(0x40 , DEF_STR(On))
	PORT_DIPNAME(0x20 , 0 , "Parity tx")
	PORT_DIPLOCATION("S2:1")
	PORT_DIPSETTING(0x00 , DEF_STR(Off))
	PORT_DIPSETTING(0x20 , DEF_STR(On))
	PORT_DIPNAME(0x10 , 0 , "Parity")
	PORT_DIPLOCATION("S2:2")
	PORT_DIPSETTING(0x00 , "Odd")
	PORT_DIPSETTING(0x10 , "Even")
	PORT_DIPNAME(0x08 , 0x08 , "Parity enable")
	PORT_DIPLOCATION("S2:3")
	PORT_DIPSETTING(0x00 , DEF_STR(Off))
	PORT_DIPSETTING(0x08 , DEF_STR(On))
	PORT_DIPNAME(0x04 , 0 , "Stop bits")
	PORT_DIPLOCATION("S2:4")
	PORT_DIPSETTING(0x00 , "1")
	PORT_DIPSETTING(0x04 , "2")
	PORT_DIPNAME(0x03 , 0x02 , "Character size")
	PORT_DIPLOCATION("S2:5,6")
	PORT_DIPSETTING(0x00 , "5")
	PORT_DIPSETTING(0x01 , "6")
	PORT_DIPSETTING(0x02 , "7")
	PORT_DIPSETTING(0x03 , "8")
	PORT_DIPNAME(0x780 , 0x300 , "Baud rate")
	PORT_DIPLOCATION("S2:7,8,9,10")
	PORT_DIPSETTING(0x000 , "50")
	PORT_DIPSETTING(0x080 , "75")
	PORT_DIPSETTING(0x100 , "110")
	PORT_DIPSETTING(0x180 , "134.5")
	PORT_DIPSETTING(0x200 , "150")
	PORT_DIPSETTING(0x280 , "200")
	PORT_DIPSETTING(0x300 , "300")
	PORT_DIPSETTING(0x380 , "600")
	PORT_DIPSETTING(0x400 , "1200")
	PORT_DIPSETTING(0x480 , "1800")
	PORT_DIPSETTING(0x500 , "2000")
	PORT_DIPSETTING(0x580 , "2400")
	PORT_DIPSETTING(0x600 , "3600")
	PORT_DIPSETTING(0x680 , "4800")
	PORT_DIPSETTING(0x700 , "7200")
	PORT_DIPSETTING(0x780 , "9600")
INPUT_PORTS_END

ioport_constructor hp82939_io_card_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(hp82939_port);
}

void hp82939_io_card_device::device_start()
{
}

ROM_START(hp82939)
	ROM_REGION(0x800 , "cpu" , 0)
	ROM_LOAD("1820-2438.bin" , 0 , 0x800 , CRC(3a2f42a2) SHA1(0f6a70eb8981a8a87c7514ce8226ff1af3ac1668))
ROM_END

uint8_t hp82939_io_card_device::p1_r()
{
	uint8_t res = uint8_t(m_sw12->read() & 0x7f);

	BIT_SET(res , 7);

	return res;
}

void hp82939_io_card_device::p1_w(uint8_t data)
{
	if (BIT(data , 7)) {
		m_uart->reset();
	}
}

uint8_t hp82939_io_card_device::p2_r()
{
	uint8_t res = uint8_t((m_sw12->read() >> 7) & 0xf);

	if (m_rs232->cts_r()) {
		BIT_SET(res , 4);
	}
	if (m_rs232->dsr_r()) {
		BIT_SET(res , 5);
	}
	// RI always reads 1
	BIT_SET(res , 6);
	if (m_rs232->dcd_r()) {
		BIT_SET(res , 7);
	}

	return res;
}

uint8_t hp82939_io_card_device::cpu_r(offs_t offset)
{
	if ((offset & 0x82) == 0x00) {
		return m_translator->uc_r(offset & 1);
	} else if ((offset & 0x83) == 0x82) {
		return m_uart->ins8250_r((offset >> 2) & 7);
	} else {
		return 0xff;
	}
}

void hp82939_io_card_device::cpu_w(offs_t offset, uint8_t data)
{
	if ((offset & 0x82) == 0x00) {
		m_translator->uc_w(offset & 1 , data);
	} else if ((offset & 0x83) == 0x82) {
		m_uart->ins8250_w((offset >> 2) & 7 , data);
	}
}

void hp82939_io_card_device::cpu_io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00 , 0xff).rw(FUNC(hp82939_io_card_device::cpu_r) , FUNC(hp82939_io_card_device::cpu_w));
}

const tiny_rom_entry *hp82939_io_card_device::device_rom_region() const
{
	return ROM_NAME(hp82939);
}

void hp82939_io_card_device::device_add_mconfig(machine_config &config)
{
	I8049(config, m_cpu, XTAL(5'529'600));
	m_cpu->set_addrmap(AS_IO, &hp82939_io_card_device::cpu_io_map);
	m_cpu->t1_in_cb().set("xlator", FUNC(hp_1mb5_device::int_r));
	m_cpu->set_t0_clk_cb("uart" , FUNC(device_t::set_unscaled_clock_int));
	m_cpu->p1_in_cb().set(FUNC(hp82939_io_card_device::p1_r));
	m_cpu->p1_out_cb().set(FUNC(hp82939_io_card_device::p1_w));
	m_cpu->p2_in_cb().set(FUNC(hp82939_io_card_device::p2_r));

	HP_1MB5(config, m_translator, 0);
	m_translator->irl_handler().set(FUNC(hp82939_io_card_device::irl_w));
	m_translator->halt_handler().set(FUNC(hp82939_io_card_device::halt_w));
	m_translator->reset_handler().set_inputline(m_cpu , INPUT_LINE_RESET);

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);

	INS8250(config , m_uart , 0);
	m_uart->out_int_callback().set_inputline(m_cpu , MCS48_INPUT_IRQ);
	m_uart->out_tx_callback().set(m_rs232 , FUNC(rs232_port_device::write_txd));
	m_uart->out_dtr_callback().set(m_rs232 , FUNC(rs232_port_device::write_dtr));
	m_uart->out_rts_callback().set(m_rs232 , FUNC(rs232_port_device::write_rts));
	m_uart->out_out1_callback().set(m_rs232 , FUNC(rs232_port_device::write_spds));

	m_rs232->rxd_handler().set(m_uart , FUNC(ins8250_device::rx_w));
	m_rs232->dcd_handler().set(m_uart , FUNC(ins8250_device::dcd_w));
	m_rs232->dsr_handler().set(m_uart , FUNC(ins8250_device::dsr_w));
	m_rs232->cts_handler().set(m_uart , FUNC(ins8250_device::cts_w));
}

// device type definition
DEFINE_DEVICE_TYPE(HP82939_IO_CARD, hp82939_io_card_device, "hp82939", "HP82939 card")
