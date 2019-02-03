// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
 * ncr5380.h SCSI controller
 *
 */

#ifndef MAME_MACHINE_NCR5380_H
#define MAME_MACHINE_NCR5380_H

#pragma once

#include "legscsi.h"

// 5380 registers
enum
{
	R5380_CURDATA = 0,  // current SCSI data (read only)
	R5380_OUTDATA = 0,  // output data (write only)
	R5380_INICOMMAND,   // initiator command
	R5380_MODE,     // mode
	R5380_TARGETCMD,    // target command
	R5380_SELENABLE,    // select enable (write only)
	R5380_BUSSTATUS = R5380_SELENABLE,  // bus status (read only)
	R5380_STARTDMA,     // start DMA send (write only)
	R5380_BUSANDSTAT = R5380_STARTDMA,  // bus and status (read only)
	R5380_DMATARGET,    // DMA target (write only)
	R5380_INPUTDATA = R5380_DMATARGET,  // input data (read only)
	R5380_DMAINIRECV,   // DMA initiator receive (write only)
	R5380_RESETPARITY = R5380_DMAINIRECV    // reset parity/interrupt (read only)
};

// special Mac Plus registers - they implemented it weird
#define R5380_OUTDATA_DTACK (R5380_OUTDATA | 0x10)
#define R5380_CURDATA_DTACK (R5380_CURDATA | 0x10)

// device stuff


class ncr5380_device : public legacy_scsi_host_adapter
{
public:
	// construction/destruction
	ncr5380_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_callback() { return m_irq_cb.bind(); }

	// our API
	uint8_t ncr5380_read_reg(uint32_t offset);
	void ncr5380_write_reg(uint32_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_stop() override;

private:
	uint8_t m_5380_Registers[8];
	uint8_t m_last_id;
	uint8_t m_5380_Command[32];
	int32_t m_cmd_ptr, m_d_ptr, m_d_limit, m_next_req_flag;
	uint8_t m_5380_Data[512];
	devcb_write_line m_irq_cb;  /* irq callback */
};

// device type definition
DECLARE_DEVICE_TYPE(NCR5380, ncr5380_device)

#endif // MAME_MACHINE_NCR5380_H
