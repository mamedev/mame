/*
 * ncr5394/5396.h SCSI controller
 *
 */

#ifndef _NCR539x_H_
#define _NCR539x_H_

#include "machine/scsidev.h"

struct NCR539Xinterface
{
	const SCSIConfigTable *scsidevs;		/* SCSI devices */
    devcb_write_line m_out_irq_cb;          /* IRQ line */
    devcb_write_line m_out_drq_cb;          /* DRQ line */
};

// 539x registers
enum
{
};

// device stuff
#define MCFG_NCR539X_ADD(_tag, _clock, _intrf) \
    MCFG_DEVICE_ADD(_tag, NCR539X, _clock) \
    MCFG_DEVICE_CONFIG(_intrf)

class ncr539x_device : public device_t,
					   public NCR539Xinterface
{
public:
	// construction/destruction
	ncr539x_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// our API
    DECLARE_READ8_MEMBER(read);
    DECLARE_WRITE8_MEMBER(write);

	void dma_read_data(int bytes, UINT8 *pData);
	void dma_write_data(int bytes, UINT8 *pData);

	void *get_scsi_device(int id);

	void scan_devices();
protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_stop();
	virtual void device_config_complete();
    virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
    void fifo_write(UINT8 data);
    void check_fifo_executable();
    void exec_fifo();
    void update_fifo_internal_state(int bytes);

	SCSIInstance *m_scsi_devices[8];

    UINT32 m_xfer_count;
    UINT32 m_dma_size;
    UINT8 m_command;
	UINT8 m_last_id;
    UINT8 m_timeout;
    UINT8 m_sync_xfer_period;
    UINT8 m_sync_offset;
    UINT8 m_control1, m_control2, m_control3, m_control4;
    UINT8 m_clock_factor;
    UINT8 m_forced_test;
    UINT8 m_data_alignment;
                   
    bool m_selected;
    bool m_chipid_available, m_chipid_lock;
                       
    static const int m_fifo_size = 16;
    UINT8 m_fifo_ptr, m_fifo[m_fifo_size];

    int m_xfer_remaining;   // amount in the FIFO when we're in data in phase

    // read-only registers
    UINT8 m_status, m_irq_status, m_internal_state, m_fifo_internal_state;

    static const int m_buffer_size = 2048;

    UINT8 m_buffer[m_buffer_size];
    int m_buffer_offset, m_buffer_remaining, m_total_data;

    emu_timer *m_operation_timer;

	devcb_resolved_write_line	m_out_irq_func;
	devcb_resolved_write_line	m_out_drq_func;
};

// device type definition
extern const device_type NCR539X;
#endif
