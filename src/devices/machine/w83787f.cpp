// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Winbond W83787IF / W83787F

'F is the base, 'IF adds IrDA.
Looks similar in design to National PC87306 (including similar reg names)

**************************************************************************************************/

#include "emu.h"
#include "bus/isa/isa.h"
#include "machine/w83787f.h"

#define LOG_WARN        (1U << 1)

#define VERBOSE (LOG_GENERAL | LOG_WARN)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)       LOGMASKED(LOG_WARN, __VA_ARGS__)

DEFINE_DEVICE_TYPE(W83787F, w83787f_device, "w83787f", "National Semiconductor W83787F Super I/O Enhanced Sidewinder Lite")

w83787f_device::w83787f_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, W83787F, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_space_config("superio_config_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(w83787f_device::config_map), this))
	, m_pc_com(*this, "uart%d", 0U)
	, m_pc_lpt(*this, "lpta")
	, m_irq1_callback(*this)
	, m_irq8_callback(*this)
	, m_irq9_callback(*this)
	, m_txd1_callback(*this)
	, m_ndtr1_callback(*this)
	, m_nrts1_callback(*this)
	, m_txd2_callback(*this)
	, m_ndtr2_callback(*this)
	, m_nrts2_callback(*this)
{ }


void w83787f_device::device_start()
{
	set_isa_device();
	//m_isa->set_dma_channel(0, this, true);
	//m_isa->set_dma_channel(1, this, true);
	//m_isa->set_dma_channel(2, this, true);
	//m_isa->set_dma_channel(3, this, true);
	remap(AS_IO, 0, 0x400);
}

void w83787f_device::device_reset()
{
	m_locked_state = true;
	m_cr1 = 0;
}

device_memory_interface::space_config_vector w83787f_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

void w83787f_device::device_add_mconfig(machine_config &config)
{
	PC_LPT(config, m_pc_lpt);
	m_pc_lpt->irq_handler().set(FUNC(w83787f_device::irq_parallel_w));

	NS16550(config, m_pc_com[0], XTAL(1'843'200));
	m_pc_com[0]->out_int_callback().set(FUNC(w83787f_device::irq_serial1_w));
	m_pc_com[0]->out_tx_callback().set(FUNC(w83787f_device::txd_serial1_w));
	m_pc_com[0]->out_dtr_callback().set(FUNC(w83787f_device::dtr_serial1_w));
	m_pc_com[0]->out_rts_callback().set(FUNC(w83787f_device::rts_serial1_w));

	NS16550(config, m_pc_com[1], XTAL(1'843'200));
	m_pc_com[1]->out_int_callback().set(FUNC(w83787f_device::irq_serial2_w));
	m_pc_com[1]->out_tx_callback().set(FUNC(w83787f_device::txd_serial2_w));
	m_pc_com[1]->out_dtr_callback().set(FUNC(w83787f_device::dtr_serial2_w));
	m_pc_com[1]->out_rts_callback().set(FUNC(w83787f_device::rts_serial2_w));
}

void w83787f_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
	{
		m_isa->install_device(0x0250, 0x0252, read8sm_delegate(*this, FUNC(w83787f_device::read)), write8sm_delegate(*this, FUNC(w83787f_device::write)));

		//if (BIT(m_fer, 0))
		const u8 lpt_setting = (m_cr1 >> 4) & 3;
		if (lpt_setting != 3)
		{
			const u16 lpt_port[3] = { 0x3bc, 0x278, 0x378 };
			const u16 lpt_addr = lpt_port[lpt_setting & 3];
			LOG("Map LPT1 to I/O port %04x-%04x\n", lpt_addr, lpt_addr + 3);

			m_isa->install_device(lpt_addr, lpt_addr + 3, read8sm_delegate(*m_pc_lpt, FUNC(pc_lpt_device::read)), write8sm_delegate(*m_pc_lpt, FUNC(pc_lpt_device::write)));
		}

		for (int i = 0; i < 2; i++)
		{
			const u8 uart_setting = (BIT(m_cr1, 2 + i) >> 1) | (BIT(m_cr1, i));
			if (uart_setting != 3)
			{
				const u16 uart_port[3] = { 0x2e8, 0x3e8, 0x3f8 };
				const u16 uart_addr = uart_port[uart_setting & 3] ^ (i ? 0x100 : 0x000);
				LOG("Map UART%c to I/O port %04x-%04x\n", i ? 'A' : 'B', uart_addr, uart_addr + 7);
				m_isa->install_device(uart_addr, uart_addr + 7, read8sm_delegate(*m_pc_com[i], FUNC(ns16450_device::ins8250_r)), write8sm_delegate(*m_pc_com[i], FUNC(ns16450_device::ins8250_w)));
			}
		}
	}
}

u8 w83787f_device::read(offs_t offset)
{
	if (offset != 2 && !machine().side_effects_disabled())
	{
		LOGWARN("Invalid %s access read\n", offset & 1 ? "EFIR" : "EFIR");
		return space().unmap();
	}

	if (m_locked_state)
		return space().unmap();

	return space().read_byte(m_index);
}

void w83787f_device::write(offs_t offset, u8 data)
{
	switch (offset)
	{
		// EFER
		// TODO: 0x89 with GMRD# pin
		case 0: m_locked_state = (data != 0x88); break;
		// EFIR
		case 1: m_index = data; break;
		// EFDR
		case 2:
			if (!m_locked_state)
				space().write_byte(m_index, data);
			break;
	}
}

