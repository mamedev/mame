// license:BSD-3-Clause
// copyright-holders:F. Ulivi

/*********************************************************************

    HP98628 Data communication interface
    HP98629 SRM interface

    HP98628 is an asynchronous & synchronous serial interface card.
    HP98629 is a special card to interface to what HP called a
    SRM network (Shared Resource Manager). A SRM network connects
    multiple workstations to a computer acting as a NAS (in modern
    parlance). This MAME driver also emulates a HP98028 multiplexer,
    which is a kind of 5-port network bridge. One port is connected
    to emulated card, another port to a bitbanger interface and the
    remaining 3 ports are not emulated.
    For an overview of SRM see:
    https://www.hp9845.net/9845/tutorials/networks/index.html

    From the hardware point of view these cards are very similar,
    they are both based on a Z80CPU-Z80SIO-Z80CTC trio and a dual
    port RAM to interface to 68k. They differ in the firmware ROM
    and in the amount of dual-port RAM.
    A 50-pin custom connector exposes RS232 and RS422 signals on
    these cards.

    Reference docs for these cards:
    - HP, 98028-90000, HP98629A Resource Management Interface
      Schematic Diagram
    - HP98628 schematic diagram by Tony Duell
    - HP, 5955-6582, Jul 82, 98628/98629 Hardware External Reference
      Specification
    - HP, 98028-90000, HP98028A Resource Management Multiplexer
      Schematic Diagram

*********************************************************************/

#include "emu.h"
#include "hp98628_9.h"

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "imagedev/bitbngr.h"
#include "machine/timer.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"

// Debugging
//#define VERBOSE 1
#include "logmacro.h"

namespace {

// Clock
constexpr auto CLOCK = 7.3728_MHz_XTAL;

// Bit manipulation
template<typename T> constexpr T BIT_MASK(unsigned n)
{
	return (T)1U << n;
}

template<typename T> void BIT_CLR(T& w , unsigned n)
{
	w &= ~BIT_MASK<T>(n);
}

template<typename T> void BIT_SET(T& w , unsigned n)
{
	w |= BIT_MASK<T>(n);
}

// +---------------------+
// | base_98628_9_device |
// +---------------------+
class base_98628_9_device :
		public device_t,
		public bus::hp_dio::device_dio16_card_interface
{
public:
	// Input signals
	void rx_in(int state);
	void cts_in(int state);
	void dcd_in(int state);
	void ri_in(int state);
	void dsr_in(int state);
	void st_in(int state);
	void rt_in(int state);

protected:
	static inline constexpr unsigned LOW_RAM_SIZE = 2048;

	base_98628_9_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void reset_in(int state) override;

	void reset(int state, bool nmi);
	void update_irq();
	void update_modem_ctrl();
	void update_clocks();

	void base_config(machine_config &config) ATTR_COLD;
	virtual void cpu_program_mem_map(address_map &map) ATTR_COLD;
	void cpu_io_mem_map(address_map &map) ATTR_COLD;
	virtual void install_68k_map(offs_t base_addr) ATTR_COLD;
	uint8_t reg_r(offs_t addr);
	void reg_w(offs_t addr, uint8_t data);
	uint8_t low_ram_r_z80(offs_t addr);
	uint16_t low_ram_r_68k(offs_t addr);
	void low_ram_w_z80(offs_t addr, uint8_t data);
	void low_ram_w_68k(offs_t addr, uint16_t data);
	uint8_t sio_r(offs_t addr);
	void sio_w(offs_t addr, uint8_t data);
	void to0_w(int state);
	void tx_clock_w(int state);
	void to1_w(int state);
	void rx_clock_w(int state);
	void update_cpu_wait();
	void sio_wrdya_w(int state);
	void sio_int_w(int state);
	void ctc_int_w(int state);
	uint8_t sw1_r(offs_t addr);
	// Output signals
	virtual void tx_out(int state) {}
	virtual void tt_out(int state) {}
	virtual void rts_out(int state) {}
	virtual void dtr_out(int state) {}
	virtual void ocd1_out(int state) {}

	required_device<z80_device> m_cpu;
	required_device<z80ctc_device> m_ctc;
	required_device<z80sio_device> m_sio;
	required_ioport m_sw1;
	required_ioport m_sw2;

	bool m_installed_io;
	bool m_reset;
	bool m_irq;
	bool m_irq_enabled;
	bool m_semaphore;
	bool m_ctsb;
	bool m_wait_a;
	bool m_ctc_irq;
	bool m_sio_irq;
	bool m_cpu_wait;
	bool m_cpu_waiting;
	bool m_to0_div;
	bool m_to1_div;
	bool m_st_in;
	bool m_rt_in;
	uint8_t m_modem_ctrl;
	uint8_t m_modem_status;
	uint8_t m_low_ram[LOW_RAM_SIZE];
};

base_98628_9_device::base_98628_9_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, bus::hp_dio::device_dio16_card_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_ctc(*this, "ctc")
	, m_sio(*this, "sio")
	, m_sw1(*this, "sw1")
	, m_sw2(*this, "sw2")
	, m_installed_io{false}
	, m_cpu_wait{false}
	, m_cpu_waiting{false}
	, m_modem_status{0}
{
}

void base_98628_9_device::device_start()
{
	save_item(NAME(m_installed_io));
	save_item(NAME(m_reset));
	save_item(NAME(m_irq));
	save_item(NAME(m_irq_enabled));
	save_item(NAME(m_semaphore));
	save_item(NAME(m_ctsb));
	save_item(NAME(m_wait_a));
	save_item(NAME(m_ctc_irq));
	save_item(NAME(m_sio_irq));
	save_item(NAME(m_cpu_wait));
	save_item(NAME(m_cpu_waiting));
	save_item(NAME(m_to0_div));
	save_item(NAME(m_to1_div));
	save_item(NAME(m_st_in));
	save_item(NAME(m_rt_in));
	save_item(NAME(m_modem_ctrl));
	save_item(NAME(m_modem_status));
	save_item(NAME(m_low_ram));
}

constexpr unsigned SW2_SWITCH8_BIT = 7;
constexpr ioport_value SW2_SWITCH8_MASK = BIT_MASK<ioport_value>(SW2_SWITCH8_BIT);
constexpr unsigned SW2_IRQ_LEVEL_SHIFT = 5;
constexpr ioport_value SW2_IRQ_LEVEL_MASK = 3;
constexpr ioport_value SW2_IRQ_LEVEL_IRQ3 = 0;
constexpr ioport_value SW2_IRQ_LEVEL_IRQ4 = 1;
constexpr ioport_value SW2_IRQ_LEVEL_IRQ5 = 2;
constexpr ioport_value SW2_IRQ_LEVEL_IRQ6 = 3;
constexpr ioport_value SW2_IRQ_LEVEL_DEF = SW2_IRQ_LEVEL_IRQ4;
constexpr unsigned SW2_SC_SHIFT = 0;
constexpr ioport_value SW2_SC_MASK = 0x1f;
constexpr ioport_value SW2_SC_DEF = 21 << SW2_SC_SHIFT;

void base_98628_9_device::device_reset()
{
	if (!m_installed_io) {
		unsigned sc = (m_sw2->read() >> SW2_SC_SHIFT) & SW2_SC_MASK;
		offs_t base_addr = 0x600000 + sc * 0x10000;
		install_68k_map(base_addr);
		m_installed_io = true;
	}

	m_reset = false;
	reset(1, false);
}

void base_98628_9_device::reset_in(int state)
{
	if (state) {
		reset(1, false);
		m_cpu->reset();
	}
}

void base_98628_9_device::reset(int state, bool nmi)
{
	bool new_reset = bool(state);
	if (!m_reset && new_reset) {
		m_irq = false;
		m_irq_enabled = false;
		m_semaphore = false;
		m_to0_div = true;
		m_to1_div = true;
		m_ctc->reset();
		m_sio->reset();
		update_irq();
		update_clocks();
	}
	if (new_reset != m_reset) {
		m_reset = new_reset;
		update_modem_ctrl();
	}
	LOG("NMI %d\n", nmi && state);
	m_cpu->set_input_line(INPUT_LINE_NMI, nmi && state);
}

static INPUT_PORTS_START(base_98628_9_port)
	PORT_START("sw2")
	PORT_DIPNAME(SW2_SWITCH8_MASK, SW2_SWITCH8_MASK, "Switch 8")
	PORT_DIPSETTING(0, DEF_STR(On))
	PORT_DIPSETTING(SW2_SWITCH8_MASK, DEF_STR(Off))

