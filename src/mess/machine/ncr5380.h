/*
 * ncr5380.h SCSI controller
 *
 */

#ifndef _NCR5380_H_
#define _NCR5380_H_

#include "machine/scsi.h"
#include "machine/scsidev.h"

struct NCR5380interface
{
	const SCSIConfigTable *scsidevs;		/* SCSI devices */
	void (*irq_callback)(running_machine &machine, int state);	/* irq callback */
};

// 5380 registers
enum
{
	R5380_CURDATA = 0,	// current SCSI data (read only)
	R5380_OUTDATA = 0,	// output data (write only)
	R5380_INICOMMAND,	// initiator command
	R5380_MODE,		// mode
	R5380_TARGETCMD,	// target command
	R5380_SELENABLE,	// select enable (write only)
	R5380_BUSSTATUS = R5380_SELENABLE,	// bus status (read only)
	R5380_STARTDMA,		// start DMA send (write only)
	R5380_BUSANDSTAT = R5380_STARTDMA,	// bus and status (read only)
	R5380_DMATARGET,	// DMA target (write only)
	R5380_INPUTDATA = R5380_DMATARGET,	// input data (read only)
	R5380_DMAINIRECV,	// DMA initiator receive (write only)
	R5380_RESETPARITY = R5380_DMAINIRECV	// reset parity/interrupt (read only)
};

// special Mac Plus registers - they implemented it weird
#define R5380_OUTDATA_DTACK	(R5380_OUTDATA | 0x10)
#define R5380_CURDATA_DTACK	(R5380_CURDATA | 0x10)

// device stuff
#define MCFG_NCR5380_ADD(_tag, _clock, _intrf) \
    MCFG_DEVICE_ADD(_tag, NCR5380, _clock) \
    MCFG_DEVICE_CONFIG(_intrf)

class ncr5380_device : public device_t,
					   public NCR5380interface
{
public:
	// construction/destruction
	ncr5380_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// our API
	UINT8 ncr5380_read_reg(UINT32 offset);
	void ncr5380_write_reg(UINT32 offset, UINT8 data);

	void ncr5380_read_data(int bytes, UINT8 *pData);
	void ncr5380_write_data(int bytes, UINT8 *pData);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_stop();
	virtual void device_config_complete();

private:
	scsidev_device *m_scsi_devices[8];

	UINT8 m_5380_Registers[8];
	UINT8 m_last_id;
	UINT8 m_5380_Command[32];
	INT32 m_cmd_ptr, m_d_ptr, m_d_limit, m_next_req_flag;
	UINT8 m_5380_Data[512];
};

// device type definition
extern const device_type NCR5380;

/***************************************************************************
    PROTOTYPES
***************************************************************************/
READ8_DEVICE_HANDLER(ncr5380_read_reg);
WRITE8_DEVICE_HANDLER(ncr5380_write_reg);

void ncr5380_read_data(device_t *dev, int bytes, UINT8 *pData);
void ncr5380_write_data(device_t *dev, int bytes, UINT8 *pData);

#endif