// none of these regs have a real naming, they are all CR*
void w83787f_device::config_map(address_map &map)
{
//	map(0x00, 0x00) IDE & FDC
	map(0x01, 0x01).lrw8(
		NAME([this] (offs_t offset) {
			return m_cr1;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_cr1 = data;
			remap(AS_IO, 0, 0x400);
		})
	);
//	map(0x02, 0x02) extension adapter mode
//	map(0x03, 0x03) game port, UART clocks
//	map(0x04, 0x04) game port, UARTA/B power-down tristate
//	map(0x05, 0x05) ECP FIFO threshold
//	map(0x06, 0x06) 2x / x4 FDD select, FDC power-down tristate, IDE power-down tristate
//	map(0x07, 0x07) FDDs type
//	map(0x08, 0x08) automatic power-down, FDD write protect
//	map(0x09, 0x09) CHIP ID, lock alias, operating mode
//	map(0x0a, 0x0a) LPT pins
//	map(0x0c, 0x0c) UARTA/B clock source, lock select
//	map(0x0d, 0x0d) IrDA select
//	map(0x0e, 0x0f) <reserved for test>
//	map(0x10, 0x10) GIO0 address select 7-0
//	map(0x11, 0x11) GIO0 address select 10-8, GI0 address MODE0-1
//	map(0x12, 0x12) GIO1 address select 7-0
//	map(0x13, 0x13) GIO1 address select 10-8, GI0 address MODE0-1
//	map(0x14, 0x14) GIO0 ddr/mode
//	map(0x15, 0x15) GIO1 ddr/mode
}

/*
 * Serial
 */

void w83787f_device::irq_serial1_w(int state)
{
	if ((m_cr1 & 0x05) == 0x05)
		return;
	request_irq(3, state ? ASSERT_LINE : CLEAR_LINE);
}

void w83787f_device::irq_serial2_w(int state)
{
	if ((m_cr1 & 0x0a) == 0x0a)
		return;
	request_irq(4, state ? ASSERT_LINE : CLEAR_LINE);
}

void w83787f_device::txd_serial1_w(int state)
{
	if ((m_cr1 & 0x05) == 0x05)
		return;
	m_txd1_callback(state);
}

void w83787f_device::txd_serial2_w(int state)
{
	if ((m_cr1 & 0x0a) == 0x0a)
		return;
	m_txd2_callback(state);
}

void w83787f_device::dtr_serial1_w(int state)
{
	if ((m_cr1 & 0x05) == 0x05)
		return;
	m_ndtr1_callback(state);
}

void w83787f_device::dtr_serial2_w(int state)
{
	if ((m_cr1 & 0x0a) == 0x0a)
		return;
	m_ndtr2_callback(state);
}

void w83787f_device::rts_serial1_w(int state)
{
	if ((m_cr1 & 0x05) == 0x05)
		return;
	m_nrts1_callback(state);
}

void w83787f_device::rts_serial2_w(int state)
{
	if ((m_cr1 & 0x0a) == 0x0a)
		return;
	m_nrts2_callback(state);
}

void w83787f_device::rxd1_w(int state)
{
	m_pc_com[0]->rx_w(state);
}

void w83787f_device::ndcd1_w(int state)
{
	m_pc_com[0]->dcd_w(state);
}

void w83787f_device::ndsr1_w(int state)
{
	m_pc_com[0]->dsr_w(state);
}

void w83787f_device::nri1_w(int state)
{
	m_pc_com[0]->ri_w(state);
}

void w83787f_device::ncts1_w(int state)
{
	m_pc_com[0]->cts_w(state);
}

void w83787f_device::rxd2_w(int state)
{
	m_pc_com[1]->rx_w(state);
}

void w83787f_device::ndcd2_w(int state)
{
	m_pc_com[1]->dcd_w(state);
}

void w83787f_device::ndsr2_w(int state)
{
	m_pc_com[1]->dsr_w(state);
}

void w83787f_device::nri2_w(int state)
{
	m_pc_com[1]->ri_w(state);
}

void w83787f_device::ncts2_w(int state)
{
	m_pc_com[1]->cts_w(state);
}

/*
 * Parallel
 */

void w83787f_device::irq_parallel_w(int state)
{
	if ((m_cr1 & 0x30) == 0x30)
		return;
	request_irq(5, state ? ASSERT_LINE : CLEAR_LINE);
}

void w83787f_device::request_irq(int irq, int state)
{
	switch (irq)
	{
	case 1:
		m_irq1_callback(state);
		break;
	case 3:
		m_isa->irq3_w(state);
		break;
	case 4:
		m_isa->irq4_w(state);
		break;
	case 5:
		m_isa->irq5_w(state);
		break;
	case 6:
		m_isa->irq6_w(state);
		break;
	case 7:
		m_isa->irq7_w(state);
		break;
	case 8:
		m_irq8_callback(state);
		break;
	case 9:
		m_irq9_callback(state);
		break;
	case 10:
		m_isa->irq10_w(state);
		break;
	case 11:
		m_isa->irq11_w(state);
		break;
	case 12:
		m_isa->irq12_w(state);
		break;
	case 14:
		m_isa->irq14_w(state);
		break;
	case 15:
		m_isa->irq15_w(state);
		break;
	}
}