	PORT_DIPNAME(SW2_IRQ_LEVEL_MASK << SW2_IRQ_LEVEL_SHIFT, SW2_IRQ_LEVEL_DEF << SW2_IRQ_LEVEL_SHIFT, "IRQ level")
	PORT_DIPSETTING(SW2_IRQ_LEVEL_IRQ3 << SW2_IRQ_LEVEL_SHIFT, "3")
	PORT_DIPSETTING(SW2_IRQ_LEVEL_IRQ4 << SW2_IRQ_LEVEL_SHIFT, "4")
	PORT_DIPSETTING(SW2_IRQ_LEVEL_IRQ5 << SW2_IRQ_LEVEL_SHIFT, "5")
	PORT_DIPSETTING(SW2_IRQ_LEVEL_IRQ6 << SW2_IRQ_LEVEL_SHIFT, "6")

	PORT_DIPNAME(SW2_SC_MASK << SW2_SC_SHIFT, SW2_SC_DEF, "Select code")
	PORT_DIPSETTING(0 << SW2_SC_SHIFT, "0")
	PORT_DIPSETTING(1 << SW2_SC_SHIFT, "1")
	PORT_DIPSETTING(2 << SW2_SC_SHIFT, "2")
	PORT_DIPSETTING(3 << SW2_SC_SHIFT, "3")
	PORT_DIPSETTING(4 << SW2_SC_SHIFT, "4")
	PORT_DIPSETTING(5 << SW2_SC_SHIFT, "5")
	PORT_DIPSETTING(6 << SW2_SC_SHIFT, "6")
	PORT_DIPSETTING(7 << SW2_SC_SHIFT, "7")
	PORT_DIPSETTING(8 << SW2_SC_SHIFT, "8")
	PORT_DIPSETTING(9 << SW2_SC_SHIFT, "9")
	PORT_DIPSETTING(10 << SW2_SC_SHIFT, "10")
	PORT_DIPSETTING(11 << SW2_SC_SHIFT, "11")
	PORT_DIPSETTING(12 << SW2_SC_SHIFT, "12")
	PORT_DIPSETTING(13 << SW2_SC_SHIFT, "13")
	PORT_DIPSETTING(14 << SW2_SC_SHIFT, "14")
	PORT_DIPSETTING(15 << SW2_SC_SHIFT, "15")
	PORT_DIPSETTING(16 << SW2_SC_SHIFT, "16")
	PORT_DIPSETTING(17 << SW2_SC_SHIFT, "17")
	PORT_DIPSETTING(18 << SW2_SC_SHIFT, "18")
	PORT_DIPSETTING(19 << SW2_SC_SHIFT, "19")
	PORT_DIPSETTING(20 << SW2_SC_SHIFT, "20")
	PORT_DIPSETTING(21 << SW2_SC_SHIFT, "21")
	PORT_DIPSETTING(22 << SW2_SC_SHIFT, "22")
	PORT_DIPSETTING(23 << SW2_SC_SHIFT, "23")
	PORT_DIPSETTING(24 << SW2_SC_SHIFT, "24")
	PORT_DIPSETTING(25 << SW2_SC_SHIFT, "25")
	PORT_DIPSETTING(26 << SW2_SC_SHIFT, "26")
	PORT_DIPSETTING(27 << SW2_SC_SHIFT, "27")
	PORT_DIPSETTING(28 << SW2_SC_SHIFT, "28")
	PORT_DIPSETTING(29 << SW2_SC_SHIFT, "29")
	PORT_DIPSETTING(30 << SW2_SC_SHIFT, "30")
	PORT_DIPSETTING(31 << SW2_SC_SHIFT, "31")

INPUT_PORTS_END

void base_98628_9_device::update_irq()
{
	auto level = (m_sw2->read() >> SW2_IRQ_LEVEL_SHIFT) & SW2_IRQ_LEVEL_MASK;
	bool irq = m_irq && m_irq_enabled;
	irq3_out(irq && level == SW2_IRQ_LEVEL_IRQ3);
	irq4_out(irq && level == SW2_IRQ_LEVEL_IRQ4);
	irq5_out(irq && level == SW2_IRQ_LEVEL_IRQ5);
	irq6_out(irq && level == SW2_IRQ_LEVEL_IRQ6);
}

void base_98628_9_device::update_modem_ctrl()
{
	if (m_reset) {
		rts_out(1);
		dtr_out(1);
	} else {
		rts_out(!BIT(m_modem_ctrl, 0));
		dtr_out(!BIT(m_modem_ctrl, 1));
	}
	ocd1_out(!BIT(m_modem_ctrl, 2));
	// Bit 3 is OCD2
	// Bit 4 is OCD3
	// Bit 5 is OCD4
}

void base_98628_9_device::update_clocks()
{
	if (BIT(m_modem_ctrl, 6)) {
		tx_clock_w(m_to0_div);
	} else {
		tx_clock_w(m_st_in);
	}
	if (BIT(m_modem_ctrl, 7)) {
		rx_clock_w(m_to1_div);
	} else {
		rx_clock_w(m_rt_in);
	}
}

static const z80_daisy_config daisy_chain_config[] = {
	{ "sio" },
	{ "ctc" },
	{ nullptr }
};

void base_98628_9_device::base_config(machine_config &config)
{
	Z80(config, m_cpu, CLOCK / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &base_98628_9_device::cpu_program_mem_map);
	m_cpu->set_addrmap(AS_IO, &base_98628_9_device::cpu_io_mem_map);
	m_cpu->set_daisy_config(daisy_chain_config);

	Z80SIO(config, m_sio, CLOCK / 2);
	m_sio->out_int_callback().set(FUNC(base_98628_9_device::sio_int_w));
	m_sio->out_txda_callback().set(m_sio, FUNC(z80sio_device::rxb_w));
	m_sio->out_txda_callback().append(FUNC(base_98628_9_device::tx_out));
	m_sio->out_rtsb_callback().set(m_sio, FUNC(z80sio_device::dcdb_w));
	m_sio->out_wrdya_callback().set(FUNC(base_98628_9_device::sio_wrdya_w));

	Z80CTC(config, m_ctc, CLOCK / 2);
	m_ctc->intr_callback().set(FUNC(base_98628_9_device::ctc_int_w));
	m_ctc->zc_callback<0>().set(FUNC(base_98628_9_device::to0_w));
	m_ctc->zc_callback<1>().set(FUNC(base_98628_9_device::to1_w));
	m_ctc->zc_callback<2>().set(m_sio, FUNC(z80sio_device::txcb_w));
	m_ctc->set_clk<0>(CLOCK / 4);
	m_ctc->set_clk<1>(CLOCK / 4);
}

void base_98628_9_device::cpu_program_mem_map(address_map &map)
{
	map.unmap_value_low();
	map(0x0000, 0x1fff).mirror(0x6000).rom();
	map(0x8000, 0x8003).rw(FUNC(base_98628_9_device::reg_r), FUNC(base_98628_9_device::reg_w));
	map(0xa000, 0xbfff).rw(FUNC(base_98628_9_device::low_ram_r_z80), FUNC(base_98628_9_device::low_ram_w_z80));
}

void base_98628_9_device::cpu_io_mem_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x70, 0x73).rw(FUNC(base_98628_9_device::sio_r), FUNC(base_98628_9_device::sio_w));
	map(0xb0, 0xb3).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0xd0, 0xd1).r(FUNC(base_98628_9_device::sw1_r));
}

