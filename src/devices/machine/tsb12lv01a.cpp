// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*************************************************************

  Texas Instruments TSB12LV01A/TSB12LV01AI IEEE 1394-1995
  High-Speed Serial-Bus Link-Layer Controller

  Skeleton device

**************************************************************/

#include "emu.h"
#include "tsb12lv01a.h"

#define LOG_READS       (1 << 1)
#define LOG_WRITES      (1 << 2)
#define LOG_UNKNOWNS    (1 << 3)
#define LOG_IRQS        (1 << 4)
#define LOG_ALL         (LOG_READS | LOG_WRITES | LOG_UNKNOWNS | LOG_IRQS)

#define VERBOSE         (LOG_ALL)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(TSB12LV01A, tsb12lv01a_device, "tsb12lv01a", "TSB12LV01A IEEE 1394 Link Controller")

tsb12lv01a_device::tsb12lv01a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TSB12LV01A, tag, owner, clock)
	, m_int_cb(*this)
	, m_phy_read_cb(*this)
	, m_phy_write_cb(*this)
{
}

void tsb12lv01a_device::device_start()
{
	save_item(NAME(m_version));
	save_item(NAME(m_node_address));
	save_item(NAME(m_ctrl));
	save_item(NAME(m_int_status));
	save_item(NAME(m_int_mask));
	save_item(NAME(m_cycle_timer));
	save_item(NAME(m_isoch_port_num));
	save_item(NAME(m_fifo_ctrl));
	save_item(NAME(m_diag_ctrl));
	save_item(NAME(m_phy_access));
	save_item(NAME(m_atf_status));
	save_item(NAME(m_itf_status));
	save_item(NAME(m_grf_status));

	m_int_cb.resolve_safe();
	m_phy_read_cb.resolve_safe(0x00);
	m_phy_write_cb.resolve_safe();
}

void tsb12lv01a_device::device_reset()
{
	m_version = 0x30313042;
	m_node_address = 0xffff0000;
	m_ctrl = 0x00000000;
	m_int_status = 0x10000000;
	m_int_mask = 0x00000000;
	m_cycle_timer = 0x00000000;
	m_isoch_port_num = 0x00000000;
	m_fifo_ctrl = 0x00000000;
	m_diag_ctrl = 0x00000000;
	m_phy_access = 0x00000000;
	m_atf_status = 0x00000000;
	m_itf_status = 0x00000000;
	m_grf_status = 0x00000000;
}

WRITE_LINE_MEMBER(tsb12lv01a_device::phy_reset_w)
{
	if (state)
	{
		set_interrupt(INT_PHRST);
	}
}

void tsb12lv01a_device::set_interrupt(uint32_t bit)
{
	m_int_status |= bit;
	LOGMASKED(LOG_IRQS, "Setting IRQ: %08x\n", bit);
	check_interrupts();
}

void tsb12lv01a_device::check_interrupts()
{
	const uint32_t active_bits = m_int_status & m_int_mask;
	if (active_bits & 0x7fffffff)
		m_int_status |= INT_INT;
	else
		m_int_status &= ~INT_INT;

	const int state = (m_int_status & m_int_mask & INT_INT) ? 1 : 0;
	LOGMASKED(LOG_IRQS, "Active IRQs: %08x, %sing IRQ\n", active_bits, state ? "rais" : "lower");
	m_int_cb(state);
}

void tsb12lv01a_device::reset_tx()
{
	// TODO
	set_interrupt(INT_TXRDY);
}

void tsb12lv01a_device::reset_rx()
{
}

void tsb12lv01a_device::clear_atf()
{
	const uint32_t atf_size = (m_fifo_ctrl & FIFO_CTRL_ATF_SIZE_MASK) >> FIFO_CTRL_ATF_SIZE_SHIFT;
	m_atf_status &= ~(ATF_STATUS_FULL | ATF_STATUS_CONERR | ATF_STATUS_ADRCOUNTER_MASK | ATF_STATUS_ATFSPACE_MASK);
	m_atf_status |= ATF_STATUS_EMPTY;
	m_atf_status |= (atf_size << ATF_STATUS_ATFSPACE_SHIFT);
}

