// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*************************************************************

  IBM 21S850 IEEE 1394 400Mb/s Physical Layer
  Transceiver (PHY)

  Skeleton device

**************************************************************/

#include "emu.h"
#include "ibm21s850.h"

#define LOG_READS		(1 << 1)
#define LOG_WRITES		(1 << 2)
#define LOG_UNKNOWNS	(1 << 3)
#define LOG_ALL			(LOG_READS | LOG_WRITES | LOG_UNKNOWNS)

#define VERBOSE			(0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(IBM21S850, ibm21s850_device, "ibm21s850", "IBM 21S850 IEEE 1394 PHY")

ibm21s850_device::ibm21s850_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, IBM21S850, tag, owner, clock)
{
}

void ibm21s850_device::device_start()
{
	save_item(NAME(m_regs));
}

void ibm21s850_device::device_reset()
{
}

uint8_t ibm21s850_device::read(offs_t offset)
{
	if (offset < 0x10)
	{
		LOGMASKED(LOG_READS, "%s: Register %02x read: %02x\n", machine().describe_context(), offset, m_regs[offset]);
		return m_regs[offset];
	}
	LOGMASKED(LOG_READS | LOG_UNKNOWNS, "%s: Unknown Register (%02x) read\n", machine().describe_context(), offset);
	return 0;
}

void ibm21s850_device::write(offs_t offset, uint8_t data)
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
			m_regs[offset] = data;
			break;
	}
}