void base_98628_9_device::install_68k_map(offs_t base_addr)
{
	dio().install_memory(
			0x0000 + base_addr,
			0x0007 + base_addr,
			read16sm_delegate(*this, NAME([this] (offs_t addr) { return reg_r(addr); })),
			write16sm_delegate(*this, NAME([this] (offs_t addr, uint16_t data) { reg_w(addr, uint8_t(data)); })));
	dio().install_memory(
			0x4000 + base_addr,
			0x7fff + base_addr,
			read16sm_delegate(*this, FUNC(base_98628_9_device::low_ram_r_68k)),
			write16sm_delegate(*this, FUNC(base_98628_9_device::low_ram_w_68k)));
}

uint8_t base_98628_9_device::reg_r(offs_t addr)
{
	uint8_t res = 0;

	switch (addr & 3) {
	case 0:
		// ID register
		res = 0x34;
		if (!BIT(m_sw2->read(), SW2_SWITCH8_BIT)) {
			BIT_SET(res, 7);
		}
		break;
	case 1:
		// IRQ register
		//
		// |  Bit | Content     |
		// |------+-------------|
		// |    7 | IRQ enabled |
		// |    6 | IRQ pending |
		// | 5..4 | IRQ level   |
		// | 3..0 | 0           |
		if (m_irq_enabled) {
			BIT_SET(res, 7);
		}
		if (m_irq) {
			BIT_SET(res, 6);
		}
		res |= ((m_sw2->read() >> SW2_IRQ_LEVEL_SHIFT) & SW2_IRQ_LEVEL_MASK) << 4;
		break;
	case 2:
		// Semaphore
		// Read & clear
		if (!m_semaphore) {
			BIT_SET(res, 7);
		}
		if (!machine().side_effects_disabled()) {
			m_semaphore = false;
		}
		break;
	case 3:
		// Modem status
		//
		// |  Bit | Content     |
		// |------+-------------|
		// | 7..6 | -           |
		// |    5 | !Wait/Rdy A |
		// |    4 | OCR2        |
		// |    3 | RI/OCR1     |
		// |    2 | CTS/CS      |
		// |    1 | DCD/RR      |
		// |    0 | DSR/DM      |
		res = m_modem_status;
		if (!m_wait_a) {
			BIT_SET(res, 5);
		}
		break;
   }

	LOG("REG R %u=%02x\n", addr, res);
	return res;
}

