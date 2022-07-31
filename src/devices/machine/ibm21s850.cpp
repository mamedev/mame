// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*************************************************************

  IBM 21S850 IEEE-1394 400Mb/s 1-Port PHY
  IBM 21S851 IEEE-1394 400Mb/s 3-Port PHY

  Skeleton device

**************************************************************/

#include "emu.h"
#include "ibm21s850.h"

#define LOG_READS       (1 << 1)
#define LOG_WRITES      (1 << 2)
#define LOG_UNKNOWNS    (1 << 3)
#define LOG_ALL         (LOG_READS | LOG_WRITES | LOG_UNKNOWNS)

#define VERBOSE         (LOG_ALL)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(IBM21S850, ibm21s850_device, "ibm21s850", "IBM 21S850 IEEE-1394 1-Port PHY")
DEFINE_DEVICE_TYPE(IBM21S851, ibm21s851_device, "ibm21s851", "IBM 21S850 IEEE-1394 3-Port PHY")

ibm21s85x_base_device::ibm21s85x_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_reset_cb(*this)
{
}

ibm21s850_device::ibm21s850_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ibm21s85x_base_device(mconfig, IBM21S850, tag, owner, clock)
{
}

ibm21s851_device::ibm21s851_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ibm21s85x_base_device(mconfig, IBM21S851, tag, owner, clock)
{
}

void ibm21s85x_base_device::device_start()
{
	save_item(NAME(m_regs));

	m_reset_timer = timer_alloc(FUNC(ibm21s85x_base_device::reset_tick), this);

	m_reset_cb.resolve_safe();
}

void ibm21s85x_base_device::device_reset()
{
	memset(m_regs, 0, 0x10);

	m_regs[ROOT_OFFS] |= ROOT_MASK;                     // Root node
	m_regs[GAP_COUNT_OFFS] |= 0x3f;                     // Initial reset value
	m_regs[SPEED_OFFS] |= SPEED_400MBIT << SPEED_SHIFT; // 21S850 and 21S851 both indicate maximum 400Mb/s rate
	m_regs[ENHANCED_REGS_OFFS] |= ENHANCED_REGS_MASK;   // 21S850 and 21S851 both have an enhanced register map
	m_regs[CABLE_PWR_OFFS] |= CABLE_PWR_MASK;           // Cable is powered
	m_regs[CONNECTION1_OFFS] |= CONNECTION1_MASK;       // Port 1 connected
	m_regs[ARB_PHASE_OFFS] |= PHASE_BUS_RESET << ARB_PHASE_OFFS; // Power up in Bus Reset phase

	power_on_reset();
}

void ibm21s850_device::device_reset()
{
	ibm21s85x_base_device::device_reset();

	m_regs[NUM_PORTS_OFFS] |= 0x01;     // 1 port available
	m_regs[ENV_OFFS] |= 1 << ENV_SHIFT; // Cable PHY environment
	m_regs[REG_COUNT_OFFS] |= 0x09;     // 9 registers following the standard block on 21S850
}

void ibm21s851_device::device_reset()
{
	ibm21s85x_base_device::device_reset();

	m_regs[NUM_PORTS_OFFS] |= 0x03;     // 3 port available
	m_regs[ENV_OFFS] |= 1 << ENV_SHIFT; // Cable PHY environment
	m_regs[REG_COUNT_OFFS] |= 0x0b;     // 11 registers following the standard block on 21S851
}

TIMER_CALLBACK_MEMBER(ibm21s85x_base_device::reset_tick)
{
	if (param)
	{
		m_reset_cb(0);
		m_reset_timer->adjust(attotime::never);
	}
	else
	{
		m_reset_cb(1);
		m_reset_timer->adjust(attotime::from_usec(26266), 1); // RC reset circuit, 3.3V in, 2.3V out, 10KOhm resistor, 2.2uF capacitor
	}
}

uint8_t ibm21s85x_base_device::read(offs_t offset)
{
	if (offset < 0x10)
	{
		LOGMASKED(LOG_READS, "%s: Register %02x read: %02x\n", machine().describe_context(), offset, m_regs[offset]);
		return m_regs[offset];
	}
	LOGMASKED(LOG_READS | LOG_UNKNOWNS, "%s: Unknown Register (%02x) read\n", machine().describe_context(), offset);
	return 0;
}

