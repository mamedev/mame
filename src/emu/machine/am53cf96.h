/*
 * am53cf96.h
 *
 */

#ifndef _AM53CF96_H_
#define _AM53CF96_H_

#include "scsi.h"
#include "scsidev.h"

struct AM53CF96interface
{
	const SCSIConfigTable *scsidevs;	/* SCSI devices */
	void (*irq_callback)(running_machine &machine);	/* irq callback */
};

#define MCFG_AM53CF96_ADD( _tag, _config ) \
	MCFG_DEVICE_ADD( _tag, AM53CF96, 0 ) \
	MCFG_DEVICE_CONFIG(_config)

// 53CF96 register set
enum
{
	REG_XFERCNTLOW = 0,	// read = current xfer count lo byte, write = set xfer count lo byte
	REG_XFERCNTMID,		// read = current xfer count mid byte, write = set xfer count mid byte
	REG_FIFO,		// read/write = FIFO
	REG_COMMAND,		// read/write = command

	REG_STATUS,		// read = status, write = destination SCSI ID (4)
	REG_IRQSTATE,		// read = IRQ status, write = timeout         (5)
	REG_INTSTATE,		// read = internal state, write = sync xfer period (6)
	REG_FIFOSTATE,		// read = FIFO status, write = sync offset
	REG_CTRL1,		// read/write = control 1
	REG_CLOCKFCTR,		// clock factor (write only)
	REG_TESTMODE,		// test mode (write only)
	REG_CTRL2,		// read/write = control 2
	REG_CTRL3,		// read/write = control 3
	REG_CTRL4,		// read/write = control 4
	REG_XFERCNTHI,		// read = current xfer count hi byte, write = set xfer count hi byte
	REG_DATAALIGN		// data alignment (write only)
};

class am53cf96_device : public device_t,
					   public AM53CF96interface
{
public:
	// construction/destruction
	am53cf96_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	void dma_read_data(int bytes, UINT8 *pData);
	void dma_write_data(int bytes, UINT8 *pData);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	static const device_timer_id TIMER_TRANSFER = 0;

	scsidev_device *devices[8];

	UINT8 scsi_regs[32];
	UINT8 fifo[16];
	UINT8 fptr;
	UINT8 xfer_state;
	UINT8 last_id;

	emu_timer* m_transfer_timer;
};

// device type definition
extern const device_type AM53CF96;

#endif