void base_98628_9_device::reg_w(offs_t addr, uint8_t data)
{
	LOG("REG W %u=%02x\n", addr, data);
	switch (addr & 3) {
	case 0:
		// Reset control
		reset(BIT(data, 7), true);
		break;
	case 1:
		// IRQ enable
		if (!m_reset) {
			m_irq_enabled = BIT(data, 7);
			update_irq();
		}
		break;
	case 2:
		// Semaphore
		// Set
		if (!m_reset && !machine().side_effects_disabled()) {
			m_semaphore = true;
		}
		break;
	case 3:
		// Modem control
		{
			uint8_t diff = m_modem_ctrl ^ data;
			m_modem_ctrl = data;
			if (diff & 0xc0) {
				update_clocks();
			}
			if (diff & 0x3f) {
				update_modem_ctrl();
			}
		}
		break;
	}
}

uint8_t base_98628_9_device::low_ram_r_z80(offs_t addr)
{
	if (addr == 1 && !machine().side_effects_disabled()) {
		// Clear CTSB/
		m_ctsb = false;
		m_sio->ctsb_w(m_ctsb);
		LOG("CTSB 0\n");
	}
	return m_low_ram[addr & (LOW_RAM_SIZE - 1)];
}

uint16_t base_98628_9_device::low_ram_r_68k(offs_t addr)
{
	if (addr == 0 && !machine().side_effects_disabled()) {
		// Clear IRQ
		m_irq = false;
		update_irq();
		LOG("IRQ 0\n");
	}
	return m_low_ram[addr & (LOW_RAM_SIZE - 1)];
}

void base_98628_9_device::low_ram_w_z80(offs_t addr, uint8_t data)
{
	if (addr == 0 && !m_reset && !machine().side_effects_disabled()) {
		// Set IRQ
		m_irq = true;
		LOG("IRQ 1\n");
		update_irq();
	}
	m_low_ram[addr & (LOW_RAM_SIZE - 1)] = data;
}