void ibm21s85x_base_device::power_on_reset()
{
	m_reset_timer->adjust(attotime::zero, 0);
}

void ibm21s85x_base_device::write(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		default:
			LOGMASKED(LOG_WRITES, "%s: Register %02x write (ignored): %02x\n", machine().describe_context(), offset, data);
			break;
		case 0x01:
			LOGMASKED(LOG_WRITES, "%s: Register 01 write: %02x\n", machine().describe_context(), data);
			LOGMASKED(LOG_WRITES, "%s:     Root Hold-off: %d\n", machine().describe_context(), (data & ROOT_HOLD_MASK) ? 1 : 0);
			LOGMASKED(LOG_WRITES, "%s:     Initiate Bus Reset: %d\n", machine().describe_context(), (data & BUS_RESET_MASK) ? 1 : 0);
			LOGMASKED(LOG_WRITES, "%s:     Gap Offset: %02x\n", machine().describe_context(), data & GAP_COUNT_MASK);
			if (data & BUS_RESET_MASK)
			{
				data &= ~BUS_RESET_MASK;
			}
			m_regs[offset] = data;
			break;
		case 0x0d:
			LOGMASKED(LOG_WRITES, "%s: Register 0d write: %02x\n", machine().describe_context(), data);
			LOGMASKED(LOG_WRITES, "%s:     Enable L-P Selftest: %d\n", machine().describe_context(), (data & LP_TEST_EN_MASK) ? 1 : 0);
			LOGMASKED(LOG_WRITES, "%s:     Enable Ack Accel Arbitration: %d\n", machine().describe_context(), (data & ACK_ACCEL_EN_MASK) ? 1 : 0);
			LOGMASKED(LOG_WRITES, "%s:     Enable Multi-Speed Concat: %d\n", machine().describe_context(), (data & MULTISP_CONCAT_EN_MASK) ? 1 : 0);
			LOGMASKED(LOG_WRITES, "%s:     Mask LPS Bit: %d\n", machine().describe_context(), (data & MASK_LPS_MASK) ? 1 : 0);
			m_regs[offset] = data;
			break;
		case 0x0e:
			LOGMASKED(LOG_WRITES, "%s: Register 0e write: %02x\n", machine().describe_context(), data);
			LOGMASKED(LOG_WRITES, "%s:     Enable Tx/Rx Timeout: %d\n", machine().describe_context(), (data & EN_TIMEOUT_MASK) ? 1 : 0);
			LOGMASKED(LOG_WRITES, "%s:     Ignore Unplug: %d\n", machine().describe_context(), (data & IGNORE_UNPLUG_MASK) ? 1 : 0);
			LOGMASKED(LOG_WRITES, "%s:     Override CMC: %d\n", machine().describe_context(), (data & OVERRIDE_CMC_MASK) ? 1 : 0);
			LOGMASKED(LOG_WRITES, "%s:     Software CMC: %d\n", machine().describe_context(), (data & SOFT_CMC_MASK) ? 1 : 0);
			LOGMASKED(LOG_WRITES, "%s:     Disable Port 1: %d\n", machine().describe_context(), (data & DISABLE_P1_MASK) ? 1 : 0);
			m_regs[offset] = data;
			break;
		case 0x0f:
			LOGMASKED(LOG_WRITES, "%s: Register 0f write: %02x\n", machine().describe_context(), data);
			LOGMASKED(LOG_WRITES, "%s:     Soft POR: %d\n", machine().describe_context(), (data & SOFT_POR_MASK) ? 1 : 0);
			LOGMASKED(LOG_WRITES, "%s:     Send PHY-Link Diag (SID) Packet: %d\n", machine().describe_context(), (data & SEND_PL_DIAG_MASK) ? 1 : 0);
			LOGMASKED(LOG_WRITES, "%s:     Degate Ack_Acc_Arb: %d\n", machine().describe_context(), (data & ACK_ACCEL_SYNC_MASK) ? 1 : 0);
			LOGMASKED(LOG_WRITES, "%s:     Initiate Short Bus Reset: %d\n", machine().describe_context(), (data & ISBR_MASK) ? 1 : 0);
			if (data & SOFT_POR_MASK)
			{
				data &= ~SOFT_POR_MASK;
				power_on_reset();
			}
			m_regs[offset] = data;
			break;
	}
}