void tsb12lv01a_device::clear_itf()
{
	const uint32_t itf_size = (m_fifo_ctrl & FIFO_CTRL_ITF_SIZE_MASK) >> FIFO_CTRL_ITF_SIZE_SHIFT;
	m_itf_status &= ~(ITF_STATUS_FULL | ITF_STATUS_ITFSPACE_MASK);
	m_itf_status |= ITF_STATUS_EMPTY;
	m_itf_status |= (itf_size << ITF_STATUS_ITFSPACE_SHIFT);
}

void tsb12lv01a_device::clear_grf()
{
	const uint32_t atf_size = (m_fifo_ctrl & FIFO_CTRL_ATF_SIZE_MASK) >> FIFO_CTRL_ATF_SIZE_SHIFT;
	const uint32_t itf_size = (m_fifo_ctrl & FIFO_CTRL_ITF_SIZE_MASK) >> FIFO_CTRL_ITF_SIZE_SHIFT;
	const uint32_t grf_size = 0x200 - (atf_size + itf_size);
	m_grf_status &= ~(GRF_STATUS_CD | GRF_STATUS_PACCOM | GRF_STATUS_GRFTOTAL_MASK | GRF_STATUS_WRITECOUNT_MASK);
	m_grf_status |= GRF_STATUS_EMPTY;
	m_grf_status |= grf_size << GRF_STATUS_GRFSIZE_SHIFT;
}

uint32_t tsb12lv01a_device::read(offs_t offset)
{
	switch (offset)
	{
		case 0x00:
			LOGMASKED(LOG_READS, "%s: TSB12 Version register read: %08x\n", machine().describe_context(), m_version);
			return m_version;
		case 0x04:
			LOGMASKED(LOG_READS, "%s: TSB12 Node Address register read: %08x\n", machine().describe_context(), m_node_address);
			return m_node_address;
		case 0x08:
			LOGMASKED(LOG_READS, "%s: TSB12 Control register read: %08x\n", machine().describe_context(), m_ctrl);
			return m_ctrl;
		case 0x0c:
			LOGMASKED(LOG_READS, "%s: TSB12 Interrupt Status register read: %08x\n", machine().describe_context(), m_int_status);
			return m_int_status;
		case 0x10:
			LOGMASKED(LOG_READS, "%s: TSB12 Interrupt Mask register read: %08x\n", machine().describe_context(), m_int_mask);
			return m_int_mask;
		case 0x14:
			LOGMASKED(LOG_READS, "%s: TSB12 Cycle Timer register read: %08x\n", machine().describe_context(), m_cycle_timer);
			return m_cycle_timer;
		case 0x18:
			LOGMASKED(LOG_READS, "%s: TSB12 Isochronous Receive-Port Number register read: %08x\n", machine().describe_context(), m_isoch_port_num);
			return m_isoch_port_num;
		case 0x1c:
			LOGMASKED(LOG_READS, "%s: TSB12 FIFO Control register read: %08x\n", machine().describe_context(), m_fifo_ctrl);
			return m_fifo_ctrl;
		case 0x20:
			LOGMASKED(LOG_READS, "%s: TSB12 Diagnostic Control register read: %08x\n", machine().describe_context(), m_diag_ctrl);
			return m_diag_ctrl;
		case 0x24:
			LOGMASKED(LOG_READS, "%s: TSB12 Phy-Chip Access register read: %08x\n", machine().describe_context(), m_phy_access);
			return m_phy_access;
		case 0x30:
			LOGMASKED(LOG_READS, "%s: TSB12 Asynchronous Transmit-FIFO (ATF) register read: %08x\n", machine().describe_context(), m_atf_status);
			return m_atf_status;
		case 0x34:
			LOGMASKED(LOG_READS, "%s: TSB12 Isochronous Transmit-FIFO (ITF) register read: %08x\n", machine().describe_context(), m_itf_status);
			return m_itf_status;
		case 0x3c:
			LOGMASKED(LOG_READS, "%s: TSB12 General Receive-FIFO (GRF) register read: %08x\n", machine().describe_context(), m_grf_status);
			return m_grf_status;
		default:
			LOGMASKED(LOG_READS | LOG_UNKNOWNS, "%s: TSB12 Unknown read: %08x\n", machine().describe_context(), offset << 2);
			return 0;
	}
}