void base_98628_9_device::low_ram_w_68k(offs_t addr, uint16_t data)
{
	if (addr == 1 && !machine().side_effects_disabled()) {
		// Set CTSB/
		m_ctsb = true;
		LOG("CTSB 1\n");
		m_sio->ctsb_w(m_ctsb);
	}
	m_low_ram[addr & (LOW_RAM_SIZE - 1)] = uint8_t(data);
}

uint8_t base_98628_9_device::sio_r(offs_t addr)
{
	uint8_t res;

	if (addr == 0 && m_cpu_wait && !machine().side_effects_disabled()) {
		// ch-A data register can't be read right now
		m_cpu->set_input_line(Z80_INPUT_LINE_WAIT, ASSERT_LINE);
		m_cpu->defer_access();
		m_cpu_waiting = true;
		res = 0;
		LOG("Z80 stalled\n");
	} else {
		res = m_sio->ba_cd_r(addr);
	}
	LOG("SIO R %u=%02x\n", addr, res);
	return res;
}

void base_98628_9_device::sio_w(offs_t addr, uint8_t data)
{
	LOG("SIO W %u=%02x\n", addr, data);
	if (addr == 0 && m_cpu_wait && !machine().side_effects_disabled()) {
		// ch-A data register can't be written right now
		m_cpu->set_input_line(Z80_INPUT_LINE_WAIT, ASSERT_LINE);
		m_cpu->defer_access();
		m_cpu_waiting = true;
		LOG("Z80 stalled\n");
	} else {
		m_sio->ba_cd_w(addr, data);
	}
}

void base_98628_9_device::to0_w(int state)
{
	if (!m_reset && state) {
		m_to0_div = !m_to0_div;
		if (BIT(m_modem_ctrl, 6)) {
			tx_clock_w(m_to0_div);
		}
	}
}

void base_98628_9_device::tx_clock_w(int state)
{
	m_sio->txca_w(state);
	m_sio->rxcb_w(state);
	m_ctc->trg2(state);
	tt_out(state);
}

void base_98628_9_device::to1_w(int state)
{
	if (!m_reset && state) {
		m_to1_div = !m_to1_div;
		if (BIT(m_modem_ctrl, 7)) {
			rx_clock_w(m_to1_div);
		}
	}
}

void base_98628_9_device::rx_clock_w(int state)
{
	m_sio->rxca_w(state);
	m_ctc->trg3(state);
}

void base_98628_9_device::update_cpu_wait()
{
	m_cpu_wait = !m_wait_a && !m_sio_irq && !m_ctc_irq;
	if (m_cpu_waiting && !m_cpu_wait) {
		LOG("WAIT released wa %d si %d ci %d\n", m_wait_a, m_sio_irq, m_ctc_irq);
		m_cpu->set_input_line(Z80_INPUT_LINE_WAIT, CLEAR_LINE);
		m_cpu_waiting = false;
	}
}

void base_98628_9_device::sio_wrdya_w(int state)
{
	LOG("WRDYA %d\n", state);
	m_wait_a = bool(state);
	update_cpu_wait();
}

void base_98628_9_device::sio_int_w(int state)
{
	LOG("SIO IRQ %d\n", state);
	m_sio_irq = bool(state);
	m_cpu->set_input_line(INPUT_LINE_IRQ0, m_sio_irq || m_ctc_irq);
	update_cpu_wait();
}

void base_98628_9_device::ctc_int_w(int state)
{
	LOG("CTC IRQ %d\n", state);
	m_ctc_irq = bool(state);
	m_cpu->set_input_line(INPUT_LINE_IRQ0, m_sio_irq || m_ctc_irq);
	update_cpu_wait();
}

uint8_t base_98628_9_device::sw1_r(offs_t addr)
{
	return uint8_t((m_sw1->read() >> (BIT(addr, 0) ? 4 : 0)) & 0x0f);
}

void base_98628_9_device::rx_in(int state)
{
	m_sio->rxa_w(state);
}

void base_98628_9_device::cts_in(int state)
{
	LOG("CTSA %d\n", state);
	m_sio->ctsa_w(state);
	if (state) {
		BIT_CLR(m_modem_status, 2);
	} else {
		BIT_SET(m_modem_status, 2);
	}
}

void base_98628_9_device::dcd_in(int state)
{
	m_sio->dcda_w(state);
	if (state) {
		BIT_CLR(m_modem_status, 1);
	} else {
		BIT_SET(m_modem_status, 1);
	}
}

void base_98628_9_device::ri_in(int state)
{
	if (state) {
		BIT_CLR(m_modem_status, 3);
	} else {
		BIT_SET(m_modem_status, 3);
	}
}

void base_98628_9_device::dsr_in(int state)
{
	if (state) {
		BIT_CLR(m_modem_status, 0);
	} else {
		BIT_SET(m_modem_status, 0);
	}
}

void base_98628_9_device::st_in(int state)
{
	m_st_in = bool(state);
	if (!BIT(m_modem_ctrl, 6)) {
		tx_clock_w(state);
	}
}

void base_98628_9_device::rt_in(int state)
{
	m_rt_in = bool(state);
	if (!BIT(m_modem_ctrl, 7)) {
		rx_clock_w(state);
	}
}

