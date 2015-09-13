// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
 * ncr5380.h SCSI controller
 *
 */

#ifndef _NCR5380_H_
#define _NCR5380_H_

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

#define MCFG_NCR5380_IRQ_CB(_devcb) \
	devcb = &ncr5380_device::set_irq_callback(*device, DEVCB_##_devcb);

class ncr5380_device : public legacy_scsi_host_adapter
{
public:
	// construction/destruction
	ncr5380_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_irq_callback(device_t &device, _Object object) { return downcast<ncr5380_device &>(device).m_irq_cb.set_callback(object); }

	// our API
	UINT8 ncr5380_read_reg(UINT32 offset);
	void ncr5380_write_reg(UINT32 offset, UINT8 data);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_stop();

private:
	UINT8 m_5380_Registers[8];
	UINT8 m_last_id;
	UINT8 m_5380_Command[32];
	INT32 m_cmd_ptr, m_d_ptr, m_d_limit, m_next_req_flag;
	UINT8 m_5380_Data[512];
	devcb_write_line m_irq_cb;  /* irq callback */
};

// device type definition
extern const device_type NCR5380;

#endif