void tsb12lv01a_device::write(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
		case 0x00:
			LOGMASKED(LOG_WRITES, "%s: TSB12 Version register write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0x04:
			LOGMASKED(LOG_WRITES, "%s: TSB12 Node Address register write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0x08:
			LOGMASKED(LOG_WRITES, "%s: TSB12 Control register write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
			if (data & CTRL_RSTTX)
			{
				data &= ~CTRL_RSTTX;
				reset_tx();
			}
			if (data & CTRL_RSTRX)
			{
				data &= ~CTRL_RSTRX;
				reset_rx();
			}
			COMBINE_DATA(&m_ctrl);
			break;
		case 0x0c:
			LOGMASKED(LOG_WRITES, "%s: TSB12 Interrupt Status register write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_int_status &= ~data;
			check_interrupts();
			break;
		case 0x10:
			LOGMASKED(LOG_WRITES, "%s: TSB12 Interrupt Mask register write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
			COMBINE_DATA(&m_int_mask);
			check_interrupts();
			break;
		case 0x14:
			LOGMASKED(LOG_WRITES, "%s: TSB12 Cycle Timer register write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0x18:
			LOGMASKED(LOG_WRITES, "%s: TSB12 Isochronous Receive-Port Number register write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0x1c:
			LOGMASKED(LOG_WRITES, "%s: TSB12 FIFO Control register write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_fifo_ctrl &= ~FIFO_CTRL_RW_BITS;
			m_fifo_ctrl |= data & FIFO_CTRL_RW_BITS;
			if (data & FIFO_CTRL_CLRATF)
				clear_atf();
			if (data & FIFO_CTRL_CLRITF)
				clear_itf();
			if (data & FIFO_CTRL_CLRGRF)
				clear_grf();
			break;
		case 0x20:
			LOGMASKED(LOG_WRITES, "%s: TSB12 Diagnostic Control register write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0x24:
			LOGMASKED(LOG_WRITES, "%s: TSB12 Phy-Chip Access register write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
			if (data & PHY_WRPHY)
			{
				const uint8_t phy_addr = (data & PHY_PHYRGAD_MASK) >> PHY_PHYRGAD_SHIFT;
				const uint8_t phy_data = (data & PHY_PHYRGDATA_MASK) >> PHY_PHYRGDATA_SHIFT;
				m_phy_write_cb(phy_addr, phy_data);
			}
			if (data & PHY_RDPHY)
			{
				const uint8_t phy_addr = (data & PHY_PHYRGAD_MASK) >> PHY_PHYRGAD_SHIFT;
				m_phy_access &= ~PHY_PHYRXAD_MASK;
				m_phy_access |= phy_addr << PHY_PHYRXAD_SHIFT;
				m_phy_access &= ~PHY_PHYRXDATA_MASK;
				m_phy_access |= m_phy_read_cb(phy_addr);

				if (phy_addr == 0)
				{
				}

				m_int_status |= INT_PHYRRX;
				check_interrupts();
			}

			m_phy_access &= ~(PHY_PHYRGAD_MASK | PHY_PHYRGDATA_MASK);
			m_phy_access |= data & (PHY_PHYRGAD_MASK | PHY_PHYRGDATA_MASK);
			break;
		case 0x30:
			LOGMASKED(LOG_WRITES, "%s: TSB12 Asynchronous Transmit-FIFO (ATF) register write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0x34:
			LOGMASKED(LOG_WRITES, "%s: TSB12 Isochronous Transmit-FIFO (ITF) register write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		case 0x3c:
			LOGMASKED(LOG_WRITES, "%s: TSB12 General Receive-FIFO (GRF) register write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
			break;
		default:
			LOGMASKED(LOG_WRITES | LOG_UNKNOWNS, "%s: TSB12 Unknown write: %08x = %08x & %08x\n", machine().describe_context(), offset << 2, data, mem_mask);
			break;
	}
}