// +--------------------+
// | dio16_98628_device |
// +--------------------+
class dio16_98628_device :
		public base_98628_9_device
{
public:
	dio16_98628_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: dio16_98628_device(mconfig, HPDIO_98628, tag, owner, clock)
	{
	}

protected:
	dio16_98628_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: base_98628_9_device(mconfig, type, tag, owner, clock)
		, m_rs232(*this, "rs232")
	{
	}

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void tx_out(int state) override;
	virtual void tt_out(int state) override;
	virtual void rts_out(int state) override;
	virtual void dtr_out(int state) override;
	virtual void ocd1_out(int state) override;

	required_device<rs232_port_device> m_rs232;
};

void dio16_98628_device::device_add_mconfig(machine_config &config)
{
	base_config(config);

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(FUNC(base_98628_9_device::rx_in));
	m_rs232->dcd_handler().set(FUNC(base_98628_9_device::dcd_in));
	m_rs232->dsr_handler().set(FUNC(base_98628_9_device::dsr_in));
	m_rs232->ri_handler().set(FUNC(base_98628_9_device::ri_in));
	m_rs232->cts_handler().set(FUNC(base_98628_9_device::cts_in));
	m_rs232->rxc_handler().set(FUNC(base_98628_9_device::rt_in));
	m_rs232->txc_handler().set(FUNC(base_98628_9_device::st_in));
}

static INPUT_PORTS_START(dio98628_port)
	PORT_INCLUDE(base_98628_9_port)

	PORT_START("sw1")
	PORT_DIPNAME(0xc0, 0x80, "Parity,bits per char")
	PORT_DIPSETTING(0x00, "N,8")
	PORT_DIPSETTING(0x40, "N,7")
	PORT_DIPSETTING(0x80, "O,7")
	PORT_DIPSETTING(0xc0, "E,7")
	PORT_DIPNAME(0x30, 0x10, "HW handshake")
	PORT_DIPSETTING(0x00, "HS OFF,no modem")
	PORT_DIPSETTING(0x10, "Full dplx modem")
	PORT_DIPSETTING(0x20, "Half dplx modem")
	PORT_DIPSETTING(0x30, "HS ON,no modem")
	PORT_DIPNAME(0x0e, 0x04, "Speed,stop bits")
	PORT_DIPSETTING(0x00, "110,2")
	PORT_DIPSETTING(0x02, "150,2")
	PORT_DIPSETTING(0x04, "300,1")
	PORT_DIPSETTING(0x06, "600,1")
	PORT_DIPSETTING(0x08, "1200,1")
	PORT_DIPSETTING(0x0a, "2400,1")
	PORT_DIPSETTING(0x0c, "4800,1")
	PORT_DIPSETTING(0x0e, "9600,1")
	PORT_DIPNAME(0x01, 0x00, "Mode")
	PORT_DIPSETTING(0x00, "Asynchronuous")
	PORT_DIPSETTING(0x01, "Data link")
INPUT_PORTS_END

ioport_constructor dio16_98628_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(dio98628_port);
}

ROM_START(dio98628)
	ROM_REGION(0x2000, "cpu", 0)
	ROM_LOAD("98628-81003.bin", 0, 0x2000, CRC(ac51d596) SHA1(738fa16ac9a0f865938e7b197d26bd37c27b8660))
ROM_END

const tiny_rom_entry *dio16_98628_device::device_rom_region() const
{
	return ROM_NAME(dio98628);
}

void dio16_98628_device::tx_out(int state)
{
	m_rs232->write_txd(state);
}

void dio16_98628_device::tt_out(int state)
{
	m_rs232->write_etc(state);
}

void dio16_98628_device::rts_out(int state)
{
	m_rs232->write_rts(state);
}

void dio16_98628_device::dtr_out(int state)
{
	m_rs232->write_dtr(state);
}

void dio16_98628_device::ocd1_out(int state)
{
	m_rs232->write_spds(state);
}

// +--------------------+
// | dio16_98629_device |
// +--------------------+
class dio16_98629_device :
		public base_98628_9_device
{
public:
	dio16_98629_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: dio16_98629_device(mconfig, HPDIO_98629, tag, owner, clock)
	{
	}

protected:
	static inline constexpr unsigned HIGH_RAM_SIZE = 2048;
	static inline constexpr unsigned MUX_FREQ = 700000;

	enum class fsm_st {
		ST_PROBING_MY_PORT,
		ST_PROBING_EXT_PORT,
		ST_MY_PORT_TX,
		ST_MY_PORT_RX,
		ST_WAITING_RX,
		ST_PAUSE
	};

	dio16_98629_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: base_98628_9_device(mconfig, type, tag, owner, clock)
		, m_stream(*this, "stream")
		, m_timer(*this, "tmr")
	{
	}

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void cpu_program_mem_map(address_map &map) override ATTR_COLD;
	virtual void install_68k_map(offs_t base_addr) override ATTR_COLD;
	uint8_t high_ram_r_z80(offs_t addr);
	void high_ram_w_z80(offs_t addr, uint8_t data);
	virtual void tx_out(int state) override;
	virtual void tt_out(int state) override;

	TIMER_DEVICE_CALLBACK_MEMBER(timer_to);

	required_device<bitbanger_device> m_stream;
	required_device<timer_device> m_timer;

	fsm_st m_state;
	unsigned m_byte_cnt;
	bool m_half_cycle;
	bool m_tx;
	bool m_last_tt;
	uint8_t m_sr;
	uint8_t m_high_ram[HIGH_RAM_SIZE];
};

void dio16_98629_device::device_start()
{
	base_98628_9_device::device_start();
	save_item(NAME(m_byte_cnt));
	save_item(NAME(m_half_cycle));
	save_item(NAME(m_tx));
	save_item(NAME(m_last_tt));
	save_item(NAME(m_sr));
	save_item(NAME(m_high_ram));
}

void dio16_98629_device::device_reset()
{
	base_98628_9_device::device_reset();

	m_state = fsm_st::ST_PROBING_MY_PORT;
	m_byte_cnt = 0;
	m_half_cycle = false;
	m_sr = 0xff;
	rx_in(1);
	cts_in(1);
	st_in(1);
	rt_in(1);
	m_timer->adjust(attotime::from_hz(MUX_FREQ / 8), 0, attotime::from_hz(MUX_FREQ / 8));
}

void dio16_98629_device::device_add_mconfig(machine_config &config)
{
	base_config(config);

	BITBANGER(config, m_stream, 0);
	TIMER(config, m_timer).configure_generic(FUNC(dio16_98629_device::timer_to));
}

static INPUT_PORTS_START(dio98629_port)
	PORT_INCLUDE(base_98628_9_port)

	PORT_START("sw1")
	PORT_DIPNAME(0x3f, 0x0a, "Bus address")
	PORT_DIPSETTING(0, "0")
	PORT_DIPSETTING(1, "1")
	PORT_DIPSETTING(2, "2")
	PORT_DIPSETTING(3, "3")
	PORT_DIPSETTING(4, "4")
	PORT_DIPSETTING(5, "5")
	PORT_DIPSETTING(6, "6")
	PORT_DIPSETTING(7, "7")
	PORT_DIPSETTING(8, "8")
	PORT_DIPSETTING(9, "9")
	PORT_DIPSETTING(10, "10")
	PORT_DIPSETTING(11, "11")
	PORT_DIPSETTING(12, "12")
	PORT_DIPSETTING(13, "13")
	PORT_DIPSETTING(14, "14")
	PORT_DIPSETTING(15, "15")
	PORT_DIPSETTING(16, "16")
	PORT_DIPSETTING(17, "17")
	PORT_DIPSETTING(18, "18")
	PORT_DIPSETTING(19, "19")
	PORT_DIPSETTING(20, "20")
	PORT_DIPSETTING(21, "21")
	PORT_DIPSETTING(22, "22")
	PORT_DIPSETTING(23, "23")
	PORT_DIPSETTING(24, "24")
	PORT_DIPSETTING(25, "25")
	PORT_DIPSETTING(26, "26")
	PORT_DIPSETTING(27, "27")
	PORT_DIPSETTING(28, "28")
	PORT_DIPSETTING(29, "29")
	PORT_DIPSETTING(30, "30")
	PORT_DIPSETTING(31, "31")
	PORT_DIPSETTING(32, "32")
	PORT_DIPSETTING(33, "33")
	PORT_DIPSETTING(34, "34")
	PORT_DIPSETTING(35, "35")
	PORT_DIPSETTING(36, "36")
	PORT_DIPSETTING(37, "37")
	PORT_DIPSETTING(38, "38")
	PORT_DIPSETTING(39, "39")
	PORT_DIPSETTING(40, "40")
	PORT_DIPSETTING(41, "41")
	PORT_DIPSETTING(42, "42")
	PORT_DIPSETTING(43, "43")
	PORT_DIPSETTING(44, "44")
	PORT_DIPSETTING(45, "45")
	PORT_DIPSETTING(46, "46")
	PORT_DIPSETTING(47, "47")
	PORT_DIPSETTING(48, "48")
	PORT_DIPSETTING(49, "49")
	PORT_DIPSETTING(50, "50")
	PORT_DIPSETTING(51, "51")
	PORT_DIPSETTING(52, "52")
	PORT_DIPSETTING(53, "53")
	PORT_DIPSETTING(54, "54")
	PORT_DIPSETTING(55, "55")
	PORT_DIPSETTING(56, "56")
	PORT_DIPSETTING(57, "57")
	PORT_DIPSETTING(58, "58")
	PORT_DIPSETTING(59, "59")
	PORT_DIPSETTING(60, "60")
	PORT_DIPSETTING(61, "61")
	PORT_DIPSETTING(62, "62")
	PORT_DIPSETTING(63, "63")
INPUT_PORTS_END

ioport_constructor dio16_98629_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(dio98629_port);
}

void dio16_98629_device::cpu_program_mem_map(address_map &map)
{
	base_98628_9_device::cpu_program_mem_map(map);
	map(0xc000, 0xdfff).rw(FUNC(dio16_98629_device::high_ram_r_z80), FUNC(dio16_98629_device::high_ram_w_z80));
}

void dio16_98629_device::install_68k_map(offs_t base_addr)
{
	base_98628_9_device::install_68k_map(base_addr);
	dio().install_memory(
			0x8000 + base_addr,
			0xbfff + base_addr,
			read16sm_delegate(*this, NAME([this] (offs_t addr) { return high_ram_r_z80(addr); })),
			write16sm_delegate(*this, NAME([this] (offs_t addr, uint16_t data) { high_ram_w_z80(addr, uint8_t(data)); })));
}

uint8_t dio16_98629_device::high_ram_r_z80(offs_t addr)
{
	return m_high_ram[addr & (HIGH_RAM_SIZE - 1)];
}

void dio16_98629_device::high_ram_w_z80(offs_t addr, uint8_t data)
{
	m_high_ram[addr & (HIGH_RAM_SIZE - 1)] = data;
}

void dio16_98629_device::tx_out(int state)
{
	m_tx = bool(state);
	if (m_state == fsm_st::ST_PROBING_MY_PORT || m_state == fsm_st::ST_MY_PORT_TX) {
		rx_in(state);
	}
}

void dio16_98629_device::tt_out(int state)
{
	if (m_state != fsm_st::ST_PROBING_MY_PORT && m_state != fsm_st::ST_MY_PORT_TX) {
		return;
	}

	if (!m_last_tt && state) {
		// Sample bit on rising edge
		m_sr >>= 1;
		if (m_tx) {
			BIT_SET(m_sr, 7);
		}
		cts_in(m_sr == 0xff);
	}
	m_last_tt = bool(state);
}

TIMER_DEVICE_CALLBACK_MEMBER(dio16_98629_device::timer_to)
{
	bool stay;
	do {
		stay = false;

		switch (m_state) {
		case fsm_st::ST_PROBING_MY_PORT:
			for (unsigned i = 0; i < 8; i++) {
				st_in(0);
				rt_in(0);
				st_in(1);
				rt_in(1);
			}
			if (m_sr == 0xff) {
				m_half_cycle = !m_half_cycle;
				if (!m_half_cycle) {
					m_state = fsm_st::ST_PROBING_EXT_PORT;
				}
			} else {
				LOG("TX %02x (start)\n", m_sr);
				m_stream->output(m_sr);
				m_state = fsm_st::ST_MY_PORT_TX;
			}
			break;

		case fsm_st::ST_PROBING_EXT_PORT:
			if (m_stream->input(&m_sr, 1) == 1 && m_sr != 0xff) {
				m_state = fsm_st::ST_MY_PORT_RX;
				stay = true;
			} else {
				m_half_cycle = !m_half_cycle;
				if (!m_half_cycle) {
					m_byte_cnt = 0;
					m_state = fsm_st::ST_PAUSE;
				}
			}
			break;

		case fsm_st::ST_MY_PORT_TX:
			for (unsigned i = 0; i < 8; i++) {
				st_in(0);
				rt_in(0);
				st_in(1);
				rt_in(1);
			}
			LOG("TX %02x\n", m_sr);
			m_stream->output(m_sr);
			if (m_sr == 0xff) {
				m_half_cycle = !m_half_cycle;
				if (!m_half_cycle) {
					LOG("TX ended\n");
					m_state = fsm_st::ST_PROBING_EXT_PORT;
				}
			}
			break;

		case fsm_st::ST_WAITING_RX:
			if (m_stream->input(&m_sr, 1) == 1) {
				m_state = fsm_st::ST_MY_PORT_RX;
				stay = true;
			}
			break;

		case fsm_st::ST_MY_PORT_RX:
			{
				LOG("RX %02x\n", m_sr);
				uint8_t tmp = m_sr;
				for (unsigned i = 0; i < 8; i++) {
					st_in(0);
					rt_in(0);
					rx_in(BIT(tmp, 0));
					tmp >>= 1;
					st_in(1);
					rt_in(1);
				}
				m_state = fsm_st::ST_WAITING_RX;
				if (m_sr == 0xff) {
					m_half_cycle = !m_half_cycle;
					if (!m_half_cycle) {
						LOG("RX ended\n");
						m_byte_cnt = 0;
						m_state = fsm_st::ST_PAUSE;
					}
				}
			}
			break;

		case fsm_st::ST_PAUSE:
			m_byte_cnt++;
			if (m_byte_cnt == 6) {
				m_state = fsm_st::ST_PROBING_MY_PORT;
			}
			break;
		}
	} while (stay);
}

ROM_START(dio98629)
	ROM_REGION(0x2000, "cpu", 0)
	ROM_LOAD("1818-1739b.bin", 0, 0x2000, CRC(5955894e) SHA1(b02b3688a8e415701905c7a1d8ae8848fd9b8b42))
ROM_END

const tiny_rom_entry *dio16_98629_device::device_rom_region() const
{
	return ROM_NAME(dio98629);
}

}  // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(HPDIO_98628, bus::hp_dio::device_dio16_card_interface, dio16_98628_device, "dio98628", "HP98628 data communication interface")
DEFINE_DEVICE_TYPE_PRIVATE(HPDIO_98629, bus::hp_dio::device_dio16_card_interface, dio16_98629_device, "dio98629", "HP98629 SRM interface")
